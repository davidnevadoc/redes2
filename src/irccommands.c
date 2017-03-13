#include "../include/irccommands.h"

#include <redes2/irc.h>

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
			sprintf( streply, ":%s NICK :%s\r\n", prefix , nick);
			send(d->socket, streply, sizeof(char)*strlen(streply), 0);
			set_nick(d->socket, nick);
			break;

		case IRCERR_NICKUSED:
			syslog(LOG_ERR,"IRCServ: Error en nick: %ld", res);
			IRCMsg_ErrNickNameInUse(&reply, SERV_NAME, get_nick(d->socket), nick);
			send(d->socket, reply, sizeof(char)*strlen(streply), 0);
			free(reply);
			break;
		default:
			syslog(LOG_ERR, "IRCServ: Error en la funcion nick. IRCTADUserSet: %ld", res );
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
				IRCMsg_RplWelcome ( &reply, SERV_NAME, nick, nick, user, serverother);
				send(d->socket, reply, strlen(reply), 0);
				free(reply);
				/*Actualizacion de la estructura local*/
				set_user(d->socket, user);
				syslog(LOG_INFO, "IRCServ: Nuevo usuario registrado: %s %s %s %d",
					user, realname, serverother, d->socket);
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
	char *prefix = NULL, *msg = NULL, *nick = NULL, *prefix_s=NULL;
	long res = 0;
	char *reply = NULL;

	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando QUIT %s", d->mensaje);
	if((res = IRCParse_Quit (d->mensaje, &prefix, &msg) )!=IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el comando QUIT, %ld", res);
		return ERROR;
	}/*TODO
	ComplexUser_bySocket(&prefix_s, &(d->socket));
	if(msg == NULL){ //Enviamos como mensaje por defecto el nick
		nick = get_nick(d->socket);
		if (IRCMsg_Quit (&reply, prefix_s, nick)!=IRC_OK ){
			syslog(LOG_ERR, "IRCServ: Error MsgQuit");
			return ERROR;
		}
	}else{
		if(IRCMsg_Quit (&reply, prefix_s, msg)){
			syslog(LOG_ERR, "IRCServ: Error MsgQuit");
			return ERROR;
		}
	}
	*/
	IRCTAD_Quit(get_nick(d->socket));
	free(reply);
	IRCMsg_Error(&reply, SERV_NAME, "Terminating");
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);

	set_user(d->socket, NULL);
	set_nick(d->socket, NULL);
	free(reply);
	free(prefix);
	free(msg);
	free(prefix_s);
	d->stop=1;
	return OK;
}
/**
 *@brief Función que atiende al comando JOIN
 *@param d Estructura de datos con la informacion del hilo
 *@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
 */

int join(data* d){
	
	char *prefix, *prefix_s, *msg, *channel, *key, *usermode, *reply;
	long res = 0;
	prefix=prefix_s=msg=channel=key=usermode=reply=NULL;

	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando JOIN: %s", d->mensaje);

	if ( (res = IRCParse_Join (d->mensaje, &prefix, &channel, &key, &msg)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de JOIN, %ld", res);
		IRCMsg_ErrNeedMoreParams (&reply, SERV_NAME, get_nick(d->socket), d->mensaje);
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);
		return ERROR;
	}
	if ( (res =IRCTAD_Join(channel, get_nick(d->socket), usermode ,key)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en IRCTAD_Join, %ld", res);
		return ERROR;
	}
	syslog(LOG_INFO, "Usuario %s, se unio al canal %s", get_nick(d->socket), channel);
	ComplexUser_bySocket(&prefix_s, &(d->socket));
	if ( IRCMsg_Join(&reply, prefix_s, msg, key, channel) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en IRCMsg_Join, %ld", res);
		return ERROR;
	}
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	return OK;

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
					SERV_NAME, get_nick(d->socket), list[i], num_users, topic?topic:"");
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
					SERV_NAME, get_nick(d->socket), list[i], num_users, topic?topic:"");
				/*Envio del mensaje*/
				send(d->socket,streply,strlen(streply), 0);
				
			}
		}
	}
	/*Mensaje de fin de lista*/
	if( (res=IRCMsg_RplListEnd(&reply, SERV_NAME, get_nick(d->socket)))!=IRC_OK){
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
	char *prefix, *target, *maskarray, *user, *real, *host, *ip, *away, *reply;
	int socket = 0, i = 0;
	long creationTS, actionTS, numberOfChannels, mode, id, res;
	char **channellist = NULL;
	char array[500] = {0};

	creationTS=actionTS=numberOfChannels=mode=id=res=0;
	prefix=target=maskarray=user=real=host=ip=away=reply=NULL;
	/*Log de ejecucion del comando*/
	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando WHOIS: %s", d->mensaje);
	/*Parseo del mensaje*/
	res = IRCParse_Whois (d->mensaje, &prefix, &target, &maskarray);
	if ( res != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de whois %ld", res);
		IRCMsg_ErrNoNickNameGiven (&reply, SERV_NAME, get_nick(d->socket));//Malformado=no se introduce nick
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);
		free(reply);
		return ERROR;
	}

	IRCTADUser_GetData (&id, &user, &maskarray, &real, &host, &ip, &socket, &creationTS, &actionTS, &away);

	if(user == NULL){ //No se ha encontrado un user con ese nick
		IRCMsg_ErrNoSuchNick(&reply, SERV_NAME, get_nick(d->socket), maskarray);
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	}else{
		if((res = IRCMsg_RplWhoIsUser (&reply, SERV_NAME, get_nick(d->socket), maskarray, user, host, real)) != IRC_OK){
			syslog(LOG_ERR, "IRCServ: Error en IRCMsg_RplWhoIsUser(): %ld", res);
			return ERROR;
		}
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);

		if((res = IRCMsg_RplWhoIsServer (&reply, SERV_NAME, get_nick(d->socket), maskarray, SERV_NAME, "info")) != IRC_OK){
			syslog(LOG_ERR, "IRCServ: Error en IRCMsg_RplWhoIsServer(): %ld", res);
			return ERROR;
		}
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
		if((res = IRCMsg_RplWhoIsChannels (&reply, SERV_NAME, get_nick(d->socket), maskarray, array)) != IRC_OK){
			syslog(LOG_ERR, "IRCServ: Error en IRCMsg_RplWhoIsChannels(): %ld", res);
			return ERROR;
		}
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	}

	if ((res = IRCMsg_RplEndOfWhoIs(&reply, SERV_NAME, get_nick(d->socket), maskarray)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en IRCMsg_RplEndOfWhoIs(): %ld", res);
		return ERROR;
	}
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	syslog(LOG_ERR, "IRCServ: Termina WHOIS");
	/*Liberamos recursos*/
	//IRC_MFree(9, reply, prefix, target, maskarray, real, host, ip, away, channellist);
	syslog(LOG_ERR, "IRCServ: Termina WHOIS");
	return OK;
}
/**
*@brief Función que atiende al comando NAMES
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int names(data* d){
	char *prefix, *channel, *target, *reply, *list;
	long res = 0, numberOfUsers = 0;
	prefix=channel=target=reply=list=NULL;
	
	/*Log de ejecucion del comando*/
	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando NAMES: %s", d->mensaje);

	/*Parseo del mensaje*/
	res = IRCParse_Names (d->mensaje, &prefix, &channel, &target);
	if (res != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de names %ld", res);
		return ERROR;
	}
	if (IRCTAD_ListNicksOnChannel(channel, &list, &numberOfUsers) != IRC_OK){
		IRCMsg_RplEndOfNames (&reply, SERV_NAME, get_nick(d->socket), channel);
		send(d->socket, reply, sizeof(char)*strlen(reply), 0);
		return OK;
	}

	if( (res = IRCMsg_RplNamReply (&reply, SERV_NAME, get_nick(d->socket), "type", channel, list)) != IRC_OK){//TODO si pongo null peta, asi que dejo type.
		syslog(LOG_ERR, "IRCServ: Error en IRCMsg_RplNamReply(): %ld", res);
		return ERROR;
	} 
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);

	if( (res = IRCMsg_RplEndOfNames (&reply, SERV_NAME, get_nick(d->socket), channel)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en IRCMsg_RplEndOfNames(): %ld", res);
		return ERROR;
	}
	send(d->socket, reply, sizeof(char)*strlen(reply), 0);

	/*Liberamos recursos*/
	free(channel);
	free(target);
	free(prefix);
	free(list);
	free(reply);
	return OK;
}
/**
*@brief Función que atiende al comando PRIVMSG
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/

int privmsg(data *d){
	/*Inicializacion de variables*/
	long res=0, nlist=0;
	int sockdest=0, i=0;
	char * msgtarget, *msg, *prefix, *prefix_s, *reply, *away, *rplaway, **list=NULL;
	char streply[MAXREPLY]={0};
	msgtarget=msg=prefix=prefix_s=reply=away=rplaway=NULL;
	/*Mensaje informativo para debugging*/
	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando privmsg: %s", d->mensaje);
	/*Parseo del comando*/
	if( (res=IRCParse_Privmsg(d->mensaje, &prefix, &msgtarget, &msg))!=IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de PRIVMSG %ld", res);
		return ERROR;
	}


	/**/
	if(msgtarget){
		/*Obtenemos el prefijo para enviar*/
		ComplexUser_bySocket(&prefix_s, &(d->socket));
		/*Mensaje para el usuario*/
		IRCMsg_Privmsg (&reply, prefix_s, msgtarget, msg);
		if(IRCTAD_ListNicksOnChannelArray(msgtarget, &list, &nlist)!=IRCERR_NOVALIDCHANNEL){/*Caso mensaje a canal*/
			for(i=0;i<nlist;i++){
				sockdest=get_sock_by_nick(list[i]);
				if(sockdest!=d->socket){
					send(sockdest, reply,  sizeof(char)*strlen(reply), 0);
				}
				free(list[i]);
			}
		}
		//TODO He cambiado la comprobacion de este else if, si no petaba cuando el nick no existía
		else if(IRCTADUser_Test (0, NULL, msgtarget) == IRC_OK){
			sockdest=get_sock_by_nick(msgtarget);
			res = IRCTADUser_GetAway(0, NULL, get_nick(d->socket), NULL, &away);
			if(res!=IRC_OK){
				syslog(LOG_ERR, "IRCServ: Error en GetAway %ld" ,res);
				//IRC_MFree(6, msgtarget, msg, prefix, prefix_s, reply, list);
				return ERROR;
			}
			if(away){
				IRCMsg_RplAway(&rplaway, SERV_NAME, get_nick(d->socket), msgtarget, away);
				send(d->socket, rplaway, sizeof(char)*strlen(rplaway), 0);
				free(rplaway);
				free(away);
			}
			syslog(LOG_INFO,"IRCServ: Mensaje enviado a %d", sockdest);
			send(sockdest, reply,  sizeof(char)*strlen(reply), 0);

		}else{ //Aqui nunca entraaaaaaaaaaaa
			syslog(LOG_INFO, "IRCServ: No se encontro el nick de destino");
			free(reply);
			IRCMsg_ErrNoSuchNick(&reply, prefix_s, get_nick(d->socket), msgtarget);
			send(d->socket, reply,  sizeof(char)*strlen(reply), 0);
		
		}
	/*Si no hay destino -> faltan parametros*/
	}else{
		syslog(LOG_INFO, "IRCServ: No se introdujo destino");
		sprintf(streply, ":%s 461 %s %s :Needed more parameters\r\n", SERV_NAME,
		get_nick(d->socket), "PRIVMSG");
		send(d->socket, streply,  sizeof(char)*strlen(streply), 0);
	}
	//IRC_MFree(6, msgtarget, msg, prefix, prefix_s, reply, list);
	free(msgtarget);
	free(prefix_s);
	free(prefix);
	free(msg);
	free(reply);
	free(list);
	return OK;
}
/**
*@brief Función que atiende al comando PING
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int ping(data * d){
	char * prefix, *server, *server2, *msg, *reply, * prefix_s;
	long res=0;
	prefix=prefix_s=server=server2=msg=reply=NULL;
	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando ping: %s", d->mensaje);
	if((res=IRCParse_Ping(d->mensaje, &prefix, &server, &server2, &msg))!=IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de PING %ld", res);
		return ERROR;
	}
	ComplexUser_bySocket(&prefix_s, &(d->socket));
	if((res=IRCMsg_Pong (&reply, SERV_NAME , server, server2, server?server:""))!=IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en IRCMsg_Pong %ld", res);
		return ERROR;
	}
	send(d->socket, reply, strlen(reply)*sizeof(char), 0);
	//IRC_MFree(6, server, server2, prefix, msg, reply, prefix_s);
	free(server);
	free(server2);
	free(prefix);
	free(msg);
	free(reply);
	free(prefix_s);
	return OK;
}
int pong(data *d){
	return OK;
}

int part(data *d){
	char * prefix, *channel, *msg, *reply, *prefix_s;
	char **list=NULL;
	char streply[MAXREPLY]={0};
	long res=0, nlist=0;
	int i=0, sockdest =0;
	prefix=channel=msg=reply=prefix_s=NULL;
	syslog(LOG_INFO, "IRCServ: Se ejecuta el comando PART: %s", d->mensaje);
	if((res=IRCParse_Part(d->mensaje, &prefix, &channel, &msg))!=IRC_OK){	
		syslog(LOG_ERR, "IRCServ: Error al parsear PART %ld" ,res);
		return ERROR;
	}
	if((res=IRCTAD_Part(channel, get_nick(d->socket)))==IRC_OK){
		syslog(LOG_INFO, "IRCServ: El usuario %s salio del canal %s", get_nick(d->socket), channel);
		ComplexUser_bySocket(&prefix_s, &(d->socket));
		IRCMsg_Part(&reply, prefix_s, channel, msg);
		send(d->socket, reply, sizeof(char)* strlen(reply), 0);
		IRCTAD_ListNicksOnChannelArray(channel, &list, &nlist);
		for(i=0;i<nlist;i++){
			sockdest=get_sock_by_nick(list[i]);
			if(sockdest!=d->socket){
				sprintf(streply, ":%s %s has quit",SERV_NAME, get_nick(d->socket));
				send(sockdest, streply, sizeof(char)*strlen(reply), 0);
			}
			free(list[i]);
		}
	}else{
		syslog(LOG_INFO, "IRCServ: Error en IRCTAD_Part %ld",res);
		//TODO Mensajes de error
	}
	//IRC_MFree(6, prefix, channel, msg, reply, prefix_s, list);
	return OK;
}

/**
*@brief Función que atiende al comando TOPIC
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int topic(data* d){
	char *prefix, *channel , *topic, *reply;
	long res = 0;
	char streply[MAXREPLY]={0};
	prefix=channel=topic=reply=NULL;
	/*Log de ejecucion del comando*/
	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando TOPIC: %s", d->mensaje);

	if( (res = IRCParse_Topic (d->mensaje, &prefix, &channel, &topic)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de TOPIC %ld", res);
		return ERROR;
	}
	if(!channel){
		syslog(LOG_INFO, "IRCServ: Canal no especificado TOPIC");
		sprintf(streply, ":%s 461 %s %s :Needed more parameters\r\n", SERV_NAME,
		get_nick(d->socket), "TOPIC");
		send(d->socket, streply,  sizeof(char)*strlen(streply), 0);
		//IRC_MFree(4, prefix, channel, topic, reply);
		return OK;
	}
	
	if(topic == NULL){ 
		IRCTAD_GetTopic (channel, &topic);
		if(topic == NULL){//Canal sin topic anteriormente establecido
			IRCMsg_RplNoTopic (&reply, SERV_NAME, get_nick(d->socket), channel);
		}else{//Mostramos el topic actual
			IRCMsg_RplTopic (&reply, SERV_NAME, get_nick(d->socket), channel, topic);
		}
	}else{ //Cambio de topic
		IRCTAD_SetTopic (channel, get_nick(d->socket), topic);
		IRCMsg_Topic (&reply, SERV_NAME, channel, topic);
	}

	send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	/*Liberamos recursos*/
	free(prefix);
	free(topic);
	free(reply);
	free(channel);
	return OK;
}

/**
*@brief Función que atiende al comando KICK
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int kick(data* d){
	char *prefix, *channel , *user, *comment, *reply;
	long res = 0, mode = 0, sockdest = 0;
	prefix=channel=user=comment=reply=NULL;
	/*Log de ejecucion del comando*/
	syslog(LOG_INFO,"IRCServ: Se ejecuta el comando KICK: %s", d->mensaje);

	if( (res = IRCParse_Kick (d->mensaje, &prefix, &channel, &user, &comment)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en el parseo de KICK %ld", res);
		return ERROR;
	} 
	mode = IRCTAD_GetUserModeOnChannel(channel, get_nick(d->socket));

	/*Veo si el usuario es operador del canal, entonces no se le puede expulsar*/
	if(mode&IRCUMODE_OPERATOR){//Le puede expulsar

		if ((res = IRCTAD_KickUserFromChannel (channel,user)) != IRC_OK){
			syslog(LOG_ERR, "IRCServ: Error en IRCTAD_KickUserFromChannel() %ld", res);
			return ERROR;
		} 

		if ((res = IRCMsg_Kick (&reply, SERV_NAME, channel, user, comment)) != IRC_OK){
			syslog(LOG_ERR, "IRCServ: Error en IRCTAD_KickUserFromChannel() %ld", res);
			return ERROR;
		}
		sockdest=get_sock_by_nick(user);
		send(sockdest, reply, sizeof(char)*strlen(reply), 0);
	}else{

		IRCMsg_ErrChanOPrivsNeeded(&reply, SERV_NAME, get_nick(d->socket), channel);
	}

	send(d->socket, reply, sizeof(char)*strlen(reply), 0);
	return OK;
}

/**
*@brief Función que atiende al comando por defecto
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int comandoDefault(data* d){
	return OK;
}
