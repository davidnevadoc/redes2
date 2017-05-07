/**
 * @brief Servidor IRC con SSL
 * @file servidor_iIRC.c
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#include "../includes/G-2302-05-P1-main.h"
#include "../includes/G-2302-05-P3-ssl_tools.h"
#include "../includes/G-2302-05-P2-tcp_tools.h"
#include <unistd.h> 
int stop=0;
void manejador_SIGINT(int sig){
	stop=1;
}


/**
 * Funcion Main del servidor. Gestiona los parametros pasados por el usuario
 * y abre un socket para ponerse a la espera de mensajes en el puerto deseado.
 * @brief Main del servidor
 * @return ERROR si se produce un fallo que provoca una salida inseperada
 * 	   OK salida controlada con el cierre del servidor
 */
int main(int argc, char *argv[]){
	/*Declaracion e inicializacion de variables*/
	struct sockaddr_in serv;
	struct sockaddr cli;

	int n=0, i=0;
	int sockfd =-1, connfd =-1;
	uint16_t port =0;
	socklen_t clilen = 0;
	
	bzero(&serv, sizeof(serv));
	bzero(&cli, sizeof(cli));

	/*Comprobacion de parametros*/
	if(argc<2){
		port = 6669;
		printf("Se asigna el puerto %" PRIu16 "\n", port);
	} else if (argc == 3 ) {
		port= (uint16_t) atoi(argv[2]) ;
	} else {
		printf("Entrada invalida."
			"Ejemplo: ./servidor_IRC [--port puerto]\n");
		exit(ERROR);	
	}
		
	if(port<1024){
		printf("%d Puerto no valido\n", port);
		exit( ERROR);
	}

	/*Armar manejador de sennal*/
	if(signal(SIGINT,manejador_SIGINT)==SIG_ERR){
		perror("Error en la captura de SIGINT");
		exit(ERROR);
	}

	
    inicializar_nivel_SSL();

	if(fijar_contexto_SSL("../certs/server/serverkey.pem", "../certs/servidor.pem", "../certs/ca/cacert.pem") == SSL_ERR){
		fprintf(stderr, "Error fijando contexto\n");
		exit(ERROR);
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
		close(sockfd);
		exit(ERROR);
	}

	/*Conexion */
	if ( listen(sockfd, MAX_QUEUE) == -1 ) {
		syslog(LOG_ERR, "IRCServ: Error en listen(): %d",
		 errno);
		exit(ERROR);
	}
	syslog(LOG_INFO, "Socket a la escucha");
	if(init_var()!=OK){
		syslog(LOG_ERR, "IRCServ: No se pudieron iniciar las variables globales");
		//TODO cerrar sockets, liberar...
		exit(ERROR);
	}
	/*bucle principal, a la espera conexiones*/
	printf("A la espera de mensajes, escuchando puerto %d\n",
		ntohs(serv.sin_port));
	while(!stop){
		clilen= sizeof(cli);
		if( (connfd=accept(sockfd, &cli, &clilen)) == -1){
			syslog(LOG_ERR,"IRCServ: Error en accept(): %d",
			 errno);
		}
		if(aceptar_canal_seguro_SSL(connfd) == SSL_ERR){
		fprintf(stderr, "Error canal seguro\n");
		exit(ERROR);
		}

		if(evaluar_post_connectar_SSL(connfd) == SSL_ERR) {
		    fprintf(stderr, "Error del certificador\n");
		    exit(ERROR);
		}
		Atiende_cliente(cli, connfd, 1);/*seleccionamos ssl*/
	}

	cerrar_canal_SSL(connfd);

	n = getdtablesize();
	for(i=0;i<n;i++){
		close(i);
	}
	free_all();
	printf("\nServidor cerrado\n");
	exit(OK);
}




