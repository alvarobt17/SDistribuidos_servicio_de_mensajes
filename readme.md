Para que solo haya un usuario conectado a la vez no se paralelizará el código del cliente
y en el servidor si alguien se intenta conectar con ese alias y hay alguien conectado ya dará error.

Creamo un thread, se encargará de estar escuchando al servidor y cuando reciba un mensaje lo imprimirá por pantalla.
Un problema podría ser que lleguen mensajes con otro tipo de operación que no sea el de enviar mensajes pero esto no se puede porque si hay un usuario conectado lo único que puede hacer es recibir mensajes que le vengan del servidor o enviar mensajes.