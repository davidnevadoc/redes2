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


void inicializar_nivel_SSL(){
    SSL_load_error_strings();
    SSL_library_init();
}

SSL_CTX* fijar_contexto_SSL(char* pkey, char* cert){
	SSL_CTX *context;
	context = SSL_CTX_new(SSLv23_method());
	if(!context){
		perror("No context");
		return NULL;
	}
	if(SSL_CTX_load_verify_locations(context, CA_CERT, NULL) != 1){
		perror("Error verify locations");
		return NULL;
	}
	SSL_CTX_set_default_verify_paths(context);

	SSL_CTX_use_certificate_chain_file(context, cert);

	if (SSL_CTX_use_PrivateKey_file(context, pkey, SSL_FILETYPE_PEM) != 1){
		perror("Error private key");
		return NULL;
	}

	SSL_CTX_set_verify(context, SSL_VERIFY_PEER, NULL); /****************/

	return context;

}

SSL* conectar_canal_seguro_SSL(int socket, SSL_CTX *context){
    SSL *ssl = SSL_new(context);

    if(SSL_set_fd(ssl, socket) != 1) return NULL;
    if(SSL_connect(ssl) != 1) return NULL;

    return ssl;
}

SSL* aceptar_canal_seguro_SSL(int socket, SSL_CTX *context){
    SSL *ssl = SSL_new(context);

    if(SSL_set_fd(ssl, socket)!=1) return NULL;
    if(SSL_accept(ssl) != 1) return NULL;

    return ssl;
}

int evaluar_post_connectar_SSL(int socket, SSL *ssl){
	/*Vemos si hay certificados y es valido*/    
	return SSL_get_peer_certificate(ssl) && SSL_get_verify_result(ssl);
} 

int enviar_datos_SSL(int socket, void* buf, SSL *ssl){
	if(!buf) return -1;
	return SSL_write(ssl, buf, strlen(buf)+1);
}

int recibir_datos_SSL(int socket, void* buf, SSL *ssl){
	if(!buf) return -1;
	return SSL_read(ssl, buf, MAX_MSG_SSL); /*no se si esto o sizeof(buf)*/
}

void cerrar_canal_SSL(int socket, SSL *ssl, SSL_CTX *context){
  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(context);
  close(socket);
}

