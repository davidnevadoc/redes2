/**
 * @brief Funciones de manejo de variables globales
 * @file G-2302-05-P1-ircserver.c
 * @author Maria Prieto
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/03/2017
 */

#include "../includes/G-2302-05-P1-ircserver.h"

/*El primer socket asignable*/
#define START 3
/*Mutex para el control de acceso a los arrays*/
pthread_mutex_t users_mutex;
pthread_mutex_t nicks_mutex;

/*Arrays de datos globales*/
char** users = NULL;
char** nicks = NULL;


/**
 *@brief Función que reserva memoria para las estructuras globales
 *@return OK si todo fue bien, ERROR si se produjo algun error
 */
int init_var(void){
	/*Se reserva memoria para el array de usuarios y nicks*/
	users= malloc(MAXUSERS * sizeof(char *));
	nicks= malloc(MAXUSERS * sizeof(char *));
	if (!(users) || !(nicks)){
		free (nicks);	
		free (users);
		return ERROR;
	}
	bzero(users,(MAXUSERS* sizeof(char *)));
	bzero(nicks,(MAXUSERS* sizeof(char *)));
	/*Inicializar mutex*/
	if(pthread_mutex_init(&users_mutex, NULL)){
		free(nicks);
		free(users);
		return ERROR;
	}
	if(pthread_mutex_init(&nicks_mutex, NULL)){
		free(nicks);
		free(users);
		pthread_mutex_destroy(&users_mutex);
		return ERROR;
	}
	return OK;
}
/**
 *@brief Asigna un nuevo nombre de usuario al socket sockfd
 *@param sockfd socket que identifica al usuario
 *@param user nuevo nombre para el usuario
 *@return OK si todo fue bien, codigo de error <0 en otro caso
 */

int set_user(int sockfd, char * user){
	if (sockfd<0 || sockfd > MAXUSERS) return ERROR;
	pthread_mutex_lock(&users_mutex);
	if(!user){
		if(users[sockfd]){
			free(users[sockfd]);
			users[sockfd]=NULL;
		}
		pthread_mutex_unlock(&users_mutex);
		return OK;
	}else {
		if( strlen(user) >USERLEN){
			pthread_mutex_unlock(&users_mutex);
			return LONGUSER;
		}		
		if(!users[sockfd]){
			users[sockfd] = malloc(USERLEN * sizeof(char));
			if(!users[sockfd]){
				pthread_mutex_unlock(&users_mutex);
				return ERR_MALLOC;
			}
		}
		strcpy(users[sockfd], user);
		pthread_mutex_unlock(&users_mutex);
		return OK;
	}

}
/**
 *@brief Asigna un nuevo nick al socket sockfd
 *@param sockfd socket que identifica al usuario
 *@param nick nuevo nick para el usuario
 *@return OK si todo fue bien, codigo de error <0 en otro caso
 */
int set_nick(int sockfd, char * nick){
	if(sockfd<0 || sockfd > MAXUSERS) return ERROR;
	pthread_mutex_lock(&nicks_mutex);
	if(!nick){
		if(nicks[sockfd]){
			free(nicks[sockfd]);
			nicks[sockfd]=NULL;
		}
		pthread_mutex_unlock(&nicks_mutex);
		return OK;
	}else {
		if(strlen(nick)> NICKLEN){
			pthread_mutex_unlock(&nicks_mutex);
			return LONGNICK;
		}
		if(!nicks[sockfd]){
			nicks[sockfd] = malloc(NICKLEN * sizeof(char));
			if(!nicks[sockfd]){
				pthread_mutex_unlock(&nicks_mutex);
				return ERR_MALLOC;
			}
		}
		strcpy(nicks[sockfd], nick);
		pthread_mutex_unlock(&nicks_mutex);
		return OK;
	}
	
}
/**
 * En caso de que hubiera dos usuarios con el mismo nick (no deberia pasar)
 * devuelve el primero que encuentre
 *
 *@brief Devuelve el socket de un usuario identificado por su nick
 *@param nick nick del usuario cuyo socket queremos conseguir
 *@return i socket del usuario, NOT_FOUND si no se encontro ningun usuario con el nick especificado
 */
int get_sock_by_nick(char * nick){
	int i;
	if(!nick) return NULL_PARAM;
	pthread_mutex_lock(&nicks_mutex);
	for(i=START;i<MAXUSERS;i++){
		if(nicks[i] && !(strcmp(nicks[i], nick))){
		pthread_mutex_unlock(&nicks_mutex);
		return i;
		}
	}
	pthread_mutex_unlock(&users_mutex);
	return NOT_FOUND;


}
/**
 * En caso de que hubiera dos usuarios con el mismo nombre de usuario (no deberia pasar)
 * devuelve el primero que encuentre
 *
 *@brief Devuelve el socket de un usuario identificado por su nick
 *@param user nombre del usuario cuyo socket queremos conseguir
 *@return i socket del usuario, NOT_FOUND si no se encontro ningun usuario especificado
 */

int get_sock_by_user(char * user){
	int i;
	if(!user) return NULL_PARAM;
	pthread_mutex_lock(&users_mutex);
	for(i=START;i<MAXUSERS;i++){
		if(users[i] && !(strcmp(users[i], user))){
		pthread_mutex_unlock(&users_mutex);
		return i;
		}
	}
	pthread_mutex_unlock(&users_mutex);
	return NOT_FOUND;
}

/**
 *@brief Devuelve el nick del usuario asgnado a un socket 
 *@param sockfd socket que identifica al usuario
 *@return aux nick solicitado, NULL si no se encontro
 */
char * get_nick(int sockfd){
	char * aux;
	pthread_mutex_lock(&nicks_mutex);
	aux=nicks[sockfd];
	pthread_mutex_unlock(&nicks_mutex);
	return aux;

}

/**
 *@brief Devuelve el nombre de usuario del usuario asgnado a un socket 
 *@param sockfd socket que identifica al usuario
 *@return aux nombre de usuario solicitado, NULL si no se encontro
 */
char * get_user(int sockfd){
	char * aux;
	pthread_mutex_lock(&users_mutex);
	aux=users[sockfd];
	pthread_mutex_unlock(&users_mutex);
	return aux;

}
/**
 * Libera la lista de usuarios y nicks asi como las estructuras mutex
 *@brief Libera todos los recursos utilizados
 * 
 */
void free_all(void){
	int i;
	for(i=0; i<MAXUSERS; i++){
		free(users[i]);
		free(nicks[i]);
	}
	free(users);
	users=NULL;
	free(nicks);
	nicks=NULL;
	pthread_mutex_destroy(&users_mutex);
	pthread_mutex_destroy(&nicks_mutex);
	return;
}
/**
 * Devuelve el host asociado a un socket, reserva memoria para la cadena de 
 * caracteres devuelta. Es decir, esta debera ser liberada posteriormente
 * @brief Devuelve el host asociado al socket 
 * @param sockfd Puntero al socket 
 * @return host asociao a socketfd
 */
char * get_host(int * sockfd){
	char * user, * nick, * real, * host, * IP, * away;
	long creationTS, actionTS;
	long res, id=0;
	creationTS=actionTS=0;
	user=nick=real=host=IP=away=NULL;
	res=IRCTADUser_GetData (&id, &user, &nick, &real, &host, &IP, sockfd, &creationTS, &actionTS, &away);
	switch (res){
		case IRC_OK:
			free(user);
			free(nick);
			free(real);
			free(IP);
			free(away);
			return host;
		case IRCERR_NOENOUGHMEMORY:
			syslog(LOG_ERR,"IRCServ: Error en gethost(): memoria insuficiente");
			return NULL;
		default:
			return NULL;			

	}

}
/**
 * @bridef Wrapper para la funcion IRC_ComplexUser1459. Devuelve el prefijo de un usuario
 * utilizando unicamente el identificador del socket
 * @param prefix Puntero a la cadena de caracteres donde se almacena el prefijo obtenido,
 * 	es decir, el prefijo asociado al socket
 *	  psocket Puntero al socket del usuario cuyo prefijo se quiere obtener
 * @rerturn se devuelve el codigo de control de la funcion IRC_COmplexUser1459
 */
long ComplexUser_bySocket(char ** prefix, int  * psocket){
	long ret=0;
	char* host=NULL;
	if(!psocket) return ERROR;
	host = get_host(psocket);
	ret = IRC_ComplexUser1459 (prefix, get_nick(*psocket), get_user(*psocket), host, NULL);
	free(host);
	return ret;

}

