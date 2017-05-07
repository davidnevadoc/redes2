/**
 * @brief 
 * @file G-2302-05-P1-atiendecliente.c
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */


#include "../includes/G-2302-05-P1-atiendecliente.h"
/*Lista con los comandos disponibles,*/ 
int (*listaComandos[NUM_COMANDOS])(data *);
int select_ssl = 0; /*1 ssl, 0 no*/
/**
*@brief Función que inicializa el array de funciones con 
*		los comandos disponibles
*/
void inicializaComandos(){
	int i = 0;
	for (i=0; i<NUM_COMANDOS-1; i++){
		listaComandos[i] = comandoDefault;
	}
	listaComandos[0] = pass;
	listaComandos[1] = nick;
	listaComandos[2] = user;
	listaComandos[4] = mode;
	listaComandos[6] = quit;
	listaComandos[8] = join;
	listaComandos[9] = part;
	listaComandos[10] = topic;
	listaComandos[12] = list;
	listaComandos[14] = kick;
	listaComandos[29] = who;
	listaComandos[30] = whois;
	listaComandos[11] = names;
	listaComandos[15] = privmsg;
	listaComandos[17] = motd;
	listaComandos[33] = ping;
	listaComandos[34] = pong;
	listaComandos[36] = away;	
}


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
	char * command = NULL, * command_q = NULL, *reply = NULL;
	/*Codigo del comando recibido*/
	long command_code = 0;
	


	inicializaComandos();
	while (d->stop != 1){
		
		if ( (messg_size=recv(d->socket, buff, BUFF_SIZE, 0)) == -1){
			syslog(LOG_ERR, "IRCServ: Error en recv(): %s",
			strerror(errno));
			break;
		}
		buff[messg_size]='\0';
		/*La cola de mensajes se inicializa al buffer recibido*/
		command_q = buff;
		while(command_q){
			command_q=IRC_UnPipelineCommands (command_q , &command);
			command_code = IRC_CommandQuery(command);
			d->mensaje=command;
			if(command){
				if(command_code < 0 || command_code > NUM_COMANDOS){					
					if(IRCMsg_ErrUnKnownCommand (&reply, SERV_NAME, get_nick(d->socket), d->mensaje)==IRC_OK){
						send(d->socket, reply, sizeof(char)*strlen(reply), 0);
					}
					syslog(LOG_ERR, "IRCServ: Error al leer el comando %s"
						 "Error: %ld ", command, command_code);
				} else { /*Llamo a la funcion del comando  correspondiente*/
					(*listaComandos[command_code - 1])(d);
				}
				free(command);
			}else {
				syslog(LOG_INFO, "IRCServ: Desconexion abrupta");
				(disconnect(d));
			}
		}
		
	}

	syslog(LOG_INFO, "IRCServ: Se cerro la conexion %d", d->socket);
	/*liberamos recursos*/
	close(d->socket);
	free_data(d); 
	pthread_exit(NULL);
}


/**
*
* @brief Wrapper de la funcion atiende_cliente. Se encarga de reservar memoria para la 
* estructura que se pasa a la funcion atiende_cliente, asi como de gestionar 
* los hilos y hacer diversos controles de error.
*
*@param cli sockaddr con direccion del socket del cliente que establecio la conexion
*
*@return Devuelve SUCCESS si todo va bien, ERROR en caso contrario
*
*/
int Atiende_cliente(struct sockaddr cli, int connfd, int s_ssl){
	/*Estructura para el paso de parametros al hilo atiende_cliente*/
	data *data_aux = NULL;
	/*Identificador del hilo*/
	pthread_t * taux= NULL;
	/*Tamanio en bytes de la direccion del cliente*/
	int addrsize=0;
	/*Indicamos si se implementa ssl o no*/
	select_ssl = s_ssl;
	/*Lanzamos hilo que atiende al cliente*/
	if(cli.sa_family!=AF_INET && cli.sa_family!=AF_INET6){
		syslog(LOG_ERR, "IRCServ: Address family not supported");
		return ERROR;
	}
	if ((data_aux = data_init(connfd) )!= NULL &&
	    (taux = (pthread_t * ) malloc(sizeof(pthread_t))) != NULL){
		
		/*Resrvamos memoria para el campo IP (v4 o v6)*/
		if(cli.sa_family!=AF_INET){
			data_aux->IP=malloc(IPV4ADDRSIZE);
			addrsize=IPV4ADDRSIZE;
				
		} else {
			data_aux->IP=malloc(IPV6ADDRSIZE);
			addrsize=IPV6ADDRSIZE;	
		}
		/*Control de la reserva de memoria*/
		if(!data_aux->IP){
			syslog(LOG_ERR, "IRCServ: Error en"
			" malloc(): No se pudo reservar memoria"
			" para la estructura del hilo");
			return ERROR;	
		}
		memcpy(data_aux->IP, cli.sa_data,addrsize);
		/*Gestion del hilo*/
		if (pthread_create(taux,NULL,
		  (void * (*)(void *)) atiende_cliente, data_aux) !=0){
			syslog(LOG_ERR, "IRCServ: Error en pthread_create");
		} else if(pthread_detach(*taux)!=0){
			syslog(LOG_ERR, "IRCServ: Error en pthread_detatch");
			
		}
		free(taux);
	} else {
		syslog(LOG_ERR, "IRCServ: Error en"
		" malloc(): No se pudo reservar memoria"
		" para la estructura del hilo");
		return ERROR;
	}
	
	return OK;
}





