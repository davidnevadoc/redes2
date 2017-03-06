#include "../include/irccommands.h"
#define IRCNAME "valknutserver"
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

	send(d->usuario->socket, ans, sizeof(char)*strlen(ans), 0);
	return 0;
}


/**
*@brief Función que atiende al comando NICK
*@param
*@return
*/
//TODO Eviar mensaje al usuario informando del cambio si todo fue bien o de si hubo algun error
int nick(data* d){
	long res = 0;
	/*Variable para el mensaje de respuesta al cliente*/
	char *reply = NULL;
	char streply[MAXREPLY];
	char *prefix = NULL, * nick = NULL, *msg = NULL;

	syslog(LOG_INFO,"IRCServ: Se ha leido %s", d->mensaje);
	if( (res = IRCParse_Nick (d->mensaje, &prefix, &nick, &msg) )!= IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en la funcion nick. IRCParse_Nick: %ld", res );
		return ERROR;
	}
	/*Solo si el usuario existe se intenta cambiar el nick, si no existe,
	(no esta creado), simplemente se asigna un nuevo valor a nick de la
	estructura usuario que mantiene el hilo de forma local */
	if(d->usuario->userId!=NOTCREATED){
		switch(IRCTADUser_Set(d->usuario->userId, d->usuario->user,
				d->usuario->nick, d->usuario->real, 
				NULL, nick, NULL)){
		
		case IRC_OK:
			syslog(LOG_ERR, "IRCServ: Error en la funcion nick. IRCTADUserSet: %ld", res );
			sprintf( streply, ":%s NICK :%s\r\n", prefix , nick);
			send(d->usuario->socket, streply, sizeof(char)*strlen(streply), 0);
			d->usuario->nick=nick;
			break;

		case IRCERR_NICKUSED:
			syslog(LOG_ERR,"IRCServ: Error en nick: %ld", res);
			IRCMsg_ErrNickNameInUse(&reply, IRCNAME, d->usuario->nick, nick);
			send(d->usuario->socket, reply, sizeof(char)*strlen(streply), 0);
			free(reply);
			break;
		default:
			syslog(LOG_ERR,"IRCServ: Error en nick: %ld", res);
			return ERROR;

		}
	
		

	} else {
		d->usuario->nick=nick;
	}
	return OK;
}


/**
*@brief Función que atiende al comando USER
*@param
*@return
*/
/*long 	IRCParse_User (char *strin, char **prefix, char **user, char **modehost, char **serverother, char **realname)*/
int user(data* d){	
	char ans[100];
	long res = 0;
	char *prefix, *user, *modehost, *serverother, *realname;

	syslog(LOG_INFO,"Se ha leido %s", d->mensaje);

	if((res = IRCParse_User (d->mensaje, &prefix, &user, &modehost, &serverother, &realname)) != IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en la funcion user. IRCParse_User: %ld", res );
		return ERROR;
	}else{ 
		switch(IRCTADUser_New (user, d->usuario->nick, realname, d->usuario->password,
				modehost, d->usuario->IP, d->usuario->socket)){
			case IRCERR_NICKUSED:
				sprintf(ans, "El nick %s ya está registrado", d->usuario->nick);
				break;
			case IRC_OK:
				sprintf(ans, "Usuario con nick %s registrado correctamente\n",d->usuario->nick );
				break;
			default:
				sprintf(ans, "Error al registrar el usuario\n");
				send(d->usuario->socket, ans, sizeof(char)*strlen(ans), 0);
				return ERROR;
		}

	}

	/*Notificamos al usuario*/
	send(d->usuario->socket, ans, sizeof(char)*strlen(ans), 0);
	return 0;
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
		send(d->usuario->socket, mensaje, sizeof(char)*strlen(mensaje), 0);
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
	
	char *prefix = NULL, *msg = NULL, *channel = NULL;
	char *key = NULL, *usermode =NULL;
	long res = 0;

	syslog(LOG_INFO,"Se ha leido %s", d->mensaje);

	if ( (res = IRCParse_Join (d->mensaje, &prefix, &channel, &key, &msg)) == IRC_OK){
		if ( IRCTAD_Join(channel, d->usuario->nick, usermode ,key ) == IRC_OK){
			syslog(LOG_INFO, "Usuario %s, se unio al canal %s",
				 d->usuario->nick, channel);
			return OK;
		}
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
	send(d->usuario->socket, ans, sizeof(char)*strlen(ans), 0);
	return 0;
}
