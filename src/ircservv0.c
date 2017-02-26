
/**
 * Version 0 del sevidor IRC
 *
 * @brief Servidor IRC v0.0
 * @author Maria Prieto
 * @author David Nevado Catalan
 * @file ircservv0.c
 * @date 12/02/2017
 */
#include <redes2/irc.h>
#include "../include/ircserv.h"
volatile int stop=0;

/**
 * @brief Estructura para el paso de parametros a la funcion de los hilos
 */
typedef struct _data{
	/** Socket de la conexion que atiende el hilo */
	int csocket;

} data;
/*
void manejador_SIGINT(int sig){
	stop=1;
}
*/
void * atiende_cliente(data * d);

int main(int argc, char *argv[]){
	struct sockaddr_in serv;
	struct sockaddr cli;
	int sockfd =-1, connfd =-1, chilo=0;
	uint16_t port =0;
	socklen_t clilen = 0;
	data *tdata_aux = NULL;
	//TODO Implementar control sobre los hilos
	pthread_t * taux= NULL;

	bzero(&serv, sizeof(serv));
	bzero(&cli, sizeof(cli));
	/*Armar manejador de sennal*//*
	if(signal(SIGINT,manejador_SIGINT)==SIG_ERR){
		perror("Error en la captura de SIGINT");
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);	
	}
	if(port<1024){
		printf("%d Puerto no valido\n", port);
		exit(EXIT_FAILURE);
	}
	if ( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		syslog(LOG_ERR, "Error en sockfd()");
		exit(FAILURE);
	} 
	
	/*rellenar estructura de serv*/
	serv.sin_family=AF_INET;
	serv.sin_addr.s_addr=htonl(INADDR_ANY);
	serv.sin_port=htons(port);
	
	
	if ( bind(sockfd, (struct sockaddr * ) &serv,
		 sizeof(serv) ) == -1){
		syslog(LOG_ERR, "Error en bind(): %d",
		 errno);
		exit(FAILURE);
	}

	/*Conexion */
	if ( listen(sockfd, MAX_QUEUE) == -1 ) {
		syslog(LOG_ERR, "Error en listen(): %d",
		 errno);
		exit(FAILURE);
	}
	syslog(LOG_INFO, "Socket a la escucha");
	
	/*bucle principal, a la espera conexiones*/
	printf("A la espera de mensajes, escuchando puerto %d\n",
		ntohs(serv.sin_port));
	while(!stop){
		clilen= sizeof(cli);
		if( (connfd=accept(sockfd, &cli, &clilen)) == -1){
			syslog(LOG_ERR,"Error en accept(): %d",
			 errno);
		}
		/*Lanzamos hilo que atiende al cliente*/
		if ((tdata_aux = malloc(sizeof(data))) != NULL &&
		    (taux = (pthread_t * ) malloc(sizeof(pthread_t))) != NULL){
			tdata_aux->csocket=connfd;
			if (pthread_create(taux,NULL,
			  (void * (*)(void *)) atiende_cliente, tdata_aux) !=0){
				syslog(LOG_ERR, "Error en pthread_create");
			}
			/*Igualamos tdata_aux a null , a partir de ahora solo 
			se gestiona desde el hilo*/		
			tdata_aux=NULL;
			chilo++;
		} else {
			syslog(LOG_ERR, "Error en malloc(): No se pudo reservar memoria"
				" para la estructura del hilo");
		}
		
	}

	close(sockfd);
	printf("Servidor cerrado\n");
	exit(SUCCESS);

}


/**
 * Funcion ejecutada por cada hilo que atiende una conexion.
 * Define el servicio basico del sevidor
 *
 * @brief Funcion que atiende cada conexion 
 * @param d Estructura de datos data
 */
void * atiende_cliente(data * d){
	/*Numero de lineas recividas por el socket*/
	ssize_t nlines=0;
	/*Buffer donde se almacena el mensaje recibido*/
	char buff[BUFF_SIZE];
	/*Comando recibido*/
	long command =0 ;
	while (!stop){

		if ( (nlines=recv(d->csocket, buff, BUFF_SIZE, 0)) == -1){
			syslog(LOG_ERR, "Error en recv(): %d",
			errno);
		}
		command = IRC_CommandQuery(buff);
		sprintf(buff, "Comando: %ld", command);	
		if ( send(d->csocket, buff, nlines,0)== -1){
			syslog(LOG_ERR, "Error en send(): %d",
			errno);
		}
	}

	close(d->csocket);
	free(d);
	pthread_exit(NULL);
}

