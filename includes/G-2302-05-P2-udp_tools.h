/**
  @file udp_tools.h
  @breif Utilidades para la conexion UDP
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil
  @date 26/04/2017
  */

#ifndef UDP_TOOLS_H
#define UDP_TOOLS_H
#include <stdint.h>

#define UDP_OK 0
#define UDP_ERR -1

int udp_open(int *psockfd, uint16_t port);
int udp_rcv(int sockfd, char *ip, uint16_t port, void *data,
 unsigned long data_max, unsigned long *len);
int udp_send(int sockfd, char* ip, uint16_t port, void* data,
 unsigned long size);


#endif
