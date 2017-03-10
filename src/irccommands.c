#include "../include/irccommands.h"

/**
*@brief Función que atiende al comando PASS
 *@param d Estructura de datos con la informacion del hilo
 *@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
/*De momento solo muestra la contraseña introducida*/
int pass(data* d){
	long res = 0;
	char *prefix, *password;
	char ans[100];

		syslog(LOG_INFO,"IRCServ: Se ejecuta el comando PASS: %s", d->mensaje);
	res = IRCParse_Pass (d->mensaje, &prefix, &password);
	if(res == IRCERR_ERRONEUSCOMMAND || res == IRCERR_NOSTRING){ /*Datos insuficientes o erroneos*/
		sprintf(ans, "Parametros insuficientes\n");
	}else{
		sprintf(ans, "Has introducido la contrasenna: %s\n", password);
	}

	send(d->socket, ans, sizeof(char)*strlen(ans), 0);
	return OK;
}


/**
*@brief Función que atiende al comando NICK
 *@param d Estructura de datos con la informacion del hilo
 *@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
//TODO Especificar mas casos de error
int nick(data* d){
	/*Codigo de respuesta devuelto por las funciones de la libreria IRC*/
	long res = 0;
	/*Variable para el mensaje de respuesta al cliente*/
	char *reply = NULL;
	/*Variable para el mensaje de respuesta al cliente con mem estatica*/
	char streply[MAXREPLY]={0};
	char *prefix = NULL, * nick = NULL, *msg = NULL;

	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando NICK: %s", d->mensaje);
	if( (res = IRCParse_Nick (d->mensaje, &prefix, &nick, &msg) )!= IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en la funcion nick. IRCParse_Nick: %ld", res );
		return ERROR;
	}
	if(get_user(d->socket)){
		switch(IRCTADUser_Set(0, get_user(d->socket),NULL, NULL, 
				NULL, nick, NULL)){
		
		case IRC_OK:
			syslog(LOG_ERR, "IRCServ: Error en la funcion nick. IRCTADUserSet: %ld", res );
			sprintf( streply, ":%s NICK :%s\r\n", prefix , nick);
			send(d->socket, streply, sizeof(char)*strlen(streply), 0);
			set_nick(d->socket, nick);
			break;

		case IRCERR_NICKUSED:
			syslog(LOG_ERR,"IRCServ: Error en nick: %ld", res);
			IRCMsg_ErrNickNameInUse(&reply, IRCNAME, get_nick(d->socket), nick);
			send(d->socket, reply, sizeof(char)*strlen(streply), 0);
			free(reply);
			break;
		default:
			syslog(LOG_ERR,"IRCServ: Error en nick: %ld", res);
			return ERROR;

		}

	}else{
		set_nick(d->socket, nick);
	}
	free(nick);
	free(prefix);
	free(msg);
	return OK;
}


/**
*@brief Función que atiende al comando USER
 *@param d Estructura de datos con la informacion del hilo
 *@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int user(data* d){	
	/*Variable para el mensaje de respuesta al cliente*/
	char *reply = NULL;
	/*Variable para el mensaje de respuesta al cliente con mem estatica*/
	char streply[MAXREPLY];
	long res = 0;
	char *prefix, *user, *modehost, *serverother, *realname, *nick;
	prefix = user = modehost = serverother = realname = NULL;
	nick = get_nick(d->socket);
		syslog(LOG_INFO,"IRCServ: Se ejecuta el comando USER: %s", d->mensaje);

	if((res = IRCParse_User (d->mensaje, &prefix, &user, &modehost, &serverother, &realname)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en la funcion user. IRCParse_User: %ld", res );
		return ERROR;
	}else{ 
		switch(IRCTADUser_New (user, nick, realname, NULL,serverother, d->IP, d->socket)){
			
			case IRC_OK:
				IRCMsg_RplWelcome ( &reply, IRCNAME, nick, nick, user, serverother);
				send(d->socket, reply, strlen(reply), 0);
				free(reply);
				/*Actualizacion de la estructura local*/
				set_user(d->socket, user);
				syslog(LOG_INFO, "IRCServ: Nuevo usuario registrado: %s %s %s",
					user, realname, serverother);
				break;
			case IRCERR_NICKUSED:
				syslog(LOG_INFO, "IRCServ: El nick %s ya está registrado", nick);
				
			default:
				sprintf(streply, "Error al registrar el usuario\n");
				send(d->socket, streply, sizeof(char)*strlen(streply), 0);
				free(prefix);
				free(user);
				free(serverother);
				free(realname);
				free(modehost);
				return ERROR;
		free(prefix);
		free(modehost);
		}

	}
	return OK;
}


/**
*@brief Función que atiende al comando QUIT
 *@param d Estructura de datos con la informacion del hilo
 *@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int quit(data* d){
	char *prefix = NULL, *msg = NULL, *nick = NULL;
	long res = 0;
	char mensaje[100];
	char *reply = NULL;

	syslog(LOG_INFO,"Se ha leido %s", d->mensaje);

	res = IRCParse_Quit (d->mensaje, &prefix, &msg);
	
	if(res == IRCERR_NOSTRING || res == IRCERR_ERRONEUSCOMMAND){
		syslog(LOG_ERR, "IRCServ: Error en el comando QUIT, %ld", res);
		sprintf(mensaje, "Error en el comando QUIT\n");
		send(d->socket, mensaje, sizeof(char)*strlen(mensaje), 0);
		return ERROR;
	}
	if(msg == NULL){ //Enviamos como mensaje por defecto el nick
		nick = get_nick(d->socket);
		IRCMsg_Quit (&reply, prefix, nick);
	}else{
		IRCMsg_Quit (&reply, prefix, msg);
	}
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	d->stop=1;
	return OK;
}
/**
 *@brief Función que atiende al comando JOIN
 *@param d Estructura de datos con la informacion del hilo
 *@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
 */

int join(data* d){
	
	char *prefix = NULL,*prefix_s= NULL, *msg = NULL, *channel = NULL;
	char *key = NULL, *usermode =NULL;
	char *reply =NULL;
	long res = 0;

	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando JOIN: %s", d->mensaje);

	if ( (res = IRCParse_Join (d->mensaje, &prefix, &channel, &key, &msg)) == IRC_OK){
		if ( (res =IRCTAD_Join(channel, get_nick(d->socket), usermode ,key )) == IRC_OK){
			syslog(LOG_INFO, "Usuario %s, se unio al canal %s",
			 get_nick(d->socket), channel);
			IRC_ComplexUser1459 (&prefix_s, get_nick(d->socket),
			  get_user(d->socket),  get_host(&(d->socket)), NULL);
			if ( IRCMsg_Join(&reply, prefix_s, NULL, key, channel) == IRC_OK){
				 //en el canal se pone NULL y el canal se manda como mensaje
				send(d->socket, reply, sizeof(char)*strlen(reply), 0);
				free(prefix_s);
				return OK;
			} 
			free(prefix_s);
		}
		//TODO hacer mensajes de error para enviar	
	}
	syslog(LOG_ERR, "IRCServ: Error en el comando JOIN, %ld", res);
	return ERROR;

}
/**
 *@brief Función que atiende al comando LIST
 *@param d Estructura de datos con la informacion del hilo
 *@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
 */
int list (data *d){
	char *prefix, *channel, *target, *mode, *topic, *reply;
	char **list=NULL;
	long i,res =-1, num=0;
	int num_users=0;
	char streply[MAXREPLY];
	prefix=channel=target=mode=topic=reply=NULL;
	/*Log de ejecucion del comando*/
	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando LIST: %s", d->mensaje);
	/*Parseo del mensaje*/
	if ( (res= IRCParse_List(d->mensaje, &prefix, &channel, &target)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de list %ld", res);
		return ERROR;
	}
	/*Obtencion de lista de todos los canales*/
	if( IRCTADChan_GetList (&list, &num, NULL) != IRC_OK){	
		syslog(LOG_ERR, "IRCServ: Error en IRCTADChan_GetList, memoria insuficiente");
		return ERROR;
	}
	/*Busqueda canal especifico*/
	if(!channel){
		/*Bucle recorre canales*/
		for(i=0;i<num;i++){
			mode=IRCTADChan_GetModeChar(list[i]);
			if( !mode || ((long) *mode != IRCMODE_SECRET)){
				syslog(LOG_ERR, "aqui se llega");
				/*get numero de usuarios*/
				num_users=IRCTADChan_GetNumberOfUsers(list[i]);
				/*get topic /tema del canal*/
				if((res=IRCTAD_GetTopic(list[i], &topic))!=IRC_OK){
					syslog(LOG_ERR, "IRCServ: Error en Get_Topic %ld",
						res);
					return ERROR;
				}
				sprintf(streply, ":%s 322 %s %s %d %s \r\n",
					IRCNAME, get_nick(d->socket), list[i], num_users, topic);
				/*Envio del mensaje*/
				send(d->socket,streply,strlen(streply), 0);
			}	
		}
	/*Busqueda general*/
	} else {
		/*Bucle recorre canales*/
		for(i=0;i<num;i++){
			mode=IRCTADChan_GetModeChar(list[i]);
			if( !mode || (!(strcmp(list[i], channel)) && ((long) *mode != IRCMODE_SECRET))){
				/*get numero de usuarios*/
				num_users=IRCTADChan_GetNumberOfUsers(list[i]);
				/*get topic /tema del canal*/
				if((res=IRCTAD_GetTopic(list[i], &topic))!=IRC_OK){
					syslog(LOG_ERR, "IRCServ: Error en Get_Topic %ld",
						res);
					return ERROR;
				}
				sprintf(streply, ":%s 322 %s %s %d %s \r\n",
					IRCNAME, get_nick(d->socket), list[i], num_users, topic);
				/*Envio del mensaje*/
				send(d->socket,streply,strlen(streply), 0);
				
			}
		}
	}
	/*Mensaje de fin de lista*/
	if( (res=IRCMsg_RplListEnd(&reply, IRCNAME, get_nick(d->socket)))!=IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en IRCMsg_RplListEnd(): %ld", res);
		return ERROR;
	}
	send(d->socket, reply, strlen(reply), 0);
	/*Liberar recursos usados*/
	free(reply);
	free(topic);
	free(mode);
	free(prefix);
	free(channel);
	free(target);
	return OK;
}

/**
*@brief Función que atiende al comando WHOIS
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int whois(data *d){
	char *prefix = NULL, *target = NULL, *maskarray = NULL;
	long res = 0;
	char *reply = NULL;
	long id = 0;
	char *user = NULL, *real = NULL, *host = NULL, *ip = NULL, *away = NULL;
	long creationTS, actionTS, numberOfChannels = 0, mode = 0;
	int socket = 0, i = 0;
	char *nick = NULL;
	char **channellist;
	char array[500];

	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando WHOIS: %s", d->mensaje);

	res = IRCParse_Whois (d->mensaje, &prefix, &target, &maskarray);
	if ( res != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de whois %ld", res);
		return ERROR;
	}
	nick = get_nick(d->socket);
	IRCTADUser_GetData (&id, &user, &maskarray, &real, &host, &ip, &socket, &creationTS, &actionTS, &away);

	if(user == NULL){ //No se ha encontrado un user con ese nick
		IRCMsg_ErrNoSuchNick(&reply, SERV_NAME, nick, maskarray);
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	}else{
		IRCMsg_RplWhoIsUser (&reply, SERV_NAME, nick, maskarray, user, host, real);
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);

		IRCMsg_RplWhoIsServer (&reply, SERV_NAME, nick, maskarray, SERV_NAME, "info");
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);

		IRCTAD_ListChannelsOfUserArray (user, maskarray, &channellist, &numberOfChannels);
		for(i=0; i<numberOfChannels;i++){
			mode = IRCTAD_GetUserModeOnChannel(channellist[i], maskarray);
			if(mode&IRCUMODE_OPERATOR){
				strcat(array, "@");
			}
			strcat(array, channellist[i]);
			strcat(array, " ");
		}
		IRCMsg_RplWhoIsChannels (&reply, SERV_NAME, nick, maskarray, array); //meto el array de canales formado manualmente porque no puto funcionaba si no!!!
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	}
	IRCMsg_RplEndOfWhoIs(&reply, SERV_NAME, nick, maskarray);
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	return OK;
}
/**
*@brief Función que atiende al comando NAMES
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int names(data* d){
	char *prefix = NULL, *channel = NULL, *target = NULL;
	char *reply = NULL, *list = NULL, *nick = NULL;
	long res = 0, numberOfUsers = 0;
	
	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando NAMES: %s", d->mensaje);

	res = IRCParse_Names (d->mensaje, &prefix, &channel, &target);
	if (res != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de names %ld", res);
		return ERROR;
	}
	nick = get_nick(d->socket);
	IRCTAD_ListNicksOnChannel(channel, &list, &numberOfUsers);
	IRCMsg_RplNamReply (&reply, SERV_NAME, nick, "a", channel, list); //TODO donde he puesto una "a" se supone que hay que poner type, pero no se a que se refuere
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	IRCMsg_RplEndOfNames (&reply, SERV_NAME, nick, channel);
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	

	return OK;
}




/**
*@brief Función que atiende al comando por defecto
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int comandoDefault(data* d){
	char ans[100];
	sprintf(ans, "Este comando no está implementado \n");
	send(d->socket, ans, sizeof(char)*strlen(ans), 0);
	return OK;
}
