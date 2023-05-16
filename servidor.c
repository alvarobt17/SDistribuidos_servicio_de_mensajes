#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include<stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>


#include "lines.h"
#include "implementacion.h"

#define MAX_SIZE 	255
#define TAM_BUFFER 	1024

#define IP_SERVER "localhost"

int id_mensaje;


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

	int puerto = atoi(argv[2]);		//Puerto de escucha del servidor

	printf("s> init server %s:%d\n", IP_SERVER, puerto);

	// Comprobamos que la carpeta datos existe, sino la creamos
	if (access("datos", F_OK) == -1) {
		if (mkdir("datos", 0777) == -1) {
			perror("Error al crear la carpeta datos");
			exit(1);
		}
	}

    //Atributos de los threads    
	pthread_attr_t t_attr;		
   	pthread_t thid;

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

	int tipo_op;

	if(strcmp(operacion, "REGISTER") == 0){
		tipo_op = 0;
	}else if(strcmp(operacion, "UNREGISTER") == 0){
		tipo_op = 1;
	}else if(strcmp(operacion, "CONNECT") == 0){
		tipo_op = 2;
	}else if(strcmp(operacion, "DISCONNECT") == 0){
		tipo_op = 3;
	}else if(strcmp(operacion, "SEND") == 0){
		tipo_op = 4;
	}else if(strcmp(operacion, "CONNECTEDUSERS") == 0){
		tipo_op = 5;
	}else{
		tipo_op = -1;
	}

	char usuario[MAX_SIZE], alias[MAX_SIZE], fecha[MAX_SIZE], str_respuesta[MAX_SIZE], destino[MAX_SIZE], mensaje[MAX_SIZE];

	switch(tipo_op){

		case 0:			//Registrarse

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

			// Mostramos por la consola del servidor el resultado de la operación
			if(respuesta == 0){
				printf("s> REGISTER %s OK\n", alias);
			}else{
				printf("s> REGISTER %s FAIL\n", alias);
			}
		
			sprintf(str_respuesta, "%d", respuesta);
			//Enviamos respuesta al cliente
			err = sendMessage(sc, str_respuesta, strlen(str_respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}

			close(sc);
			
			break;

        case 1:			//Baja del sistema

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);

			//Dar de baja al usuario
			respuesta = baja(alias);

			if(respuesta == 0){
				printf("s> UNREGISTER %s OK\n", alias);
			}else{
				printf("s> UNREGISTER %s FAIL\n", alias);
			}

			sprintf(str_respuesta, "%d", respuesta);
			//Enviamos respuesta al cliente
			err = sendMessage(sc, str_respuesta, strlen(str_respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}	

			close(sc);
			
			break;
        
        case 2:				//Conectarse

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);
			int puerto;
			//Puerto
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			puerto = atoi(buffer);

			//Obtenemos la dirección ip del usuario
			struct sockaddr_in client_addr;
			socklen_t client_addr_len = sizeof(client_addr);
			if (getpeername(sc, (struct sockaddr*)&client_addr, &client_addr_len) == -1) {
				printf("s> Error al obtener la dirección IP del cliente al intentar conectar\n");
				respuesta = -1;

				//Enviamos respuesta al cliente
				sprintf(str_respuesta, "%d", respuesta);
				err = sendMessage(sc, str_respuesta, strlen(str_respuesta)+1);
				if (err == -1) {
					printf("s> Error en el envío de la respuesta al cliente\n");
				}

			}

			char ip_cliente[MAX_SIZE];
			inet_ntop(AF_INET, &(client_addr.sin_addr), ip_cliente, MAX_SIZE);

			//Conectar al usuario
			respuesta = conectar(alias, puerto, ip_cliente);

			if(respuesta == 0){
				printf("s> CONNECT %s OK\n", alias);
			}else{
				printf("s> CONNECT %s FAIL\n", alias);
			}
			//Enviamos respuesta al cliente
			sprintf(str_respuesta, "%d", respuesta);
			err = sendMessage(sc, str_respuesta, strlen(str_respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}
			
			
			if(respuesta == 0){
				// Enviamos los mensajes pendientes si hay
				if (mensajes_pendientes(alias, puerto, ip_cliente) == -1) {
					printf("s> Error al enviar los mensajes pendientes\n");
				}
			}
			

			close(sc);
			
			break;

        case 3:			//Desconectar de otro usuario

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);
			//Desconectar al usuario
			respuesta = desconectar(alias);

			if(respuesta == 0){
				printf("s> DISCONNECT %s OK\n", alias);
			}else{
				printf("s> DISCONNECT %s FAIL\n", alias);
			}

			//Enviamos respuesta al cliente
			sprintf(str_respuesta, "%d", respuesta);
			err = sendMessage(sc, str_respuesta, strlen(str_respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}

			close(sc);
			
			break;

        case 4:				//Enviar mensaje a otro usuario

			//Alias
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(alias, buffer);
			printf("s> Alias_server: %s\n", alias);

			//Usuario destino
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(destino, buffer);
			printf("s> Destino_server: %s\n", destino);

			//Mensaje
			err = readLine(sc, buffer, TAM_BUFFER+1);
			if (err == -1) {
				printf("s> Error en recepcion\n");
			}
			strcpy(mensaje, buffer);

			id_mensaje++;		//Incrementamos el id del mensaje
			int id_local = id_mensaje;

			//Enviar mensaje
			respuesta = enviar(alias, destino, mensaje, id_local);

			//Enviamos respuesta al cliente
			sprintf(str_respuesta, "%d", respuesta);
			err = sendMessage(sc, str_respuesta, strlen(str_respuesta)+1);
			if (err == -1) {
				printf("s> Error en el envío de la respuesta al cliente\n");
			}

			if(respuesta == 0){
				sprintf(str_respuesta, "%d", id_local);
				err = sendMessage(sc, str_respuesta, sizeof(str_respuesta));
				if (err == -1) {
					printf("s> Error en el envío del id del mensaje al cliente\n");
				}
			}

			close(sc);
			
			break;

        case 5:		//Lista de usuarios conectados

			respuesta = usuarios_conectados(sc);


			if(respuesta == 0){
				printf("s> CONNECTEDUSERS OK\n");
				
			}else{
				printf("s> CONNECTEDUSERS FAIL\n");
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
