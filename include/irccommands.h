
#ifndef IRCCOMMANDS_H
#define IRCCOMMANDS_H

#include "ircserv.h"
/**
*@brief Función que atiende al commando PASS
*@param
*@return
*/
/*De momento solo muestra la contraseña introducida*/
int pass(data* d);

/**
*@brief Función que atiende al commando NICK
*@param
*@return
*/
//TODO Eviar mensaje al usuario informando del cambio si todo fue bien o de si hubo algun error
int nick(data* d);

/**
*@brief Función que atiende al commando USER
*@param
*@return
*/
/*long 	IRCParse_User (char *strin, char **prefix, char **user, char **modehost, char **serverother, char **realname)*/
int user(data* d);


/**
*@brief Función que atiende al commando QUIT
*@param
*@return
*/
int quit(data* d);


#endif
