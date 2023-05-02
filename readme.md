Para que solo haya un usuario conectado a la vez no se paralelizará el código del cliente
y en el servidor si alguien se intenta conectar con ese alias y hay alguien conectado ya dará error.

Creamo un thread, se encargará de estar escuchando al servidor y cuando reciba un mensaje lo imprimirá por pantalla.
Un problema podría ser que lleguen mensajes con otro tipo de operación que no sea el de enviar mensajes pero esto no se puede porque si hay un usuario conectado lo único que puede hacer es recibir mensajes que le vengan del servidor o enviar mensajes.


La ip se pasará como constante en el servidor

Cuando creamos el socket podemos ver que estamos usando el protocolo TCP al usar el parámetro SOCK_STREAM

La función de usuarios conectados tendrá un tratamiento especial para que no se límite el número de usuarios conectados que se pueden enviar.
Así, la respuesta del servidor irá implementada dentro de la función usuarios_conectados.