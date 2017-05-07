/**
 * @brief Librería de manejo de la capa SSL
 * @file G-2302-05-P3-ssl_tools.c
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */
#include "../includes/G-2302-05-P3-ssl_tools.h"
#include <stdio.h>         
#include <stdlib.h>
#include <netdb.h>
#include <pthread.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

SSL_CTX *context=NULL;
typedef SSL *pssl;
pssl s[1024]={0};

/**
*@brief Función que se encarga de realizar todas las llamadas necesarias para 
*que la aplicación pueda usar la capa segura SSL.
*/
void inicializar_nivel_SSL(){
    SSL_load_error_strings();
    SSL_library_init();
}

/**
*@brief Función que se encarga de inicializar correctamente el contexto que 
*será utilizado para la creación de canales seguros mediante SSL. 
*Deberá recibir información sobre las rutas a los certificados y claves con 
*los que vaya a trabajar la aplicación.
*@param pkey: clave privada
*@param cert: path del certificado
*@param ca_cert: path del certificado de la CA
*@return SSL_OK, SSL_ERR
*/
int fijar_contexto_SSL(char* pkey, char* cert, char* ca_cert){
	context = SSL_CTX_new(SSLv23_method());
	if(!context){
		perror("No context");
		return SSL_ERR;
	}
	if(SSL_CTX_load_verify_locations(context, ca_cert, NULL) != 1){
		perror("Error verify locations");
		return SSL_ERR;
	}
	SSL_CTX_set_default_verify_paths(context);

	SSL_CTX_use_certificate_chain_file(context, cert);

	if (SSL_CTX_use_PrivateKey_file(context, pkey, SSL_FILETYPE_PEM) != 1){
		perror("Error private key");
		return SSL_ERR;
	}

	SSL_CTX_set_verify(context, SSL_VERIFY_PEER, NULL);

	return SSL_OK;
}

/**
*@brief Función que dado un contexto SSL y un descriptor de socket se *encarga de obtener un canal seguro SSL iniciando el proceso de 
*handshake con el otro extremo.
*@param socket: socket asociado a la conexión
*@return SSL_OK, SSL_ERR
*/
int conectar_canal_seguro_SSL(int socket){
    s[socket] = SSL_new(context);
    if(SSL_set_fd(s[socket], socket) != 1) return SSL_ERR;
    if(SSL_connect(s[socket]) != 1) return SSL_ERR;/*falla aqui*/

    return SSL_OK;
}

/**
*@brief Función que dado un contexto SSL y un descriptor de socket  se *encarga de bloquear la aplicación, que se quedará esperando hasta 
*recibir un handshake por parte del cliente.
*@param socket: socket asociado a la conexión
*@return SSL_OK, SSL_ERR
*/
int aceptar_canal_seguro_SSL(int socket){
	s[socket] = SSL_new(context);
    if(SSL_set_fd(s[socket], socket)!=1) return SSL_ERR;
    if(SSL_accept(s[socket]) != 1) return SSL_ERR;

    return SSL_OK;
}

/**
*@brief Función que comprueba una vez realizado el handshake que el 
*canal de comunicación se puede considerar seguro.
*@param socket: socket asociado a la conexión
*@return SSL_OK, SSL_ERR
*/
int evaluar_post_connectar_SSL(int socket){
	/*Vemos si hay certificados y es valido*/    
	return SSL_get_peer_certificate(s[socket]) && SSL_get_verify_result(s[socket]);
} 

/**
*@brief Función que envía datos a través del canal seguro.
*@param socket: socket asociado a la conexión
*@param buf: buffer que se quiere enviar
*@return SSL_OK, SSL_ERR
*/
int enviar_datos_SSL(int socket, void* buf){
	if(!buf) return SSL_ERR;
	return SSL_write(s[socket], buf, strlen(buf)+1);
}

/**
*@brief Función que recibe datos a través del canal seguro.
*@param socket: socket asociado a la conexión
*@param buff: buffer rellenado con los datos que se reciben
*@return >0 nuemero de bytes leidos,<0 error o SSL_ERR
*/
int recibir_datos_SSL(int socket, void* buf){
	if(!buf) return SSL_ERR;
	return SSL_read(s[socket], buf, MAX_MSG_SSL);
}

/**
*@brief Función que libera todos los recursos y cierra el canal de *comunicación seguro creado previamente.
*@param socket: socket asociado a la conexión
*/
void cerrar_canal_SSL(int socket){
  SSL_shutdown(s[socket]);
  SSL_free(s[socket]);
  SSL_CTX_free(context);
  close(socket);
}

