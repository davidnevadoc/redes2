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
#include <unistd.h> 
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>

/**
*@brief Funcion main 
*@param argc Numero de argumentos
*@param argv Argumentos
*@return 0
*/
int main(int argc, char** argv) {
	int sockfd = 0;
	char buf[MAX_MSG_SSL];
	int port = 6611; /*se pasa como arg de entrada?*/
	SSL_CTX *context;
	SSL *ssl;
	int connfd = 0;
	struct sockaddr_in serv;
	struct sockaddr cli;
	socklen_t clilen = 0;
	bzero(&cli, sizeof(cli));


	if(argc == 2){
		port = atoi(argv[1]);
	}

	//puts("Inicializando nivel SSL...");
    inicializar_nivel_SSL();

    //puts("Fijando contexto SSL...");
	context = fijar_contexto_SSL(SERVER_KEY, SERVER_CERT);

	/*Abro conexion TCP*/
	//puts("Abriendo conexión TCP...");
	//tcp_listen_p(5, &socket, port); /*deberia añadir el puerto??*/

	if ( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		puts("---1");
		exit(-1);
	}
	/*rellenar estructura de serv*/
	serv.sin_family=AF_INET; /*Vamos en IPv4*/
	serv.sin_addr.s_addr=htonl(INADDR_ANY);
	serv.sin_port=htons(port);
	bzero((void*)&(serv.sin_zero), 8);

	if ( bind(sockfd, (struct sockaddr * ) &serv, sizeof(serv) ) == -1){
		close(sockfd);
		puts("---2");
		exit(-1);
	}

	/*Conexion */
	if ( listen(sockfd, 20) == -1 ) {
		puts("---3");
		exit(-1);
	}

	//fprintf(stderr, "socket --> %d\n", socket);
	//fprintf(stderr, "puerto --> %d\n", port);

	/*accept*/
	connfd = accept(sockfd, &cli, &clilen);
	//fprintf(stderr, "puerto cliente --> %d\n", connfd);
	ssl = aceptar_canal_seguro_SSL(connfd, context);
	if(!ssl){
		fprintf(stderr, "Error canal seguro\n");
		exit(1);
	}

	if(evaluar_post_connectar_SSL(connfd, ssl)) {
        fprintf(stderr, "Error del certificador\n");
        exit(1);
    }

	//puts("SERVIDOR ECHO");
	while(1){
        memset(buf, 0, MAX_MSG_SSL);
		recibir_datos_SSL(connfd, buf, ssl);
		//fprintf(stdout, "recibo ---> %s\n", buf);
		if(!strcmp(buf, "exit\n")) break;
		enviar_datos_SSL(connfd, buf, ssl);
	}

	cerrar_canal_SSL(connfd, ssl, context);
	
	return 0;
}
