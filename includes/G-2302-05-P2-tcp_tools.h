/**
  @file tcp_tools.h
  @breif Utilidades para la conexion TCP
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil
  @date 26/04/2017
  */
#ifndef TCP_TOOLS_H
#define TCP_TOOLS_H

#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
long tcp_send(int sockfd, char * command);
long tcp_recv(int sockfd, char* msg, size_t max);
long tcp_listen(int n_cli, int *socklisten);
long tcp_connect( int *sock,struct in_addr ip, int port, char *server);

#endif
