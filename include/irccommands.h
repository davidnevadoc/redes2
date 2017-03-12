
#ifndef IRCCOMMANDS_H
#define IRCCOMMANDS_H


#define MAXREPLY 256

#include "ircserver.h"
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
int whois(data *d);

/**
*@brief Función que atiende al comando NAMES
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int names(data *d);

int privmsg(data *d);
int ping(data *d);
int pong(data *d);
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
*@brief Función que atiende al comando por defecto
*@param d Estructura de datos con la informacion del hilo
*@return OK si el comando se ejecuto de forma correcta, ERROR en otro caso
*/
int comandoDefault(data* d);


#endif
