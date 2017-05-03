/**
 * @brief 
 * @file G-2302-05-P1-irccommands.h
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#ifndef IRCCOMMANDS_H
#define IRCCOMMANDS_H


#define MAXREPLY 256

#include "G-2302-05-P1-ircserver.h"
/**
*@brief Función que atiende al comando PASS
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int pass(data* d);

/**
*@brief Función que atiende al comando NICK
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int nick(data* d);

/**
*@brief Función que atiende al comando USER
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int user(data* d);


/**
*@brief Función que atiende al comando QUIT
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int quit(data* d);


/**
*@brief Función que atiende al comando JOIN
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int join(data* d);

/**
*@brief Función que atiende al comando LIST
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int list(data *d);

/**
*@brief Función que atiende al comando WHOIS
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int who(data *d);

/**
*@brief Función que atiende al comando WHOIS
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int whois(data *d);

/**
*@brief Función que atiende al comando NAMES
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int names(data *d);

/**
*@brief Función que atiende al comando PRIVMSG
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int privmsg(data *d);

/**
*@brief Función que atiende al comando PING
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int ping(data *d);

/**
*@brief Función que atiende al comando PONG
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int pong(data *d);

/**
*@brief Función que atiende al comando PART
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int part(data *d);

/**
*@brief Función que atiende al comando TOPIC
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int topic(data* d);

/**
*@brief Función que atiende al comando KICK
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int kick(data* d);

/**
*@brief Función que atiende al comando AWAY
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int away(data* d);

/**
*@brief Función que atiende al comando PASS
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int pass(data* d);

/**
*@brief Función que atiende al comando MOTD
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int motd(data *d );

/**
*@brief Función que atiende al comando MODE
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int mode(data *d );

int disconnect(data *d);
/**
*@brief Función que atiende al comando por defecto
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int comandoDefault(data* d);


#endif
