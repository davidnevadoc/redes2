
/**
  @file ucommands.h
  @breif Comandos introducidos por el usuario
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil
  @date 10/04/2017
  */

#ifndef UCOMMANDS_H
#define UCOMMANDS_H

#define MAX_UCOMM 512
void udefault(char *comm);
void uJoin(char *comm);
void uPart(char *comm);
void uNick(char *comm);
void uAway(char *comm);
void uInvite(char *comm);
void uKick(char *comm);
void uList(char *comm);
void uWho(char *comm);
void uWhois(char *comm);
void uHelp(char *comm);
void uTopic(char *comm);
void uNames(char *comm);
void uQuit(char* comm);
void uMode(char * comm);
void uPrivmsg(char * comm);

#endif
