/**
 * Version 0 del sevidor IRC
 *
 * @brief Servidor IRC v0.0
 * @author Maria Prieto
 * @author David Nevado Catalan
 * @file main.c
 * @date 12/02/2017
 */
#define MAX_QUEUE 10
#include "../include/main.h"


int main(int argc, char *argv[]){
	struct sockaddr_in serv;
	struct sockaddr cli;

	int sockfd =-1, connfd =-1;
	uint16_t port =0;
	socklen_t clilen = 0;
	

	bzero(&serv, sizeof(serv));
	bzero(&cli, sizeof(cli));
	/*Armar manejador de sennal*//*
	if(signal(SIGINT,manejador_SIGINT)==SIG_ERR){
		perror("IRCServ: Error en la captura de SIGINT");
		exit( ERROR);
	}
	*/
	/*Comprobacion de parametros*/
	if(argc<2){
		port = 55000;
		printf("Se asigna el puerto 55000\n");
	} else if (argc == 2 ) {
		port= (uint16_t) atoi(argv[1]) ;
	} else {
		printf("Entrada invalida."
			"Especifique puerto de escucha\n");
		exit( ERROR);	
	}
	if(port<1024){
		printf("%d Puerto no valido\n", port);
		exit( ERROR);
	}
	if ( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		syslog(LOG_ERR, "IRCServ: Error en sockfd()");
		exit(ERROR);
	} 
	
	/*rellenar estructura de serv*/
	serv.sin_family=AF_INET; /*Vamos en IPv4*/
	serv.sin_addr.s_addr=htonl(INADDR_ANY);
	serv.sin_port=htons(port);
	
	
	if ( bind(sockfd, (struct sockaddr * ) &serv,
		 sizeof(serv) ) == -1){
		syslog(LOG_ERR, "IRCServ: Error en bind(): %d",
		 errno);
		exit(ERROR);
	}

	/*Conexion */
	if ( listen(sockfd, MAX_QUEUE) == -1 ) {
		syslog(LOG_ERR, "IRCServ: Error en listen(): %d",
		 errno);
		exit(ERROR);
	}
	syslog(LOG_INFO, "Socket a la escucha");
	
	/*bucle principal, a la espera conexiones*/
	printf("A la espera de mensajes, escuchando puerto %d\n",
		ntohs(serv.sin_port));
	while(1){
		clilen= sizeof(cli);
		if( (connfd=accept(sockfd, &cli, &clilen)) == -1){
			syslog(LOG_ERR,"IRCServ: Error en accept(): %d",
			 errno);
		}
		
		Atiende_cliente(cli, connfd);
	}
	close(sockfd);
	printf("Servidor cerrado\n");
	exit(OK);
}




