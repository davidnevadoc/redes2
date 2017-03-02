
#include "../include/atiendecliente.h"
/*Lista con los comandos disponibles,*/ /*TODO tiene que ser global?*/
int (*listaComandos[])(data *) = {pass, nick, user, NULL, NULL, NULL, quit} ;



/**
 * Funcion INTERNA ejecutada por cada hilo que atiende una conexion.
 * Define el servicio basico del sevidor
 *
 * @brief Funcion que atiende cada conexion 
 * @param d Estructura de datos data
 */
void * atiende_cliente(data* d){
	/*Tamanio del mensaje recibido en bytes*/
	ssize_t messg_size=0;
	/*Buffer donde se almacena el mensaje recibido*/
	char buff[BUFF_SIZE];
	/*Comando recibido*/
	long command = 0;
	/*Usuario de esta conexion*/

 	/*TODO Aqui deberiamos hacer el pipeline*/
	while (1){
		
		if ( (messg_size=recv(d->usuario->socket, buff, BUFF_SIZE, 0)) == -1){
			syslog(LOG_ERR, "IRCServ: Error en recv(): %s",
			strerror(errno));
		} else {
			buff[messg_size]='\0';
			command = IRC_CommandQuery(buff);
			sprintf(d->mensaje,"%s", buff);
			if(command < 0 || command > 7){
				syslog(LOG_ERR, "IRCServ: Error al leer el comando %ld",
				command);
			} else { /*Llamo a la funcion del comando  correspondiente*/
				(*listaComandos[command - 1])(d);
			}
		}
	}
	/*liberamos recursos*/
	close(d->usuario->socket);
	
	free_data(d);
	pthread_exit(NULL);
}


/**
*
* Wrapper de la funcion atiende_cliente. Se encarga de reservar memoria para la 
* estructura que se pasa a la funcion atiende_cliente, asi como de gestionar 
* los hilos y hacer diversos controles de error.
*
*@brief Wrapper de la funcion atiende_cliente
*@param cli sockaddr con direccion del socket del cliente que establecio la conexion
*
*@return Devuelve SUCCESS si todo va bien, ERROR en caso contrario
*
*/
int Atiende_cliente(struct sockaddr cli, int connfd){
	/*Estructura para el paso de parametros al hilo atiende_cliente*/
	data *data_aux = NULL;
	/*Identificador del hilo*/
	pthread_t * taux= NULL;
	/*Tamanio en bytes de la direccion del cliente*/
	int addrsize=0;
	/*Lanzamos hilo que atiende al cliente*/
	if(cli.sa_family!=AF_INET && cli.sa_family!=AF_INET6){
		syslog(LOG_ERR, "IRCServ: Address family not supported");
		return ERROR;
	}
	if ((data_aux = malloc(sizeof(data))) != NULL &&
	    (data_aux->usuario=malloc(sizeof(User))) != NULL &&
	    (taux = (pthread_t * ) malloc(sizeof(pthread_t))) != NULL){
		data_aux->usuario->socket=connfd;
		/*Resrvamos memoria para el campo IP (v4 o v6)*/
		if(cli.sa_family!=AF_INET){
			data_aux->usuario->IP=malloc(IPV4ADDRSIZE);
			addrsize=IPV4ADDRSIZE;
				
		} else {
			data_aux->usuario->IP=malloc(IPV6ADDRSIZE);
			addrsize=IPV6ADDRSIZE;	
		}
		/*Control de la reserva de memoria*/
		if(!data_aux){
			syslog(LOG_ERR, "IRCServ: Error en"
			" malloc(): No se pudo reservar memoria"
			" para la estructura del hilo");
			return ERROR;	
		}
		memcpy(data_aux->usuario->IP, cli.sa_data,addrsize);
		/*Gestion del hilo*/
		if (pthread_create(taux,NULL,
		  (void * (*)(void *)) atiende_cliente, data_aux) !=0){
			syslog(LOG_ERR, "IRCServ: Error en pthread_create");
		} else if(pthread_detach(*taux)!=0){
			syslog(LOG_ERR, "IRCServ: Error en pthread_detatch");
			
		}
	} else {
		syslog(LOG_ERR, "IRCServ: Error en"
		" malloc(): No se pudo reservar memoria"
		" para la estructura del hilo");
		return ERROR;
	}
	
	return OK;
}




