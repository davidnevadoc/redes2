
/**
 * Cabecera del fichero principal del servidor IRC
 * @brief Biblioteca de semaforos
 * @file ircserv.h
 * @author Maria Prieto
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 12/02/2017
 */
#ifndef IRCSERV_H
#define IRCSERV_H

#include <redes2/irc.h>


#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>
#include <string.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <arpa/inet.h>





#define OK 0
#define ERROR -1

/**
 * @brief Estructura para el paso de parametros a la funcion de los hilos
 */
typedef struct _data{
	
	/** Contenido del mensaje*/
	char *mensaje;

	struct user * usuario;	
}data;


#include "utilities.h"
#endif
