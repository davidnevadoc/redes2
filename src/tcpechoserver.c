#include "../include/tcpechoserver.h"



/***************************************************/
/* Servidor TCP de echo muy basico,                */
/* Solo funciona con direcciones IPv4              */
/***************************************************/
int main(int argc, char *argv[]){
	int sockfd =-1, connfd =-1;
	char buff[BUFF_SIZE];
	uint16_t port =0;
	time_t ticks;
	struct sockaddr_in serv;
	struct sockaddr cli;
	socklen_t addrlen = 0;

	bzero(&serv, sizeof(serv));
	bzero(&cli, sizeof(cli));

	/*Comprobacion de parametros*/
	if(argc<2){
		port = 0;
		printf("Se asignarar un puerto automaticamente\n");
	} else if (argc == 2 ) {
		port= (uint16_t) atoi(argv[1]) ;
	} else {
		printf("Entrada invalida."
			"Especifique puerto de escucha\n");
		exit(EXIT_FAILURE);	
	}	
	if(port<1024 && port > 0){
		printf("%d Puerto no valido\n", port);
		exit(EXIT_FAILURE);
	}
	
	if ( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		syslog(LOG_ERR, "Error en sockfd()");
		exit(FAILURE);
	} 
	
	/*rellenar estructu ra de serv*/
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=htonl(INADDR_ANY);
	serv.sin_port=htons(port);
	
	
	if ( bind(sockfd, (struct sockaddr * ) &serv,
		 sizeof(serv) ) == -1){
		syslog(LOG_ERR, "Error en bind(): %s",
		 strerror(errno));
		exit(FAILURE);
	}

	/*Conexion */
	//TODO Wrapper a la funcion listen para poder modificar el
	// tamnio de la cola y ajustarla en funcion de las necesidades
	// del servidor. De momento macro definida en tcpechoserver.h
	if ( listen(sockfd, MAX_QUEUE) == -1 ) {
		syslog(LOG_ERR, "Error en listen(): %s",
		 strerror(errno));
		exit(FAILURE);
	}
	syslog(LOG_INFO, "Socket a la scucha");
	
	/*bucle principal*/
	printf("A la espera de mensajes, escuchando puerto %d\n",
		ntohs(serv.sin_port));
	while(1){
		if( (connfd=accept(sockfd, &cli, &addrlen)) == -1){
			syslog(LOG_ERR,"Error en accept(): %s",
			 strerror(errno));
		}
		ticks = time (NULL);
		snprintf(buff, sizeof(buff),
			 "%.24s\r\n", ctime(&ticks)); 
	
		write(connfd, buff, strlen(buff));
		close(connfd);
	}

	close(sockfd);
	exit(SUCCESS);

}

