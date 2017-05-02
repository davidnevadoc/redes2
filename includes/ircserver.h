/**
 * @brief Biblioteca de semaforos
 * @file ircserver.h
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 12/02/2017
 */
#ifndef IRCSERVER_H
#define IRCSERVER_H

#include <redes2/irc.h>


#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>
#include <string.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>


#define SERV_NAME "valknutserver"

/*TODO Revisar poniendo datos del RFC*/
#define USERLEN 16
#define NICKLEN 10
#define MAXUSERS 512

#define OK 0
#define ERROR -1

#define NULL_PARAM -2
#define NOT_FOUND -3
#define LONGNICK -4
#define LONGUSER -5
#define NOTCREATED -6
#define ERR_MALLOC -7


/**
 * @brief Estructura para el paso de parametros a la funcion de los hilos
 */
typedef struct _data{
	/*socket del usuario (y del hilo)*/
	int socket;
	/*Direccion Ip del cliente conenctado al socket*/
	char * IP;
	/** Contenido del mensaje*/
	char *mensaje;
	/* Variable que controla el cierre de la conexion*/
	int stop;
}data;

int init_var(void);
/**
 *@brief Asigna un nuevo nombre de usuario al socket sockfd
 *@param sockfd socket que identifica al usuario
 *@param user nuevo nombre para el usuario
 *@return OK si todo fue bien, codigo de error <0 en otro caso
 */

int set_user(int sockfd, char * user);
/**
 *@brief Asigna un nuevo nick al socket sockfd
 *@param sockfd socket que identifica al usuario
 *@param nick nuevo nick para el usuario
 *@return OK si todo fue bien, codigo de error <0 en otro caso
 */
int set_nick(int sockfd, char * nick);
/**
 * En caso de que hubiera dos usuarios con el mismo nick (no deberia pasar)
 * devuelve el primero que encuentre
 *
 *@brief Devuelve el socket de un usuario identificado por su nick
 *@param nick nick del usuario cuyo socket queremos conseguir
 *@return i socket del usuario, NOT_FOUND si no se encontro ningun usuario con el nick especificado
 */
int get_sock_by_nick(char * nick);
/**
 * En caso de que hubiera dos usuarios con el mismo nombre de usuario (no deberia pasar)
 * devuelve el primero que encuentre
 *
 *@brief Devuelve el socket de un usuario identificado por su nick
 *@param user nombre del usuario cuyo socket queremos conseguir
 *@return i socket del usuario, NOT_FOUND si no se encontro ningun usuario especificado
 */

int get_sock_by_user(char * user);


/**
 *@brief Devuelve el nick del usuario asgnado a un socket 
 *@param sockfd socket que identifica al usuario
 *@return aux nick solicitado, NULL si no se encontro
 */
char * get_nick(int sockfd);

/**
 *@brief Devuelve el nombre de usuario del usuario asgnado a un socket 
 *@param sockfd socket que identifica al usuario
 *@return aux nombre de usuario solicitado, NULL si no se encontro
 */
char * get_user(int sockfd);
/**
 *@brief Libera todos los recursos utilizados
 */
void free_all(void);
char * get_host(int * sockfd);
long ComplexUser_bySocket(char ** prefix, int  * psocket);
#include "utilities.h"
#endif
