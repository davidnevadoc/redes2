#include "../include/irccommands.h"

/**
*@brief Función que atiende al commando PASS
*@param
*@return
*/
/*De momento solo muestra la contraseña introducida*/
int pass(data* d){
	long res = 0;
	char *prefix, *password;
	char ans[100];

	fprintf(stderr, "\nSe ha leido %s \n", d->mensaje);
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
*@brief Función que atiende al commando NICK
*@param
*@return
*/
//TODO Eviar mensaje al usuario informando del cambio si todo fue bien o de si hubo algun error
int nick(data* d){
	char ans[100];
	long res = 0;
	char *prefix = NULL, * nick;

	syslog(LOG_INFO,"Se ha leido %s", d->mensaje);
	if( (res = IRCParse_Nick (d->mensaje, &prefix, &nick, NULL) )!= IRC_OK){
		syslog(LOG_ERR, "IRCServ: Error en la funcion nick. IRCParse_Nick: %ld", res );
		return ERROR;

	}
	/*Solo si el usuario existe se intenta cambiar el nick, si no existe,
	(no esta creado), simplemente se asigna un nuevo valor a nick de la
	estructura usuario que mantiene el hilo de forma local */
	if(d->usuario->nick){
		if (res= IRCTADUser_Set(d->usuario->userId, d->usuario->user,
				d->usuario->nick, d->usuario->real, 
				NULL, nick, NULL)!=IRC_OK){
				syslog(LOG_ERR, "IRCServ: Error en la funcion nick. IRCTADUserSet: %ld", res );

			/*TODO MUY IMPORTANTE. Hay que diferenciar entre tipos de errores
			en esta funcion, ya que  IRCTADUser_Set, puede dar error si no
			encuentra el usuario, pero esto puede ocurrir por ejemplo si un usuario
			envia dos veces el comando nick sin haber creado un usuario primero.
			Por esta razon esta comentado el return ERROR*/
			//return ERROR;
		}

	}
	
	/*El nick se asigna de forma local de todas formas*/
	d->usuario->nick=nick;
	return OK;
}


/**
*@brief Función que atiende al commando USER
*@param
*@return
*/
/*long 	IRCParse_User (char *strin, char **prefix, char **user, char **modehost, char **serverother, char **realname)*/
int user(data* d){	
	long res = 0;
	char *prefix, *user, *modehost, *serverother, *realname;


	res = IRCParse_User (d->mensaje, &prefix, &user, &modehost, &serverother, &realname);

	if(res == IRCERR_ERRONEUSCOMMAND || res == IRCERR_NOSTRING){ /*Datos insuficientes o erroneos*/
		fprintf(stderr, "\nPARAMETROS INSUFICIENTES\n");
	}else{ //TODO como ver si el usuario ya estaba registrado
		/*switch(IRCTADUser_New (user, char *nick, char *realname, char *password, modehost, char *IP, d->usuario->socket)){
			case
		}*/
	}

	return 0;
}


/**
*@brief Función que atiende al commando QUIT
*@param
*@return
*/
int quit(data* d){
	char *prefix, *msg;
	long res = 0;
	char mensaje[100];
	res = IRCParse_Quit (d->mensaje, &prefix, &msg);//msg contiene el mensaje que escribe el user al irse?
	
	if(res == IRCERR_NOSTRING || res == IRCERR_ERRONEUSCOMMAND){
		sprintf(mensaje, "Error en el comando QUIT\n");
		send(d->usuario->socket, mensaje, sizeof(char)*strlen(mensaje), 0);
	}
	/*TODO Esto esmejor moverlo a la funcion atender_cliente.
	Podemos pasar un parametro stop que quit ponga a 0 y de esta forma
	atender_cliente sale del bucle y libera todos los recusos*/
	/*
	}else{ 
		sprintf(mensaje, "Cerrando conexión...\n");
		send(d->usuario->socket, mensaje, sizeof(char)*strlen(mensaje), 0);
		close(d->usuario->socket);
		free(d);
		pthread_exit(NULL);
	}
	*/
	return 0;
}
