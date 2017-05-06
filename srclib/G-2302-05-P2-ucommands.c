/**
  @file ucommands.c
  @breif Comandos introducidos por el usuario
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil
  @date 10/04/2017
  */

#include <redes2/irc.h>
#include <redes2/ircxchat.h>
#include "../includes/G-2302-05-P2-ucommands.h"
#include "../includes/G-2302-05-P2-tcp_tools.h"
#include "../includes/G-2302-05-P2-client_tools.h"
#include <syslog.h>


//void (*listauComandos[MAX_UCOMM])(char *);

/**
 *@brief Comando por defecto. Informa en el cliente y en syslog de que el comando no esta implementado
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void udefault(char *comm){
	syslog(LOG_ERR, "IRCCli: Comando no implementado %s", comm);
	IRCInterface_WriteSystem(NULL, "Comando no implementado");
}
/**
 *@brief Comando JOIN
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uJoin(char *comm){
	char *msg, *channel, *pass;
	msg=channel=pass=NULL;
	if (IRCUserParse_Join(comm, &channel, &pass)!=IRC_OK){
		syslog(LOG_ERR,"IRCCli: Error en IRCUserParse_Join");
		return;
	}
	/*Prefijo y comentario a null*/
	IRCMsg_Join(&msg, NULL, channel, pass, NULL);
	client_send(msg);
	IRC_MFree(3, &msg, &channel, &pass);
}
/**
 *@brief Comando PART
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uPart(char *comm){
	char *msg, *farewell; /*farwell es el mensaje de despedida*/
	msg=farewell=NULL;
	if (IRCUserParse_Part(comm, &farewell)!=IRC_OK){
		syslog(LOG_ERR,"IRCCli: Error en IRCUserParse_Part");
		return;	
	}
	IRCMsg_Part(&msg, NULL, IRCInterface_ActiveChannelName(), farewell?farewell:"bye bye");
	client_send(msg);	
	IRC_MFree(2, &msg, &farewell);

}
/**
 *@brief Comando NICK
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uNick(char *comm){
	char *nick, *msg;
	nick=msg=NULL;
	if (IRCUserParse_Nick(comm, &nick)!=IRC_OK){
		syslog(LOG_ERR, "IRCCli: Error en IRCUSerParseNick");
		return;
	}
	IRCMsg_Nick(&msg, NULL, nick, NULL);
	client_send(msg);
	IRC_MFree(2, &msg, &nick);
}
/**
 *@brief Comando PRIVMSG
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uPrivmsg(char *comm){
	char *dest, *text, *msg;
	if(!comm) return;
	dest=text=msg=NULL;
	IRCUserParse_Priv(comm, &dest, &text);
	IRCMsg_Privmsg(&msg, NULL,dest,text);
	client_send(msg);
	IRC_MFree(3, &dest, &text,&msg);
}

/**
 *@brief Comando AWAY
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uAway(char *comm){
	char *msg, *away;/*away es el mensaje de ausencia del usuario*/
	msg=away=NULL;
	if(IRCUserParse_Away(comm, &away)){
		syslog(LOG_ERR, "IRCCli: Error en IRCUSerParseAway");
		return;
	}
	IRCMsg_Away(&msg, NULL, away);
	client_send(msg);
	IRC_MFree(2, &msg, &away);
}
/**
 *@brief Comando INVITE
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uInvite(char *comm){
	char *msg, *channel, *nick;
	msg=channel=nick=NULL;
	if (IRCUserParse_Invite(comm, &nick, &channel)!=IRC_OK){
		syslog(LOG_ERR,"IRCCli: Error en IRCUSerParseInvite");
		return;
	}
	if(!channel) channel=IRCInterface_ActiveChannelName();
	IRCMsg_Invite(&msg, NULL, nick, channel );
	client_send(msg);
	IRC_MFree(3, &msg, &nick, &channel);

}
/**
 *@brief Comando KICK
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uKick(char *comm){
	char *msg, *nick, *farewell;
	/*Farewell es el mensaje para el expulsado, debe ser ofensivo*/
	msg=nick=farewell=NULL;	
	if(IRCUserParse_Kick(comm, &nick, &farewell)!=IRC_OK){
		syslog(LOG_ERR, "IRCCli: Error en IRCUSerParseKick");
		return;
	}

	IRCMsg_Kick(&msg,NULL,IRCInterface_ActiveChannelName(),nick,
		  farewell?farewell:"This is Sparta!");
	client_send(msg);
	IRC_MFree(3, &msg, &nick, &farewell);



}
/**
 *@brief Comando LIST
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uList(char *comm){
	char *msg, *channel, *search_str;
	msg=channel=search_str=NULL;
	if(IRCUserParse_List(comm, &channel, &search_str) !=IRC_OK){
		syslog(LOG_ERR, "IRCCli: Error en IRCUserParseList");
		return;
	}
	IRCMsg_List(&msg, NULL, channel, NULL);
	client_send(msg);	
	IRC_MFree(3, &msg, &channel, & search_str);
}
/**
 *@brief Comando NAMES
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uNames(char *comm){
	char * channel, *target, *msg, *prefix;
	channel=target=msg=prefix=NULL;
	IRCParse_Names(comm,&prefix, &channel, &target);	
	IRCMsg_Names(&msg, NULL,
 	  channel?channel:IRCInterface_ActiveChannelName(),target );
	client_send(msg);
	IRC_MFree(4,&prefix, &channel, &target, &msg);
}
/**
 *@brief Comando TOPIC
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uTopic(char *comm){
	char * topic, *msg, *channel, *prefix;
	topic=msg=prefix=channel=NULL;
	IRCParse_Topic(comm,&prefix, &channel, &topic);
	IRCMsg_Topic(&msg, NULL,
	  channel?channel:IRCInterface_ActiveChannelName(), topic);
	client_send(msg);
	IRC_MFree(2, &msg, &topic, &prefix, &channel);
}
/**
 *@brief Comando WHO
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uWho(char *comm){
	char *mask , *msg;
	mask=msg=NULL;
	IRCUserParse_Who(comm, &mask);
	if(mask){
		IRCMsg_Who(&msg, NULL, mask, NULL);
		free(mask);
	}else{
		IRCMsg_Who(&msg, NULL,IRCInterface_ActiveChannelName(), NULL);
	}
	client_send(msg);	
	free(msg);

}
/**
 *@brief Comando WHOIS
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uWhois(char *comm){
	char *nick, *msg;
	nick=msg=NULL;
	IRCUserParse_Whois(comm, &nick);
	IRCMsg_Whois(&msg, NULL, NULL, nick);
	client_send(msg);	
	IRC_MFree(2, &nick, &msg);

}
/**
 *@brief Comando HELP
 *@param comm Comando recibido. Debe ser distinto de NULL
 */

void uHelp(char *command){
	//TODO MEter los comandos implementados
	IRCInterface_WriteSystem(NULL, "TODO");

}
/**
 *@brief Comando MODE
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uMode(char * comm){
	char *filter, *mode, *msg;
	filter=mode=msg=NULL;
	IRCUserParse_Mode(comm, &filter, &mode);
	IRCMsg_Mode(&msg, NULL,IRCInterface_ActiveChannelName(), mode, filter);
	client_send(msg);
	IRC_MFree(3, &filter, &msg, &mode);

}

/**
 *@brief Comando QUIT
 *@param comm Comando recibido. Debe ser distinto de NULL
 */
void uQuit(char* comm){
	char *msg, *farewell;
	msg=farewell=NULL;
	IRCUserParse_Quit(comm, &farewell);
	IRCMsg_Quit(&msg, NULL, farewell?farewell:"bye");
	client_send(msg);
	client_disconnect();
	IRC_MFree(2, &msg, &farewell);
}
