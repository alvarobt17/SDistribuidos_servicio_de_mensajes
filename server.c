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

#include "lines.h"
#include "implementacion.h"

#define MAX_LINE 	256
#define MAX_SIZE 	255
#define TAM_BUFFER 	1024

//VARIABLES GLOBALES
pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;

void tratar_mensaje(int *sc);


int main(int argc, char *argv[]){

    //Atributos de los threads    
	pthread_attr_t t_attr;		
   	pthread_t thid;

	if (argc < 2) {
    printf("Uso: %s <puerto>\n", argv[0]);
    exit(1);
	}

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
		printf("Error en bind\n");
		return -1;
	}

    err = listen(sd, SOMAXCONN);
	if (err == -1) {
		printf("Error en listen\n");
		return -1;
	}

    size = sizeof(client_addr);
    /*
	El servidor estará siempre en marcha e ira gestionando
	las peticiones que reciba de los clientes
	*/
	while (1){
		printf("Esperando conexion\n");
		sc = accept(sd, (struct sockaddr *)&client_addr, (socklen_t *)&size);           //Conexión del socket con el cliente
			
		printf("Conexión establecida con un cliente\n");
        if (sc == -1){
            printf("Error en accept\n");
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
	int err;              //Variable error

	//Recibimos la petición del cliente
	char buffer[TAM_BUFFER];

	//Código de operación
	err = readLine(sc, buffer, TAM_BUFFER+1);
	if (err == -1) {
		printf("Error en recepcion\n");
	}

	//Ya se puede despertar al servidor
	mensaje_no_copiado = false;

	pthread_cond_signal(&cond_mensaje);

	pthread_mutex_unlock(&mutex_mensaje);

	//Comprobamos el tipo de operacion que se quiere realizar
	pthread_mutex_lock(&mutex_mensaje);
	switch(mensaje->tipo_op){

		case "REGISTER":			//Registrarse

        case "UNREGISTER":			//Baja del sistema
        
        case "CONNECT":				//Conectar con otro usuario

        case "DISCONNECT":			//Desconectar de otro usuario

        case "SEND":

        case "CONNECTEDUSERS":
			

		default:
			perror("Error al tratar la peticion");
			mensaje->key = -1;

			//Enviar mensaje al cliente
			//Key
			sprintf(buffer, "%d", mensaje->key);
			err = sendMessage(sc, buffer, strlen(buffer)+1);
			if (err == -1) {
					printf("Error en envío\n");
			}

			close(sc);                      // cierra la conexión (sc)

			break;
	}
	pthread_mutex_unlock(&mutex_mensaje);

	free(cliente);
	
	pthread_exit(0);
}
