import subprocess
import sys
import PySimpleGUI as sg
from enum import Enum
import argparse
import socket
from contextlib import closing
import threading


class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _quit = 0
    _username = None
    _alias = None
    _date = None
    _thread = None
    _socket = None

    estado_thread = 1       # Nos indica si el thread creado tiene que seguir esperando mensajes del servidor

    # ******************** METHODS *******************
    # *
    # * @param socket - socket to read from 
    # *
    @staticmethod
    def readString(socket):
        a = ''
        while True:
            msg = socket.recv(1)
            if (msg == b'\0'):
                break;
            a += msg.decode()
        return a

    # *
    # * @param user - User name to register in the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user is already registered
    # * @return ERROR if another error occurred
    @staticmethod
    def  register(user, window):
        # Nos conectamos al servidor
        connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        connection.connect((client._server, client._port))
        
        # Vamos a enviar un mensaje al servidor con el código de la operación
        mensaje = "REGISTER"
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())        # Enviamos el mensaje al servidor

        # Enviamos los datos del usuario al servidor
        # Usuario
        mensaje = client._username
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())  

        # Alias
        mensaje = client._alias
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())

        # Fecha
        mensaje = client._date
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())

        # Recibimos la respuesta del servidor
        respuesta = client.readString(connection)
        respuesta = int(respuesta)

        if(respuesta == 0):
            window["_SERVER_"].print("REGISTER OK")
        elif(respuesta == 1):
            window["_SERVER_"].print("USERNAME IN USE")
        else:
            window["_SERVER_"].print("REGISTER FAIL") 
        
        connection.close()

        return client.RC.ERROR

    # *
    # 	 * @param user - User name to unregister from the system
    # 	 *
    # 	 * @return OK if successful
    # 	 * @return USER_ERROR if the user does not exist
    # 	 * @return ERROR if another error occurred
    @staticmethod
    def  unregister(user, window):
        # Nos conectamos al servidor
        connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        connection.connect((client._server, client._port))
        
        # Vamos a enviar un mensaje al servidor con el código de la operación
        mensaje = "UNREGISTER"
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())        # Enviamos el mensaje al servidor

        # Enviamos los datos necesarios al servidor 
        # Alias
        mensaje = client._alias
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())

        # Recibimos la respuesta del servidor
        respuesta = client.readString(connection)
        respuesta = int(respuesta)

        if(respuesta == 0):
            window["_SERVER_"].print("UNREGISTER OK")
        elif(respuesta == 1):
            window["_SERVER_"].print("USERNAME DOES NOT EXIST")
        else:
            window["_SERVER_"].print("UNREGISTER FAIL") 

        connection.close()

        return client.RC.ERROR


    # *
    # * @param user - User name to connect to the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist or if it is already connected
    # * @return ERROR if another error occurred
    @staticmethod
    def  connect(user, window):
        # Nos conectamos al servidor
        connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        connection.connect((client._server, client._port))
        
        # Vamos a enviar un mensaje al servidor con el código de la operación
        mensaje = "CONNECT"
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())        # Enviamos el mensaje al servidor

        # Enviamos el alias del usuario para identificarlo
        # Alias
        mensaje = client._alias
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())

        # Creamos un socket para el thread con el primer puerto libre
        socket_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        socket_client.bind(('', 0))  # Especifica una dirección IP vacía (localhost) y un puerto 0 para obtener un puerto disponible automáticamente
        puerto_escucha = socket_client.getsockname()[1]

        # Guardamos el socket
        client._socket = socket_client

        mensaje = str(puerto_escucha)
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())

        # Recibimos la respuesta del servidor
        respuesta = client.readString(connection)
        respuesta = int(respuesta)

        if(respuesta == 0):
            window["_SERVER_"].print("CONNECT OK")
            
            # Creamos un hilo para recibir los mensajes del servidor
            client._thread = threading.Thread(target=client.receiveMessages, args=(socket_client, window))

            # Iniciamos el thread
            client._thread.start()
            #********CONECTARSE AL THEAD********
            

        elif(respuesta == 1):
            window["_SERVER_"].print("CONNECT FAIL, USER DOES NOT EXIST")
        elif(respuesta == 2):
            window["_SERVER_"].print("USER ALREADY CONNECTED")
        else:
            window["_SERVER_"].print("CONNECT FAIL")


        return client.RC.ERROR
    
    # *
    # * @param connection - Connection socket
    # *
    # * Función que se encarga de escuchar al servidor para recibir los mensajes
    

    @staticmethod
    def receiveMessages(socket, window):
        # El sockect se queda escuchando al servidor
        socket.listen(1)

        client._estado_thread = 1
        while client._estado_thread: 
            # Aceptamos la conexión
            conn, addr = socket.accept()
            # Recibimos el código de operación
            mensaje_usuario = client.readString(conn)
            # Pasamos de una lista a un string
            mensaje_usuario = "".join(mensaje_usuario)

            print("e> " + mensaje_usuario)

            if(mensaje_usuario == "SEND_MESSAGE"):
                print("e> CONECTADO CON USUARIO")
                # Recibimos el alias del usuario que envía el mensaje
                alias = client.readString(conn)

                print("e> alias: "+alias)

                # Recibimos el identificador del mensaje
                id_mensaje = client.readString(conn)

                print("e> id: "+ id_mensaje)

                # Recibimos el mensaje
                mensaje = client.readString(conn)

                print("e> mensaje: "+mensaje)

                # Imprimimos por pantalla el mensaje
                window["_SERVER_"].print("s> MESSAGE " + id_mensaje + " FROM " + alias + "\n" + mensaje + "\nEND")
                
            elif (mensaje_usuario == "SEND_MESS_ACK"):
                # Recibimos el identificador del mensaje
                id_mensaje = client.readString(conn)

                # Imprimimos por pantalla el mensaje
                window["_SERVER_"].print("s> SEND MESSAGE " + id_mensaje + " OK")

            
   

    """
    mensaje = client.readString(socket)
            mensaje = mensaje.split("\0")
            window["_SERVER_"].print(mensaje)
    """


    # *
    # * @param user - User name to disconnect from the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist
    # * @return ERROR if another error occurred
    @staticmethod
    def  disconnect(user, window):
        # Nos conectamos al servidor
        connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        connection.connect((client._server, client._port))
        
        # Vamos a enviar un mensaje al servidor con el código de la operación
        mensaje = "DISCONNECT"
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())        # Enviamos el mensaje al servidor

        # Enviamos los datos del usuario al servidor
        # Alias
        mensaje = client._alias
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())
        
        # Recibimos la respuesta del servidor
        respuesta = client.readString(connection)
        respuesta = int(respuesta)

        if(respuesta == 0):
            window["_SERVER_"].print("DISCONNECT OK")
            
            # Cerramos el thread, para ello ponemos la variable que controla la ejecución del bucledel thread a 0
            # y forzamos el cierre del socket
            client._estado_thread = 0
            client._socket.close()
            
        elif(respuesta == 1):
            window["_SERVER_"].print("DISCONNECT FAIL / USER DOES NOT EXIST")
        elif(respuesta == 2):
            window["_SERVER_"].print("DISCONNECT FAIL / USER NOT CONNECTED")
        else:
            window["_SERVER_"].print("DISCONNECT FAIL")


        return client.RC.ERROR

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def  send(user, message, window):
        window["_SERVER_"].print("c> SEND " + user + " " + message)

        # Nos conectamos al servidor
        connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        connection.connect((client._server, client._port))
        
        # Vamos a enviar un mensaje al servidor con el código de la operación
        mensaje = "SEND"
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())        # Enviamos el mensaje al servidor

        # Enviamos los datos del usuario al servidor
        # Alias
        mensaje = client._alias
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())

        # Usuario destino
        mensaje = user
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())

        # Verificamos que el mensaje <= 255 caracteres
        if(len(message) >= 255):
            window["_SERVER_"].print("s> MESSAGE TOO LONG")
            return client.RC.ERROR
        
        # Mensaje
        mensaje = message
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())

        # Recibimos la respuesta del servidor
        respuesta = client.readString(connection)
        respuesta = int(respuesta)

        if(respuesta == 0):
            #identificador del mensaje
            id_mensaje = client.readString(connection)
            id_mensaje = int(id_mensaje)

            if(id_mensaje != -1):           # Si es distinto de -1 significa que el mensaje se ha enviado en ese momento
                window["_SERVER_"].print("s> SEND OK - MESSAGE " + str(id_mensaje))
            
        elif(respuesta == 1):
            window["_SERVER_"].print("s> SEND FAIL / USER DOES NOT EXIST")
        else:
            window["_SERVER_"].print("s> SEND FAIL")

        return client.RC.ERROR

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # * @param file    - file  to be sent

    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def  sendAttach(user, message, file, window):
        window['_SERVER_'].print("s> SENDATTACH MESSAGE OK")
        print("SEND ATTACH " + user + " " + message + " " + file)
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  connectedUsers(window):
        window['_CLIENT_'].print("c> CONNECTEDUSERS")

        # Nos conectamos al servidor
        connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        connection.connect((client._server, client._port))
        
        # Vamos a enviar un mensaje al servidor con el código de la operación
        mensaje = "CONNECTEDUSERS"
        mensaje = mensaje + "\0"
        connection.sendall(mensaje.encode())        # Enviamos el mensaje al servidor

        # Recibimos la respuesta del servidor
        respuesta = client.readString(connection)
        print("e>"+respuesta)
        respuesta = int(respuesta)

        if(respuesta == 0):
            # Recibimos la cantidad de usuarios conectados
            cantidad = client.readString(connection)
            print("e> respuesta servidor: "+cantidad)
            cantidad = int(cantidad)

            if(cantidad > 0):
                usuarios = client.readString(connection)
                usuarios = usuarios.strip()
                for i in range(cantidad-1):
                    #Recibimos los alias de los usuarios conectados y los vamos guardando en una lista
                    alias = client.readString(connection)
                    usuarios = "".join([usuarios, ",", alias.strip()])

                window["_SERVER_"].print("s> CONNECTED USERS ( " + str(cantidad) + " users connected ) OK - " + usuarios)

            
        elif (respuesta == 1):
            window["_SERVER_"].print("s> CONNECTED USERS FAIL / USER IS NOT CONNECTED")
        
        else: 
            window["_SERVER_"].print("s> CONNECTED USERS FAIL")

        return client.RC.ERROR


    @staticmethod
    def window_register():
        layout_register = [[sg.Text('Ful Name:'),sg.Input('Text',key='_REGISTERNAME_', do_not_clear=True, expand_x=True)],
                            [sg.Text('Alias:'),sg.Input('Text',key='_REGISTERALIAS_', do_not_clear=True, expand_x=True)],
                            [sg.Text('Date of birth:'),sg.Input('',key='_REGISTERDATE_', do_not_clear=True, expand_x=True, disabled=True, use_readonly_for_disable=False),
                            sg.CalendarButton("Select Date",close_when_date_chosen=True, target="_REGISTERDATE_", format='%d-%m-%Y',size=(10,1))],
                            [sg.Button('SUBMIT', button_color=('white', 'blue'))]
                            ]

        layout = [[sg.Column(layout_register, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window("REGISTER USER", layout, modal=True)
        choice = None

        while True:
            event, values = window.read()

            if (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                break

            if event == "SUBMIT":
                if(values['_REGISTERNAME_'] == 'Text' or values['_REGISTERNAME_'] == '' or values['_REGISTERALIAS_'] == 'Text' or values['_REGISTERALIAS_'] == '' or values['_REGISTERDATE_'] == ''):
                    sg.Popup('Registration error', title='Please fill in the fields to register.', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                client._username = values['_REGISTERNAME_']
                client._alias = values['_REGISTERALIAS_']
                client._date = values['_REGISTERDATE_']
                break
        window.Close()


    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;

        client._server = args.s
        client._port = args.p

        return True


    def main(argv):

        if (not client.parseArguments(argv)):
            client.usage()
            exit()

        lay_col = [[sg.Button('REGISTER',expand_x=True, expand_y=True),
                sg.Button('UNREGISTER',expand_x=True, expand_y=True),
                sg.Button('CONNECT',expand_x=True, expand_y=True),
                sg.Button('DISCONNECT',expand_x=True, expand_y=True),
                sg.Button('CONNECTED USERS',expand_x=True, expand_y=True)],
                [sg.Text('Dest:'),sg.Input('User',key='_INDEST_', do_not_clear=True, expand_x=True),
                sg.Text('Message:'),sg.Input('Text',key='_IN_', do_not_clear=True, expand_x=True),
                sg.Button('SEND',expand_x=True, expand_y=False)],
                [sg.Text('Attached File:'), sg.In(key='_FILE_', do_not_clear=True, expand_x=True), sg.FileBrowse(),
                sg.Button('SENDATTACH',expand_x=True, expand_y=False)],
                [sg.Multiline(key='_CLIENT_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True),
                sg.Multiline(key='_SERVER_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True)],
                [sg.Button('QUIT', button_color=('white', 'red'))]
            ]


        layout = [[sg.Column(lay_col, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window('Messenger', layout, resizable=True, finalize=True, size=(1000,400))
        window.bind("<Escape>", "-ESCAPE-")


        while True:
            event, values = window.Read()

            if (event in (None, 'QUIT')) or (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                sg.Popup('Closing Client APP', title='Closing', button_type=5, auto_close=True, auto_close_duration=1)
                break

            #if (values['_IN_'] == '') and (event != 'REGISTER' and event != 'CONNECTED USERS'):
             #   window['_CLIENT_'].print("c> No text inserted")
             #   continue

            if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None) and (event != 'REGISTER'):
                sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                continue

            if (event == 'REGISTER'):
                client.window_register()

                if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                window['_CLIENT_'].print('c> REGISTER ' + client._alias)
                client.register(client._alias, window)

            elif (event == 'UNREGISTER'):
                window['_CLIENT_'].print('c> UNREGISTER ' + client._alias)
                client.unregister(client._alias, window)


            elif (event == 'CONNECT'):
                window['_CLIENT_'].print('c> CONNECT ' + client._alias)
                client.connect(client._alias, window)


            elif (event == 'DISCONNECT'):
                window['_CLIENT_'].print('c> DISCONNECT ' + client._alias)
                client.disconnect(client._alias, window)


            elif (event == 'SEND'):
                window['_CLIENT_'].print('c> SEND ' + values['_INDEST_'] + " " + values['_IN_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_INDEST_'] != 'User' and values['_IN_'] != 'Text') :
                    client.send(values['_INDEST_'], values['_IN_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message>")


            elif (event == 'SENDATTACH'):

                window['_CLIENT_'].print('c> SENDATTACH ' + values['_INDEST_'] + " " + values['_IN_'] + " " + values['_FILE_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_FILE_'] != '') :
                    client.sendAttach(values['_INDEST_'], values['_IN_'], values['_FILE_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message> <attachedFile>")


            elif (event == 'CONNECTED USERS'):
                window['_CLIENT_'].print("c> CONNECTEDUSERS")
                client.connectedUsers(window)



            window.Refresh()

        window.Close()


if __name__ == '__main__':
    client.main([])
    print("+++ FINISHED +++")
