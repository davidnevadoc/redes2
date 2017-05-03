/**
 * @brief 
 * @file G-2302-05-P1-utilities.c
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#include "../includes/G-2302-05-P1-utilities.h"

/**
 * @brief Funcion que inicializa los valores de un usuario 
 * @param connfd valor del socket
 * @return devuelve al usuario
 */
User * user_init(int connfd){
	User * user = NULL;
	if( (user = (User *) malloc(sizeof(User))) ==NULL ) return NULL;
	user->user=NULL;
	user->away=NULL;
	user->host=NULL;
	user->IP=NULL;
	user->Next=NULL;
	user->nick=NULL;
	user->password=NULL;
	user->real=NULL;
	user->socket=connfd;
	user->userId =NOTCREATED;
	return user;
}

/**
 * @brief Funcion que inicializa la estructura de datos data
 * @param connfd valor del socket
 * @return estructura de datos data inicializada
 */
data * data_init(int connfd){
	data * d =NULL;
	if ( (d = malloc(sizeof(data))) == NULL) return NULL;
	d->socket=connfd;
	d->IP=NULL;
	d->mensaje =NULL;
	d->stop=0;
	return d;


}

/**
 * @brief Funcion que libera la estructura de datos data 
 * @param d Estructura de datos data
 */
void free_data( data * d){
	free(d->IP);
	free(d);
}


