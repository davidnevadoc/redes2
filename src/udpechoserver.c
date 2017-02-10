#include "../include/udpechoserver.h"


/***************************************************/
/* Servidor UDP de echo muy basico,                */
/* Solo funciona con direcciones IPv4              */
/***************************************************/
int main(int argc, char *argv[]){
	int sockfd =-1;
	char buff[BUFF_SIZE];
	struct sockaddr_in serv;
	struct sockaddr from;
	socklen_t addrlen = 0;

	bzero(&serv, sizeof(serv));
	bzero(&from, sizeof(from));	
	
	if ( (sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		syslog(LOG_ERR, "Error en sockfd()");
		exit(FAILURE);
	} 
	
	/*rellenar estructu ra de serv*/
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=htonl(INADDR_ANY);
	serv.sin_port=htons(53);
	
	
	if ( bind(sockfd, (struct sockaddr * ) &serv, sizeof(serv) ) == -1){
		syslog(LOG_ERR, "Error en bind(): %d", errno);
		exit(FAILURE);
	}
	/*bucle principal*/
	
	while(1){
		printf("A la espera de mensajes,"
			"escuchando puerto %d\n", serv.sin_port);
		recvfrom(sockfd, &buff,(size_t)BUFF_SIZE, 0,
			 &from, &addrlen);
		printf("Message arrived!\n");
		break;
	}

	close(sockfd);
	exit(SUCCESS);

}

