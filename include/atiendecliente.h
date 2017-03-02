#ifndef ATIENDECLIENTE_H
#define ATIENDECLIENTE_H


#define IPV4ADDRSIZE 4
#define IPV6ADDRSIZE 16


#define BUFF_SIZE 500

#include <pthread.h>
#include "ircserv.h"
#include "irccommands.h"


/**
*
*@brief Wrapper de la funcion atiende_cliente
*@param cli sockaddr con direccion del socket del cliente que establecio la conexion
*
*@return Devuelve SUCCESS si todo va bien, ERROR en caso contrario
* Wrapper de la funcion atiende_cliente. Se encarga de reservar memoria para la 
* estructura que se pasa a la funcion atiende_cliente, asi como de gestionar 
* los hilos y hacer diversos controles de error.
*/
int Atiende_cliente(struct sockaddr cli, int connfd);


#endif
