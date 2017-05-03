/**
 * @brief 
 * @file G-2302-05-P1-utilities.h
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include "G-2302-05-P1-ircserver.h"

/**
 * @brief Funcion que libera la estructura de datos data 
 * @param d Estructura de datos data
 */
void free_data( data * d);

/**
 * @brief Funcion que inicializa la estructura de datos data
 * @param connfd valor del socket
 * @return estructura de datos data inicializada
 */
data * data_init( int connfd);

#endif
