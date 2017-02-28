
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
#define MAX_MSG 512

int pass(void* info);
int nick(void* info);
int user(void* info);
int quit(void* info);



#endif
