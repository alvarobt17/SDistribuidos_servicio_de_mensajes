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
    if(fichero != NULL){
        // El usuario ya existe
        fclose(fichero);
        return 1;           // USERNAME IN USE
    }

    //Guardamos los datos del usuario en su fichero
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
    int contandor = 1;
    while(fgets(buffer, TAM_BUFFER, fichero)!= NULL){
        if(contandor == 4){
            break;
        }
        contandor++;
    }

    // Comprobamos que el usuario no esté ya conectado
    int cont = atoi(buffer);
    if(cont == 1){
        // El usuario ya está conectado
        fclose(fichero);
        return 2;           // USER ALREADY CONNECTED
    }

    // Cambiamos el estado del usuario a conectado
    fprintf(fichero, "%d", 1);

    // Cambiamos la ip y el puerto del usuario

    // Leemos la 5 línea para ver la ip del usuario
    

    //PROBAR PRIMERO EN UN ARCHIVO



}

int desconectar(char alias[MAX_SIZE]);

int enviar(char alias[MAX_SIZE], char mensaje[MAX_SIZE]);

int usuarios_conectados(int sc);