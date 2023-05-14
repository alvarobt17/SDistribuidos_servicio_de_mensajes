#ifndef __IMPLEMENTACION_H__
#define __IMPLEMENTACION_H__

#define MAX_SIZE 255

int registrar(char usuario[MAX_SIZE],char alias[MAX_SIZE], char fecha[MAX_SIZE]);

int baja(char alias[MAX_SIZE]);

int conectar(char alias[MAX_SIZE], int puerto, char ip[MAX_SIZE]);

int desconectar(char alias[MAX_SIZE]);

int enviar(char alias[MAX_SIZE], char destino[MAX_SIZE], char mensaje[MAX_SIZE], int id);

int error_enviar_mensaje(char alias[MAX_SIZE], char destino[MAX_SIZE], char mensaje[MAX_SIZE], int id);

int usuarios_conectados(int sc);

int mensajes_pendientes(char alias[MAX_SIZE], int puerto, char ip[MAX_SIZE]);



#endif // __IMPLEMENTACION_H__
