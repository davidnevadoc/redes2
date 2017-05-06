/**
  @file client_tools.h
  @breif Funciones de conexion del cliente.
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil
  @date 10/04/2017
  */
#ifndef CLIENT_TOOLS_H
#define CLIENT_TOOLS_H
void client_send(char *comm);
void client_connect( int *socket, char* server, int port);
void client_disconnect();
void client_stopsnd();
void client_rejectfile(char *nick);
void client_setauddest(char *ip, long port);
void client_sendaudreply(char  *nick);
void client_launchaudio();
#endif
