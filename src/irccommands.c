#include "../include/irccommands.h"

/**
*@brief Función que atiende al comando PASS
*@param
*@return
*/
/*De momento solo muestra la contraseña introducida*/
int pass(data* d){
	long res = 0;
	char *prefix, *password;
	char ans[100];

	syslog(LOG_INFO,"\nSe ha leido %s \n", d->mensaje);
	res = IRCParse_Pass (d->mensaje, &prefix, &password);
	if(res == IRCERR_ERRONEUSCOMMAND || res == IRCERR_NOSTRING){ /*Datos insuficientes o erroneos*/
		sprintf(ans, "Parametros insuficientes\n");
	}else{
		sprintf(ans, "Has introducido la contrasenna: %s\n", password);
	}

	send(d->socket, ans, sizeof(char)*strlen(ans), 0);
	return 0;
}


/**
*@brief Función que atiende al comando NICK
*@param
*@return
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

	syslog(LOG_INFO,"IRCServ: Se ha leido %s", d->mensaje);
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
*@param
*@return
*/
/*long 	IRCParse_User (char *strin, char **prefix, char **user, char **modehost, char **serverother, char **realname)*/
int user(data* d){	
	/*Variable para el mensaje de respuesta al cliente*/
	char *reply = NULL;
	/*Variable para el mensaje de respuesta al cliente con mem estatica*/
	char streply[MAXREPLY];
	long res = 0;
	char *prefix, *user, *modehost, *serverother, *realname, *nick;
	prefix = user = modehost = serverother = realname = NULL;
	nick = get_nick(d->socket);
	syslog(LOG_INFO,"Se ha leido %s", d->mensaje);

	if((res = IRCParse_User (d->mensaje, &prefix, &user, &modehost, &serverother, &realname)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en la funcion user. IRCParse_User: %ld", res );
		return ERROR;
	}else{ 
		switch(IRCTADUser_New (user, nick, realname, NULL,
				serverother, d->IP, d->socket)){
			
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
*@param
*@return
*/
int quit(data* d){
	char *prefix, *msg;
	long res = 0;
	char mensaje[100];

	syslog(LOG_INFO,"Se ha leido %s", d->mensaje);

	res = IRCParse_Quit (d->mensaje, &prefix, &msg);//msg contiene el mensaje que escribe el user al irse?
	
	if(res == IRCERR_NOSTRING || res == IRCERR_ERRONEUSCOMMAND){
		syslog(LOG_ERR, "IRCServ: Error en el comando QUIT, %ld", res);
		sprintf(mensaje, "Error en el comando QUIT\n");
		send(d->socket, mensaje, sizeof(char)*strlen(mensaje), 0);
		return ERROR;
	}
	d->stop=1;
	return OK;
}
/**
*@brief Función que atiende al comando PASS
*@param
*@return
*/
/*De momento solo muestra la contraseña introducida*/
int join(data* d){
	
	char *prefix = NULL,*prefix_s= NULL, *msg = NULL, *channel = NULL;
	char *key = NULL, *usermode =NULL;
	char *reply =NULL;
	long res = 0;

	syslog(LOG_INFO,"Se ha leido %s", d->mensaje);

	if ( (res = IRCParse_Join (d->mensaje, &prefix, &channel, &key, &msg)) == IRC_OK){
		if ( (res =IRCTAD_Join(channel, get_nick(d->socket), usermode ,key )) == IRC_OK){
			syslog(LOG_INFO, "Usuario %s, se unio al canal %s",
			 get_nick(d->socket), channel);
			IRC_ComplexUser1459 (&prefix_s, get_nick(d->socket),  get_user(d->socket),  get_host(&(d->socket)), NULL);
			if ( IRCMsg_Join(&reply, prefix, channel, key, msg) == IRC_OK){
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
*@brief Función que atiende al comando por defecto
*@param
*@return
*/
int comandoDefault(data* d){
	char ans[100];
	sprintf(ans, "Este comando no está implementado \n");
	send(d->socket, ans, sizeof(char)*strlen(ans), 0);
	return 0;
}
