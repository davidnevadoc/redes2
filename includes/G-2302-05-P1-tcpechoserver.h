/**
 * @brief 
 * @file G-2302-05-P1-tcpechoserver.h
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#ifndef TCPECHOSERVER_H
#define TCPECHOSERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>
#include <string.h>
#include <unistd.h> 
#include <errno.h> 
#include <signal.h>
#include <pthread.h>

#define SUCCESS 0
#define FAILURE -1
#define MAX_QUEUE 10
#define BUFF_SIZE 500
#define MAXHILOS 100


#endif
