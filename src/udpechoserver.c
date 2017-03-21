#include "../includes/udpechoserver.h"


/***************************************************/
/* Servidor UDP de echo muy basico,                */
/* Solo funciona con direcciones IPv4              */
/***************************************************/
int main(int argc, char *argv[]){
	int sockfd =-1;
	char buff[BUFF_SIZE];
	uint16_t port =0;
	ssize_t msglen;
	struct sockaddr_in serv;
	struct sockaddr from;
	socklen_t addrlen = 0;

	bzero(&serv, sizeof(serv));
	bzero(&from, sizeof(from));

	/*Comprobacion de parametros*/
	if(argc!=2){
		printf("Entrada invalida."
			"Especifique puerto de escucha\n");
		exit(EXIT_FAILURE);
	}
	port= (uint16_t)atoi(argv[1]);
	if(port<1024){
		printf("%d Puerto no valido\n", port);
		exit(EXIT_FAILURE);
	}
	
	if ( (sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		syslog(LOG_ERR, "Error en sockfd()");
		exit(FAILURE);
	} 
	
	/*rellenar estructu ra de serv*/
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=htonl(INADDR_ANY);
	serv.sin_port=htons(port);
	
	
	if ( bind(sockfd, (struct sockaddr * ) &serv, sizeof(serv) ) == -1){
		syslog(LOG_ERR, "Error en bind(): %d", errno);
		exit(FAILURE);
	}
	/*bucle principal*/
	printf("A la espera de mensajes, escuchando puerto %d\n",port);
	while(1){
		
		msglen=recvfrom(sockfd, &buff,(size_t)BUFF_SIZE, 0,
			 &from, &addrlen);
		sendto(sockfd, &buff, msglen, 0, &from, addrlen);
		
		
	}

	close(sockfd);
	exit(SUCCESS);

}

