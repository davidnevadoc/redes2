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

/*Macros de error*/
#define SSLERR_LOADVERIFY -2
#define SSLERR_INVALIDKEY -3
#define SSLERR_INVALIDCERT -4
#define SSLERR_SETFD -5
#define SSLERR_FAIL -6


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
*/
SSL_CTX* fijar_contexto_SSL(char* pkey, char* cert);

/**
*@brief Función que dado un contexto SSL y un descriptor de socket se *encarga de obtener un canal seguro SSL iniciando el proceso de 
*handshake con el otro extremo.
*/
SSL* conectar_canal_seguro_SSL(int socket, SSL_CTX *context);

/**
*@brief Función que dado un contexto SSL y un descriptor de socket  se *encarga de bloquear la aplicación, que se quedará esperando hasta 
*recibir un handshake por parte del cliente.
*/
SSL* aceptar_canal_seguro_SSL(int socket, SSL_CTX *context);

/**
*@brief Función que comprueba una vez realizado el handshake que el 
*canal de comunicación se puede considerar seguro.
*/
int evaluar_post_connectar_SSL(int socket, SSL *ssl);


/**
*@brief Función que envía datos a través del canal seguro.
*/
int enviar_datos_SSL(int socket, void* buf, SSL *ssl);

/**
*@brief Función que recibe datos a través del canal seguro.
*/
int recibir_datos_SSL(int socket, void* buf, SSL *ssl);

/**
*@brief Función que libera todos los recursos y cierra el canal de *comunicación seguro creado previamente.
*/
void cerrar_canal_SSL(int socket, SSL *ssl, SSL_CTX *context);


#endif
