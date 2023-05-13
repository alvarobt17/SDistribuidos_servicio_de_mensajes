#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include<stdbool.h>
#include <netinet/in.h>

#include "lines.h"
#include "implementacion.h"

#define MAX_LINE 	256
#define MAX_SIZE 	255
#define TAM_BUFFER 	1024

#define IP_SERVER "localhost"


//VARIABLES GLOBALES
pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;

void tratar_mensaje(int *sc);


int main(int argc, char *argv[]){

	if(argc < 2){
		perror("La ejecución del programa debe ser: ./server -p <puerto>");
		return -1;
	}

	int puerto = atoi(argv[2]);

	printf("s> init server %s:%d\n", IP_SERVER, puerto);

    //Atributos de los threads    
	pthread_attr_t t_attr;		
   	pthread_t thid;

	int puerto = atoi(argv[1]);		//Puerto de escucha

    int err;                //Variable error

    //Inicializamos las variables de los threads
	pthread_mutex_init(&mutex_mensaje, NULL);
	pthread_cond_init(&cond_mensaje, NULL);
	pthread_attr_init(&t_attr);

    //Atributos de los threads, threads independientes
	pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

    //Abrimos el socket y lo conectamos a un puerto
    struct sockaddr_in server_addr,  client_addr;
	socklen_t size;
    int sd, sc;
	int val;

	sd =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd < 0) {
		perror("Error in socket");
		exit(1);
	}

	val = 1;
	err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &val,sizeof(int));
	if (err < 0) {
		perror("Error in opction");
		exit(1);
	}

	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port        = htons(puerto);

	err = bind(sd, (const struct sockaddr *)&server_addr,sizeof(server_addr));
	if (err == -1) {
		printf("s> Error en bind\n");
		return -1;
	}

    err = listen(sd, SOMAXCONN);
	if (err == -1) {
		printf("s> Error en listen\n");
		return -1;
	}

    size = sizeof(client_addr);
    /*
	El servidor estará siempre en marcha e ira gestionando
	las peticiones que reciba de los clientes
	*/
	while (1){
		printf("s> Esperando conexion\n");
		sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);           //Conexión del socket con el cliente
			
		printf("s> Conexión establecida con un cliente\n");
        if (sc == -1){
            printf("s> Error en accept\n");
            return -1;
        }
        int *cliente = malloc(sizeof(int));
        if (cliente == NULL) {
            perror("Error al asignar memoria para el cliente");
            exit(1);
        }

        *cliente = sc;
        //Creamos un thread para tratar el mensaje
        pthread_create(&thid, &t_attr, (void *) tratar_mensaje, (int*) cliente);    

        //Esperemos a que los hilos terminen de ejecutarse
        pthread_detach(thid);
	}

	close (sd);
	return(0);
}


void tratar_mensaje(int *cliente){
	//El thread copia el mensaje a un mensaje local
	pthread_mutex_lock(&mutex_mensaje);

	int sc = *cliente;
	int err, respuesta;              //Variable error

	//Recibimos la petición del cliente
	char buffer[TAM_BUFFER];

	//Código de operación
	err = readLine(sc, buffer, TAM_BUFFER+1);
	if (err == -1) {
		printf("s> Error en recepcion\n");
	}

	char operacion[MAX_SIZE];
	strcpy(operacion, buffer);

	//Ya se puede despertar al servidor
	mensaje_no_copiado = false;

	pthread_cond_signal(&cond_mensaje);

	pthread_mutex_unlock(&mutex_mensaje);

	//Comprobamos el tipo de operacion que se quiere realizar
	pthread_mutex_lock(&mutex_mensaje);
	switch(operacion){

		case "REGISTER":			//Registrarse

			char usuario[MAX_SIZE], alias[MAX_SIZE], fecha[MAX_SIZE];

			//Usuario
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(usuario, buffer);

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);

			//Fecha
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(fecha, buffer);

			//Registrar usuario
			respuesta = registrar(usuario, alias, fecha);
			
			//Enviamos respuesta al cliente
			err = sendMessage(sc, respuesta, strlen(respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}

			close(sc);
			
			break;

        case "UNREGISTER":			//Baja del sistema

			char alias[MAX_SIZE];

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);

			//Dar de baja al usuario
			respuesta = baja(alias);

			//Enviamos respuesta al cliente
			err = sendMessage(sc, respuesta, strlen(respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}	

			close(sc);
			
			break;
        
        case "CONNECT":				//Conectar con otro usuario

			char alias[MAX_SIZE];
			int puerto;

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);

			//Puerto
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			puerto = atoi(buffer);

			//Obtenemos la dirección ip del usuario
			// Dentro de la función tratar_mensaje()
			struct sockaddr_in client_addr;
			socklen_t client_addr_len = sizeof(client_addr);
			if (getpeername(sc, (struct sockaddr*)&client_addr, &client_addr_len) == -1) {
				printf("s> Error al obtener la dirección IP del cliente al intentar conectar\n");
				respuesta = -1;

			}else{
				char ip_cliente[MAX_SIZE] = inet_ntoa(client_addr.sin_addr);

				//Conectar al usuario
				respuesta = conectar(alias, puerto, ip_cliente);
			}
				
			//Enviamos respuesta al cliente
			err = sendMessage(sc, respuesta, strlen(respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}

			if(respuesta == 0){
				// Enviamos los mensajes pendientes si hay
				if (mensajes_pendientes(alias, puerto, ip) == -1) {
					printf("s> Error al enviar los mensajes pendientes\n");
				}
			}

			close(sc);
			
			break;

        case "DISCONNECT":			//Desconectar de otro usuario

			char alias[MAX_SIZE];

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);

			//Desconectar al usuario
			respuesta = desconectar(alias);

			//Enviamos respuesta al cliente
			err = sendMessage(sc, respuesta, strlen(respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}

			close(sc);
			
			break;

        case "SEND":				//Enviar mensaje a otro usuario

			char alias[MAX_SIZE], destino[MAX_SIZE], mensaje[MAX_SIZE];

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);

			//Usuario destino
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(destino, buffer);

			//Mensaje
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(mensaje, buffer);

			int *id_mens = (int*)malloc(sizeof(int))

			//Enviar mensaje
			respuesta = enviar(alias, destino, mensaje, id_mens);

			//Enviamos respuesta al cliente
			err = sendMessage(sc, respuesta, strlen(respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}

			if(respuesta == 0){
				err = sendMessage(sc, id_mens, sizeof(int));
				if (err == -1) {
					printf("s> Error en el envío del id del mensaje al cliente\n");
				}
			}

			free(id_mens);

			close(sc);
			
			break;

        case "CONNECTEDUSERS":		//Lista de usuarios conectados

			respuesta = usuarios_conectados(sc);

			if(respuesta = -1){
				printf("s> Error en la función usuarios_conectados\n");
			}

			close(sc);
			
			break;

		default:				//Operacion no válida

			printf("s> Código de operación no válido\n");
			printf("s> Código recibido: %s\n", operacion);

			close(sc);                      // cierra la conexión (sc)

			break;
	}
	pthread_mutex_unlock(&mutex_mensaje);

	free(cliente);
	
	pthread_exit(0);
}
