/**
 * @brief Librería de manejo de la capa SSL
 * @file G-2302-05-P3-ssl_tools.h
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#ifndef SSL_H
#define SSL_H

#define SSL_OK 0
#define SSL_ERR -1

/*Path certificados*/
#define CA_CERT "./certs/ca/cacert.pem"
#define CLIENT_CERT "./certs/cliente.pem"
#define CLIENT_KEY "./certs/client/clientkey.pem"
#define SERVER_CERT "./certs/servidor.pem"
#define SERVER_KEY "./certs/server/serverkey.pem"

#define MAX_MSG_SSL 50000


/**
*@brief Función que se encarga de realizar todas las llamadas necesarias para 
*que la aplicación pueda usar la capa segura SSL.
*/
void inicializar_nivel_SSL();

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
int fijar_contexto_SSL(char* pkey, char* cert, char* ca_cert);

/**
*@brief Función que dado un contexto SSL y un descriptor de socket se *encarga de obtener un canal seguro SSL iniciando el proceso de 
*handshake con el otro extremo.
*@param socket: socket asociado a la conexión
*@return SSL_OK, SSL_ERR
*/
int conectar_canal_seguro_SSL(int socket);

/**
*@brief Función que dado un contexto SSL y un descriptor de socket  se *encarga de bloquear la aplicación, que se quedará esperando hasta 
*recibir un handshake por parte del cliente.
*@param socket: socket asociado a la conexión
*@return SSL_OK, SSL_ERR
*/
int aceptar_canal_seguro_SSL(int socket);

/**
*@brief Función que comprueba una vez realizado el handshake que el 
*canal de comunicación se puede considerar seguro.
*@param socket: socket asociado a la conexión
*@return SSL_OK, SSL_ERR
*/
int evaluar_post_connectar_SSL(int socket);


/**
*@brief Función que envía datos a través del canal seguro.
*@param socket: socket asociado a la conexión
*@param buf: buffer que se quiere enviar
*@return SSL_OK, SSL_ERR
*/
int enviar_datos_SSL(int socket, void* buf);

/**
*@brief Función que recibe datos a través del canal seguro.
*@param socket: socket asociado a la conexión
*@param buff: buffer rellenado con los datos que se reciben
*@return SSL_OK, SSL_ERR
*/
int recibir_datos_SSL(int socket, void* buf);

/**
*@brief Función que libera todos los recursos y cierra el canal de *comunicación seguro creado previamente.
*@param socket: socket asociado a la conexión
*/
void cerrar_canal_SSL(int socket);


#endif
