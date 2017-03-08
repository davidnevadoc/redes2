
#ifndef IRCCOMMANDS_H
#define IRCCOMMANDS_H


#define MAXREPLY 256

#include "ircserver.h"
/**
*@brief Función que atiende al comando PASS
*@param
*@return
*/
/*De momento solo muestra la contraseña introducida*/
int pass(data* d);

/**
*@brief Función que atiende al comando NICK
*@param
*@return
*/
//TODO Eviar mensaje al usuario informando del cambio si todo fue bien o de si hubo algun error
int nick(data* d);

/**
*@brief Función que atiende al comando USER
*@param
*@return
*/
/*long 	IRCParse_User (char *strin, char **prefix, char **user, char **modehost, char **serverother, char **realname)*/
int user(data* d);


/**
*@brief Función que atiende al comando QUIT
*@param
*@return
*/
int quit(data* d);


/**
*@brief Función que atiende al comando JOIN
*@param
*@return
*/
int join(data* d);


/**
*@brief Función que atiende al comando por defecto
*@param
*@return
*/
int comandoDefault(data* d);


#endif
