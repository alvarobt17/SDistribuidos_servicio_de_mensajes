#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <regex.h>

#include "implementacion.h"

#define TAM_BUFFER 1024

int registrar(char usuario[MAX_SIZE],char alias[MAX_SIZE], char fecha[MAX_SIZE]){

    // Cada usuario tendrá su propio txt en el que se guardarán todos sus datos y los mensajes que tenga pendientes de enviar
    // Comprobamos que el usuario no exista ya
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/%s.txt", alias);

    fichero = fopen(nombre_fichero, "w");
    if (fichero == NULL) {
        // Error al abrir el archivo
        return 1;
    }

    // Guardamos los datos del usuario en su fichero
    fprintf(fichero, "%s\n%s\n%s\n%d\n%s\n%s", usuario, alias, fecha, 0, "NULL", "NULL");
    fclose(fichero);

    return 0;

}

int baja(char alias[MAX_SIZE]){
    // Comprobamos que el usuario exista
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/%s.txt", alias);

    fichero = fopen(nombre_fichero, "r");
    if(fichero == NULL){
        // El usuario no existe
        fclose(fichero);
        return 1;           // USER DOES NOT EXIT
    }

    // Borramos el fichero
    fclose(fichero);
    if(remove(nombre_fichero) == -1){
        perror("Error al borrar el fichero");
        return 2;
    }

    return 0;
}

int conectar(char alias[MAX_SIZE], int puerto, char ip[MAX_SIZE]){

    //Cambiamos el estado del usuario a conectado e incluimos su ip y puerto
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/%s.txt", alias);

    fichero = fopen(nombre_fichero, "r+");
    if(fichero == NULL){
        // El usuario no existe
        fclose(fichero);
        return 1;           // USER DOES NOT EXIT
    }

    // Leemos la 4 línea para ver el estado del usuario
    char buffer[TAM_BUFFER];
    for(int i = 0; i < 4; i++){
        if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
            // Error al leer el fichero
            fclose(fichero);
            return 1;
        }
    }
    
    // Comprobamos que el usuario no esté ya conectado
    int conect = atoi(buffer);
    if(conect == 1){
        // El usuario ya está conectado
        fclose(fichero);
        return 2;           // USER ALREADY CONNECTED
    }

    // Cambiamos el estado del usuario a conectado y incluimos el puerto y la ip
    fseek(fichero, -2, SEEK_CUR);
    fprintf(fichero, "%d\n%d\n%s", 1, puerto, ip);

    fclose(fichero);

    return 0;

}

int desconectar(char alias[MAX_SIZE]){
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/%s.txt", alias);

    fichero = fopen(nombre_fichero, "r+");
    if(fichero == NULL){
        // El usuario no existe
        fclose(fichero);
        return 1;           // USER DOES NOT EXIT
    }

    // Leemos la 4 línea para ver el estado del usuario
    char buffer[TAM_BUFFER];
    int contandor = 1;
    for(int i = 0; i < 4; i++){
        if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
            // Error al leer el fichero
            fclose(fichero);
            return 3;           // DISCONNECTED ERROR
        }
    }
    
    // Comprobamos que el usuario no esté ya conectado
    int cont = atoi(buffer);
    if(cont == 0){
        // El usuario ya está desconectado
        fclose(fichero);
        return 2;           // USER NOT CONNECTED
    }

    // Cambiamos el estado del usuario a Dconectado y borramos el puerto y la ip
    fseek(fichero, -2, SEEK_CUR);
    fprintf(fichero, "%d\n%s\n%s", 0, "NULL", "NULL");

    fclose(fichero);
}

int enviar(char alias[MAX_SIZE], char mensaje[MAX_SIZE]){

    //Comprobamos si el usuario está conectado
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/%s.txt", alias);

    fichero = fopen(nombre_fichero, "r+");
    if(fichero == NULL){
        // El usuario no existe
        fclose(fichero);
        return 1;           // USER DOES NOT EXIT
    }

    // Leemos la 4 línea para ver el estado del usuario
    char buffer[TAM_BUFFER];
    int contandor = 1;
    for(int i = 0; i < 4; i++){
        if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
            // Error al leer el fichero
            fclose(fichero);
            return 1;
        }
    }

    // Comprobamos que el usuario esté conectado
    int cont = atoi(buffer);
    if(cont == 0){
        // El usuario no está conectado, guardamos el mensaje para enviarlo cuando se conecte
        fclose(fichero);

        // Guardamos el mensaje en el fichero de mensajes pendientes y si no existe lo creamos
        sprintf(nombre_fichero, "datos/%s_mensajes.txt", alias);
        fichero = fopen(nombre_fichero, "a+");
        if(fichero == NULL){
            // Error al abrir el fichero
            fclose(fichero);
            return 1;
        }
        
        //Vamos a la última línea del fichero y escribimos el mensaje nuevo
        fseek(fichero, 0, SEEK_END);
        fprintf(fichero, "%s\n", mensaje);

    }

}

int usuarios_conectados(int sc){
   
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "connected_users.txt");

    fichero = fopen(nombre_fichero, "r");
    if (fichero == NULL) {
        printf("No se pudo abrir el fichero.\n");
        return -1;
    }

    char buffer[TAM_BUFFER];
    fgets(buffer, TAM_BUFFER, fichero);

    int num_connect = atoi(buffer);

    if (num_connect <= 0) {
        fclose(fichero);
        return -1;
    }

    // Enviamos la cantidad de personas conectadas al cliente
    if(send(sc, &num_connect, sizeof(int), 0) == -1){
        printf("s> Error al enviar el número de personas conectadas");
        fclose(fichero);
        return -1;
    }

    while (fgets(buffer, TAM_BUFFER, fichero) != NULL) {
        if (strcmp(buffer, "") != 0) {
            err = sendMessage(sc, buffer, strlen(buffer)+1);
            if( err == -1){
                printf("s> Error al enviar el número de mensajes");
                return -1;
            }
        }
    }

    fclose(fichero);

    return 0;

}

int mensajes_pendientes(char alias[MAX_SIZE], int puerto, char ip[MAX_SIZE]){       // NO PROBADA
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/%s_mensajes.txt", alias);

    fichero = fopen(nombre_fichero, "r+");
    if(fichero == NULL){
        // No hay mensajes pendientes
        fclose(fichero);
        return 0;
    }

    // Conectamos el socket mediante la ip y el puerto dados
    int sc = socket(AF_INET, SOCK_STREAM, 0);
    if(sc == -1){
        printf("s> Error al crear el socket");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(puerto);
    server.sin_addr.s_addr = inet_addr(ip);

    if(connect(sc, (struct sockaddr *)&server, sizeof(server)) == -1){
        printf("s> Error al conectar el socket");
        return -1;
    }


    // Leemos el fichero y enviamos los mensajes
    char buffer[TAM_BUFFER];
    int contador = 0;
    while(fgets(buffer, TAM_BUFFER, fichero) != NULL){
        if(strcmp(buffer, "") != 0){
            contador ++;
        }
    }
    
    int err;

    // Enviamos el número de mensajes que vamos a enviar
    sprintf(buffer, "%d", contador);
    err = sendMessage(sc, buffer, strlen(buffer)+1);
    if( err == -1){
        printf("s> Error al enviar el número de mensajes");
        return -1;
    }

    // Enviamos los mensajes
    fseek(fichero, 0, SEEK_SET);
    while(fgets(buffer, TAM_BUFFER, fichero) != NULL){
        if(!strcmp(buffer, "")){
            err = sendMessage(sc, buffer, strlen(buffer)+1);
            if( err == -1){
                printf("s> Error al enviar el número de mensajes");
                return -1;
            }
        }
    }

    // Borramos el fichero
    fclose(fichero);
    if(remove(nombre_fichero) == -1){
        perror("Error al borrar el fichero");
        return -1;
    }

    return 0;
}