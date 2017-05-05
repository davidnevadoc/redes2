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


/*Variable global de contexto SSL*/
SSL_CTX *context = NULL;

typedef SSL* pSSL;
pSSL s_sockets[1024]={0};

void inicializar_nivel_SSL(){
    SSL_load_error_strings();
    SSL_library_init();
}

int fijar_contexto_SSL(char* pkey, char* cert){
	context = SSL_CTX_new(SSLv23_method());
	if(!context){
		perror("No context");
		return SSLERR_NOCTX;
	}
	if(!SSL_CTX_load_verify_locations(context, "./certs/ca/cacert.pem", NULL)){
		perror("Error verify locations");
		return SSLERR_LOADVERIFY;
	}
	SSL_CTX_set_default_verify_paths(context);

	SSL_CTX_use_certificate_chain_file(context, cert);

	if (SSL_CTX_use_PrivateKey_file(context, pkey, SSL_FILETYPE_PEM) != 1){
		perror("Error private key");
		return SSLERR_INVALIDKEY;
	}

	SSL_CTX_set_verify(context, SSL_VERIFY_PEER, NULL);
	return SSL_OK;

}

int conectar_canal_seguro_SSL(int socket){
    s_sockets[socket] = SSL_new(context);

    if(SSL_set_fd(s_sockets[socket], socket) != 1) return SSLERR_SETFD;
    if(SSL_connect(s_sockets[socket]) != 1) return SSLERR_FAIL;

    return SSL_OK;
}

int aceptar_canal_seguro_SSL(int socket){
    s_sockets[socket] = SSL_new(context);

    if(SSL_set_fd(s_sockets[socket], socket)!=1) return SSLERR_SETFD;
    if(SSL_accept(s_sockets[socket]) != 1) return SSLERR_FAIL;

    return SSL_OK;
}

int evaluar_post_connectar_SSL(int socket){
	if(!s_sockets[socket])  return SSLERR_FAIL;

	/*Vemos si hay certificados y es valido*/    
	return SSL_get_peer_certificate(s_sockets[socket]) && SSL_get_verify_result(s_sockets[socket]);
} 

int enviar_datos_SSL(int socket, void* buf){
	if(!s_sockets[socket]) return SSLERR_FAIL;
	return SSL_write(s_sockets[socket], buf, sizeof(buf));
}

int recibir_datos_SSL(int socket, void* buf, int max){
	if(!s_sockets[socket]) return SSLERR_FAIL;
	return SSL_read(s_sockets[socket], buf, max);
}

void cerrar_canal_SSL(int socket){
	if(!s_sockets[socket]) return;
  SSL_shutdown(s_sockets[socket]);
  SSL_free(s_sockets[socket]);
  s_sockets[socket] = NULL;
}

