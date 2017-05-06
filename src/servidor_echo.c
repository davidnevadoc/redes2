/**
 * @brief Servidor echo SSL
 * @file servidor_echo.c
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#include "../includes/G-2302-05-P3-ssl_tools.h"
#include "../includes/G-2302-05-P2-tcp_tools.h"
#include <stdio.h>

/**
*@brief Funcion main 
*@param argc Numero de argumentos
*@param argv Argumentos
*@return 0
*/
int main(int argc, char** argv) {
	int socket = 0;
	char buf[500];
	int port = 6669; /*se pasa como arg de entrada?*/
	SSL_CTX *context;
	SSL *ssl;
	int connfd = 0;
	struct sockaddr cli;
	int clilen = sizeof(cli);


	if(argc == 2){
		port = atoi(argv[1]);
	}

	puts("Inicializando nivel SSL...");
    inicializar_nivel_SSL();

    puts("Fijando contexto SSL...");
	context = fijar_contexto_SSL("../certs/server/serverkey.pem" , "../certs/servidor.pem");

	/*Abro conexion TCP*/
	puts("Abriendo conexión TCP...");
	tcp_listen_p(20, &socket, port); /*deberia añadir el puerto??*/

		fprintf(stderr, "socket --> %d\n", socket);
		fprintf(stderr, "puerto --> %d\n", port);
	/*accept*/
	connfd = accept(socket, &cli, &clilen);

	ssl = aceptar_canal_seguro_SSL(connfd, context);
	if(!ssl){
		fprintf(stderr, "Error canal seguro\n");
		exit(1);
	}

	if(evaluar_post_connectar_SSL(connfd, ssl)) {
        fprintf(stderr, "Error del certificador\n");
        exit(1);
    }

	puts("SERVIDOR ECHO");
	while(1){
        memset(buf, 0, 500);
		recibir_datos_SSL(connfd, buf, ssl);
		if(!strcmp(buf, "exit\n")) break;
		enviar_datos_SSL(connfd, buf, ssl);
	}

	cerrar_canal_SSL(connfd, ssl, context);
	
	return 0;
}
