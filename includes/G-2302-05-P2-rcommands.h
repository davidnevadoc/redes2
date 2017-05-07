/**
  @file rcommands.c
  @breif Comandos recibidos del servidor.
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil
  @date 10/04/2017
  */
#ifndef RCOMMANDS_H
#define RCOMMANDS_H

#define MAX_RCOMM 512

void rdefault(char* comm);
void rNick(char *comm);
void rJoin(char * comm);
void rPart(char *comm);
void rKick(char *comm);
void rQuit(char *comm);
void rTopic(char *comm);
void rPrivMsg(char *comm);
void rRplList(char *comm);
void rRplTopic(char *comm);	
void rRplListEnd(char *comm);
void rRplWhoIsUser(char *comm);
void rRplEndOfWhoIs(char *comm);
void rRplWhoIsChannels(char * comm);
void rRplWhoIsOperator(char * comm);
void rRplWhoIsServer(char * comm);
void rRplMotd(char *comm);
void rRplEndMotd(char *comm);
void rRplEndOfNames(char *comm);
void rRplNamReply(char *comm);
void rRplWelcome(char * comm);
void rRplNowAway(char * comm);
void rMode(char *comm);
void rRplChannelModeIs(char * comm);
void rPing(char *comm);
void rErrNoSuchNick(char *comm);
void rErrNoSuchChannel(char *comm);
void rErrNoPrivileges(char *comm);
void rErrCanNotSendToChan(char *comm);
void rErrDefault(char * comm);

#endif
