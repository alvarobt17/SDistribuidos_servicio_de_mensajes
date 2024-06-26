#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>



#include "implementacion.h"
#include "lines.h"


#define TAM_BUFFER 1024

int registrar(char usuario[MAX_SIZE],char alias[MAX_SIZE], char fecha[MAX_SIZE]){

    // Cada usuario tendrá su propio txt en el que se guardarán todos sus datos y los mensajes que tenga pendientes de enviar
    // Comprobamos que el usuario no exista ya
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/%s.txt", alias);

    fichero = fopen(nombre_fichero, "r");
    if (fichero != NULL) {
        fclose(fichero);
        return 1; // USERNAME IN USE
    }

    fichero = fopen(nombre_fichero, "w");
    if (fichero == NULL) {
        return 2; // Error al abrir el archivo
    }

    //Guardamos los datos del usuario en el fichero
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

    // Comprobamos que el usuario no está conectado 
    desconectar(alias);

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
    
    // INCLUIMOS AL USUARIO EN CONNECTED_USERS

    sprintf(nombre_fichero, "datos/connected_users.txt");

    fichero = fopen(nombre_fichero, "r");
    if(fichero == NULL){
        // El archivo no existe
        // Creamos el archivo
        fichero = fopen(nombre_fichero, "w");
        if (fichero == NULL) {
            // Error al abrir el archivo
            return 3;
        }
        fprintf(fichero, "%d\n", 0);

        fclose(fichero);
    }

    fichero = fopen(nombre_fichero, "r+");
    if (fichero == NULL) {
        // Error al abrir el archivo
        return 3;
    }

    // Leemos la primera línea para ver el número de usuarios conectados
    fseek(fichero, 0, SEEK_SET);
    if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
        // Error al leer el fichero
        fclose(fichero);
        return 1;
    }
    int conectado = atoi(buffer);
    
    conectado++;

    // Modificamos los usuarios conectados
    fseek(fichero, 0, SEEK_SET);
    fprintf(fichero, "%d\n", conectado);
    for(int i = 0; i < conectado-1; i++){
        if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
            // Error al leer el fichero
            fclose(fichero);
            return 3;
        }
    }
    // Escribimos el nombre del usuario
    fprintf(fichero, "%s\n", alias);

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

    // Comprobamos que el usuario no esté ya desconectado
    int cont = atoi(buffer);
    if(cont == 0){
        // El usuario ya está desconectado
        fclose(fichero);
        return 2;           // USER NOT CONNECTED
    }

    // Cambiamos el estado del usuario a desconectado y borramos el puerto y la ip
    // Creamos un fichero temporal para guardar los datos
    FILE *fichero_temp;
    char nombre_fichero_temp[MAX_SIZE];
    sprintf(nombre_fichero_temp, "datos/%s_aux.txt", alias);

    fichero_temp = fopen(nombre_fichero_temp, "w+");
    if(fichero_temp == NULL){
        // Error al abrir el archivo
        fclose(fichero);
        return 3;
    }

    // Copiamos los datos del fichero original (usuario, alias y fecha) al temporal
    fseek(fichero, 0, SEEK_SET);

    for(int i = 0; i < 3; i++){
        if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
            // Error al leer el fichero
            fclose(fichero);
            fclose(fichero_temp);
            return 3;
        }
        fprintf(fichero_temp, "%s", buffer);
    }

    fprintf(fichero_temp, "%d\n%s\n%s", 0, "NULL", "NULL");

    // Borramos el fichero original y cambiamos de nombre al temporal
    fclose(fichero);
    remove(nombre_fichero);
    rename(nombre_fichero_temp, nombre_fichero);
    fclose(fichero_temp);

    // ELIMINAR AL USUARIO DE CONNECTED_USERS

    sprintf(nombre_fichero, "datos/connected_users.txt");

    fichero = fopen(nombre_fichero, "r+");
    if(fichero == NULL){
        // Escribimos en la primera línea 0, número usuarios conectados
        fprintf(fichero, "%d\n", 0);
        printf("Fichero vacío\n");
    }

    // Leemos la primera línea para ver el número de usuarios conectados
    fseek(fichero, 0, SEEK_SET);
    if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
        // Error al leer el fichero
        fclose(fichero);
        return 1;
    }
    int conectado = atoi(buffer);

    if(conectado > 0){
        // Creamos un nuevo fichero para ir copiando todos los usuarios menos el que queremos desconectar
        FILE *fichero_aux;
        char nombre_fichero_aux[MAX_SIZE];
        sprintf(nombre_fichero_aux, "datos/connected_users_aux.txt");

        fichero_aux = fopen(nombre_fichero_aux, "w");

        // Copiamos el número de usuarios conectados
        fprintf(fichero_aux, "%d\n", conectado-1);

        for(int i = 0; i < conectado; i++){
            if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
                // Error al leer el fichero
                fclose(fichero);
                return 1;
            }

            int cmp = strncmp(buffer, alias, strlen(alias));
            if( cmp != 0){  // Si el usuario está conectado no lo incluimos en el nuevo fichero
                fprintf(fichero_aux, "%s", buffer);
            }
        }

        fclose(fichero);
        fclose(fichero_aux);

        // Borramos el fichero original
        remove(nombre_fichero);

        // Renombramos el fichero auxiliar
        rename(nombre_fichero_aux, nombre_fichero);

    }else{
        fclose(fichero);
    }

    return 0;

}

int enviar(char alias[MAX_SIZE], char destino[MAX_SIZE], char mensaje[MAX_SIZE], int id){
    char mensaje_local[TAM_BUFFER]; 

    //Comprobamos si el usuario está conectado
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/%s.txt", destino);
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

    // Comprobamos si el usuario está conectado
    int connectado = atoi(buffer);
    if(connectado == 0){
        // El usuario no está conectado, guardamos el mensaje para enviarlo cuando se conecte
        fclose(fichero);

        sprintf(mensaje_local, "SEND_MESSAGE;%d;%s;%s", id, alias, mensaje); 

        // Guardamos el mensaje en el fichero de mensajes pendientes y si no existe lo creamos
        sprintf(nombre_fichero, "datos/%s_mensajes.txt", destino);
        fichero = fopen(nombre_fichero, "a+");
        if(fichero == NULL){
            // Error al abrir el fichero
            fclose(fichero);
            return 1;
        } 

        //Vamos a la última línea del fichero y escribimos el mensaje nuevo
        fseek(fichero, 0, SEEK_END);
        fprintf(fichero, "%s\n", mensaje_local);

        fclose(fichero);

    }else{          //Usuario conectado en este momento

        // Leemos la 5 línea para ver el puerto del usuario
        if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
            // Error al leer el fichero
            fclose(fichero);
            return 1;
        }

        int puerto = atoi(buffer);

        // Leemos la 6 línea para ver la ip del usuario
        if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
            // Error al leer el fichero
            fclose(fichero);
            return 1;
        }

        char ip[TAM_BUFFER];
        strcpy(ip, buffer);

        // Enviamos el mensaje al usuario
        int smens;
        if((smens = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("s> Error al crear el socket\n");
            fclose(fichero);
            return 1;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(puerto);
        server_addr.sin_addr.s_addr = inet_addr(ip);

        if(connect(smens, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
            printf("s> Error al conectar con el usuario\n");
            fclose(fichero);
            return 1;
        }

        // Enviamos el código de la operación
        char operacion[MAX_SIZE];
        sprintf(operacion, "%s", "SEND_MESSAGE");
        if(sendMessage(smens, operacion, strlen(operacion)+1) == -1){
            printf("s> Error al enviar el código de la operación\n");
            fclose(fichero);
            return 1;
        }

        // Enviamos el alias del usuario que envíe el mensaje
        if(sendMessage(smens, alias, strlen(alias)+1) == -1){
            printf("s> Error al enviar el alias del usuario\n");
            fclose(fichero);
            return 1;
        }

        // Enviamos el identificador del mensaje
        char enviar[MAX_SIZE];
        sprintf(enviar, "%d", id);
        if(sendMessage(smens,enviar, strlen(enviar)+1) == -1){
            printf("s> Error al enviar el identificador del mensaje\n");
            fclose(fichero);
            return 1;
        }

        // Enviamos el mensaje
        if(sendMessage(smens, mensaje, strlen(mensaje)+1) == -1){
            printf("s> Error al enviar el mensaje\n");
            fclose(fichero);
            return 1;
        }

        printf("s> SEND MESSAGE %d FROM %s TO %s\n", id, alias, destino);

        fclose(fichero);
        close(smens);

        //Enviamos confirmación al usuario que envió el mensaje
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

        // Comprobamos si el usuario está conectado
        int conectado = atoi(buffer);
        if(conectado == 1){             //Usuario conectado actualmente

            // Leemos la 5 línea para ver el puerto del usuario
            if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
                // Error al leer el fichero
                fclose(fichero);
                return 1;
            }

            int puerto = atoi(buffer);

            // Leemos la 6 línea para ver la ip del usuario
            if(fgets(buffer, TAM_BUFFER, fichero) == NULL){
                // Error al leer el fichero
                fclose(fichero);
                return 1;
            }

            char ip[TAM_BUFFER];
            strcpy(ip, buffer);

            // Enviamos el mensaje al usuario
            int smens;
            if((smens = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                printf("s> Error al crear el socket\n");
                fclose(fichero);
                return 1;
            }

            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(puerto);
            server_addr.sin_addr.s_addr = inet_addr(ip);

            if(connect(smens, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
                printf("s> Error al conectar con el usuario\n");
                fclose(fichero);
                return 1;
            }

            // Enviamos el código de la operación
            char operacion[MAX_SIZE];
            sprintf(operacion, "%s", "SEND_MESS_ACK");
            if(sendMessage(smens, operacion, strlen(operacion)+1) == -1){
                printf("s> Error al enviar el código de la operación\n");
                fclose(fichero);
                return 1;
            }

            // Enviamos el identificador del mensaje
            char enviar[MAX_SIZE];
            sprintf(enviar, "%d", id);
            if(sendMessage(smens, enviar, strlen(enviar)+1) == -1){
                printf("s> Error al enviar el alias del usuario\n");
                fclose(fichero);
                return 1;
            }

            fclose(fichero);
            close(smens);
        }else{      // El usuario no está conectado actualmente
            // Guardamos el mensjaje de confirmación
            sprintf(mensaje_local, "SEND_MESS_ACK;%d", id); 

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
            fprintf(fichero, "%s\n", mensaje_local);

            fclose(fichero);

        }

    }

    return 0;

}


int usuarios_conectados(int sc){
   
    FILE *fichero;
    char nombre_fichero[MAX_SIZE];
    sprintf(nombre_fichero, "datos/connected_users.txt");

    int resultado;

    fichero = fopen(nombre_fichero, "r");
    if (fichero == NULL) {
        printf("s> No se pudo abrir el fichero.\n");
        resultado = -1;
    }else{
        resultado = 0;
    }

    char buffer[TAM_BUFFER];
    fgets(buffer, TAM_BUFFER, fichero);

    int num_connect = atoi(buffer);

    if (num_connect <= 0) {
        fclose(fichero);
        resultado = -1;
    }else{
        resultado = 0;
    }

    //Enviamos el resultado de la operación (0)
    char mensaje[MAX_SIZE];
    sprintf(mensaje, "%d", resultado);
    if(sendMessage(sc, mensaje, strlen(mensaje)+1) == -1){
        printf("s> Error al enviar el resultado de la operación");
        fclose(fichero);
        return -1;
    }

    if(resultado == -1){
        return -1;
    }

    // Enviamos la cantidad de personas conectadas al cliente
    sprintf(mensaje, "%d", num_connect);
    if(sendMessage(sc, mensaje, strlen(mensaje)+1) == -1){
        printf("s> Error al enviar el número de personas conectadas");
        fclose(fichero);
        return -1;
    }

    int err;

    //Envío de personas conectadas
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

int mensajes_pendientes(char destino[MAX_SIZE], int puerto, char ip[MAX_SIZE]){     
    FILE *fichero;
    char nombre_fichero[TAM_BUFFER];
    sprintf(nombre_fichero, "datos/%s_mensajes.txt", destino);
    // Leemos el fichero y enviamos los mensajes
    char buffer[TAM_BUFFER];
    char alias[MAX_SIZE];

    while( (fichero = fopen(nombre_fichero, "r+")) != NULL){       //Si existe significa que tenemos mensajes pendientes
        fseek(fichero, 0, SEEK_SET);
        fgets(buffer, TAM_BUFFER, fichero);

        //Abrimos el socket para enviar los mensajes 

        // Conectamos el socket mediante la ip y el puerto dados
        int sock_destino = socket(AF_INET, SOCK_STREAM, 0);
        if(sock_destino == -1){
            printf("s> Error al crear el socket");
            fclose(fichero);
            return 1;
        }

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(puerto);
        server.sin_addr.s_addr = inet_addr(ip);

        if(connect(sock_destino, (struct sockaddr *)&server, sizeof(server)) == -1){
            printf("s> Error al conectar el socket\n");
            fclose(fichero);
            return -1;
        }

        // Obtenemos el tipo de mensaje,id, alias y el mensaje a enviar
        char tipo_op[MAX_SIZE];
        char *token = strtok(buffer, ";");
        strcpy(tipo_op, token);
        token = strtok(NULL, ";");
        int id = atoi(token);

        if(strcmp(tipo_op, "SEND_MESSAGE") == 0){

            token = strtok(NULL, ";");
            char usuario[MAX_SIZE];
            strcpy(usuario, token);
            token = strtok(NULL, ";");
            char mensaje[MAX_SIZE];
            strcpy(mensaje,token);

            // Enviamos el tipo de operación
            if(sendMessage(sock_destino, tipo_op, strlen(tipo_op)+1) == -1){
                printf("s> Error al enviar el tipo de operación");
                return -1;
            }
        
            // Enviamos el alias del usuario que envió el mensaje
            if(sendMessage(sock_destino, usuario, strlen(usuario)+1) == -1){
                printf("s> Error al enviar el alias del usuario\n");
                fclose(fichero);
                return 1;
            }

            // Enviamos el identificador del mensaje
            char envio[MAX_SIZE];
            sprintf(envio, "%d", id);
            if(sendMessage(sock_destino, envio, strlen(envio)+1) == -1){
                printf("s> Error al enviar el identificador del mensaje\n");
                fclose(fichero);
                return 1;
            }

            // Enviamos el mensaje

            mensaje[strlen(mensaje)-1] = '\0';
            if(sendMessage(sock_destino, mensaje, strlen(mensaje)+1) == -1){
                printf("s> Error al enviar el mensaje\n");
                fclose(fichero);
                return 1;
            }


            printf("s> SEND MESSAGE %d FROM %s TO %s STORED\n", id, usuario, destino);

            close(sock_destino);


            //CONFIRMACIÓN DEL MENSAJE

            // Accedemos al fichero del usuario que envió el mensaje
            char fich_usuario[TAM_BUFFER];
            sprintf(fich_usuario, "datos/%s.txt", usuario);

            // Accedemos a su fichero para ver su estado
            FILE *fichero_usuario = fopen(fich_usuario, "r+");
            if(fichero_usuario == NULL){
                return -1;
            }
            
            // Leemos la 4 línea para ver el estado del usuario
            char buffer[TAM_BUFFER];
            int contandor = 1;
            for(int i = 0; i < 4; i++){
                if(fgets(buffer, TAM_BUFFER, fichero_usuario) == NULL){
                    // Error al leer el fichero
                    fclose(fichero_usuario);
                    return 1;
                }
            }

            int conectado = atoi(buffer);

            if(conectado == 1){     //El usuario está conectado, le enviamos el mensaje de confirmación
                // Leemos la 5 línea para ver el puerto del usuario
                if(fgets(buffer, TAM_BUFFER, fichero_usuario) == NULL){
                    // Error al leer el fichero
                    fclose(fichero_usuario);
                    return 1;
                }

                int puerto_usuario = atoi(buffer);

                // Leemos la 6 línea para ver la ip del usuario
                if(fgets(buffer, TAM_BUFFER, fichero_usuario) == NULL){
                    // Error al leer el fichero
                    fclose(fichero_usuario);
                    return 1;
                }
                char ip_usuario[TAM_BUFFER];
                strcpy(ip_usuario, buffer);

                fclose(fichero_usuario);

                // Enviamos el mensaje al usuario
                int smens = socket(AF_INET, SOCK_STREAM, 0);
                if(smens == -1){
                    printf("s> Error al crear el socket\n");
                    fclose(fichero_usuario);
                    return 1;
                }

                struct sockaddr_in server_addr;
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(puerto_usuario);
                server_addr.sin_addr.s_addr = inet_addr(ip_usuario);

                if(connect(smens, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
                    printf("s> Error al conectar con el usuario\n");
                    fclose(fichero_usuario);
                    return -1;
                }
    
                // Enviamos el código de la operación
                sprintf(tipo_op, "SEND_MESS_ACK");
                if(sendMessage(smens, tipo_op, strlen(tipo_op)+1) == -1){
                    printf("s> Error al enviar el código de la operación\n");
                    fclose(fichero_usuario);
                    return 1;
                }

                // Enviamos el identificador
                char envio[MAX_SIZE];
                sprintf(envio, "%d", id);
                if(sendMessage(smens, envio, strlen(envio)+1) == -1){
                    printf("s> Error al enviar el identificador del mensaje\n");
                    fclose(fichero_usuario);
                    return 1;
                }

                close(smens);

            }else{      //El usuario no está conectado

                fclose(fichero_usuario);
                //Guardamos el mensaje de confirmación en el fichero de mensajes pendientes
                char mensaje_guardar[MAX_SIZE], fich_remitente[TAM_BUFFER];
                sprintf(mensaje_guardar,"SEND_MESS_ACK;%d", id);

                // Guardamos el mensaje en el fichero de mensajes pendientes y si no existe lo creamos
                sprintf(fich_remitente, "datos/%s_mensajes.txt", usuario);
                FILE *fichero_mensajes;
                fichero_mensajes = fopen(fich_remitente, "a+");
                if(fichero_mensajes == NULL){
                    // Error al abrir el fichero
                    return 1;
                } 

                //Vamos a la última línea del fichero y escribimos el mensaje nuevo
                fseek(fichero_mensajes, 0, SEEK_END);
                fprintf(fichero_mensajes, "%s\n", mensaje_guardar);

                fclose(fichero_mensajes);
            }



        }else if(strcmp(tipo_op, "SEND_MESS_ACK") == 0){      //Confirmaciones de mensajes 

            // Enviamos el tipo de operación
            if(sendMessage(sock_destino, tipo_op, strlen(tipo_op)+1) == -1){
                printf("s> Error al enviar el tipo de operación");
                return -1;
            }

            // Enviamos el identificador del mensaje
            char envio[MAX_SIZE];
            sprintf(envio, "%d", id);
            if(sendMessage(sock_destino, envio, strlen(envio)+1) == -1){
                printf("s> Error al enviar el identificador del mensaje\n");
                fclose(fichero);
                return 1;
            }
            
            close(sock_destino);
            
        }

        // Comprobamos si hay más mensajes
        fseek(fichero, 0, SEEK_SET);
        fgets(buffer, TAM_BUFFER, fichero);     //Línea con el mensaje que acabamos de enviar

        if(fgets(buffer, TAM_BUFFER, fichero) != NULL){
            
            FILE *fichero_aux;
            char nombre_fichero_aux[MAX_SIZE];
            sprintf(nombre_fichero_aux, "datos/%s_mensajes_aux.txt", destino);

            fichero_aux = fopen(nombre_fichero_aux, "w+");
            if(fichero_aux == NULL){
                // Error al abrir el archivo
                fclose(fichero);
                return 1;
            }

            fprintf(fichero_aux, "%s", buffer);
            // Copiamos los demás mensajes si existen
            while(fgets(buffer, TAM_BUFFER, fichero) != NULL){
                fprintf(fichero_aux, "%s", buffer);
            }

            //Cerramos ficheros
            fclose(fichero);
            fclose(fichero_aux); 
            // Renombramos el fichero auxiliar y borramos el original
            remove(nombre_fichero);
            rename(nombre_fichero_aux, nombre_fichero);
 
        }else{
            fclose(fichero);

            remove(nombre_fichero);
        }
    
    }


    return 0;
}