/**
  @file rcommands.c
  @breif Comandos recibidos del servidor.
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil
  @date 10/04/2017
  */

#include <sys/types.h>
#include <redes2/irc.h>
#include <redes2/ircxchat.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "../includes/G-2302-05-P2-rcommands.h"
#include "../includes/G-2302-05-P2-client_tools.h"
#include "../includes/G-2302-05-P2-tcp_tools.h"

/*Definicion de funciones privadas*/
char * getNick_Thread();
void IRCParseCasero_Fsend(char *strin, char ** fname, unsigned int *ip, unsigned short *port, unsigned long *size);

/*Variable global, caracter para mostrar por la interfaz en mensajes impersonales*/
char info[] ="*";
// TODO documentacion de esto
typedef struct{
	char *fname;	/*File name*/
	char *ip;	/*Direccion (IPv4, 4bytes)*/
	long port;	/*Puerto (en realidad es unsigned short)*/
	long length;	/*Tamano en bytes del archivo*/
} trcv_file_data;
void trcv_file(trcv_file_data * data);

typedef struct{
	char *ip;
	long port;
} taudio_data;
/**
 * @brief Atiende a comandos no implementados, no hace nada.
 * @param [in] comm Comando recibido
 */

void rdefault(char* comm){}
/**
 * @brief Atiende al comando Join
 * @param [in] comm Comando recibido
 */
void rJoin(char * comm){
	char * prefix, *channel, *key, *msg, *user, *host, *server, *nick, *mynick;
	char buff[128]={0};
	prefix=channel=key=msg=user=host=server=nick=mynick=NULL;
	/*Parseo del comando join*/
	IRCParse_Join(comm, &prefix, &channel, &key, &msg);
	/*Obtencion del nick del mensaje*/
	IRCParse_ComplexUser(prefix, &nick, &user, &host, &server);
	/*Obtencion de nuestro nick*/
	mynick=getNick_Thread();
	/*Alguien se unio al canal*/
	if(strcmp(nick, mynick)){/*
		if(!IRCInterface_QueryChannelExist(msg)){
			IRCInterface_AddNewChannelThread( msg,
			IRCInterface_ModeToIntMode("w"));	
		}*/
		snprintf(buff,128, "%s has joined.", nick);
		IRCInterface_WriteChannelThread(msg,info,buff);
		IRCInterface_AddNickChannelThread(msg, nick, nick, nick, nick, NONE);
	/*Nos unimos a un canal*/
	}else{
		/*Si no existe el canal se crea*/
		if(!IRCInterface_QueryChannelExist(msg)){
			IRCInterface_AddNewChannelThread( msg,
			IRCInterface_ModeToIntMode("w"));	
		}
		IRCInterface_WriteChannelThread(msg,info,"Has entrado al canal");

	}
	IRC_MFree(9, &prefix, &channel, &key, &msg, &nick, &user, &host, &server, &mynick);
}

/**
 * @brief Atiende al comando Part
 * @param [in] comm Comando recibido
 */
void rPart(char * comm){
	char *prefix,*channel,*msg,*nick,*user,*host,*server,*mynick;
	char buff[64]={0};
	prefix=channel=msg=nick=user=host=server=mynick=NULL;
	IRCParse_Part(comm, &prefix, &channel, &msg);
	IRCParse_ComplexUser(prefix, &nick, &user, &host, &server);
	mynick=getNick_Thread();
	
	/*Nos vamos*/
	if(!strcmp(mynick, nick)){
		snprintf(buff, 64,"You left the channel %s", channel);
		IRCInterface_RemoveChannelThread(channel);
		IRCInterface_WriteSystemThread(info, buff);
	/*Se fue alguien*/
	}else{
		snprintf(buff, 64, "%s left the channel.", nick);
		/*Segundo parametro null? no lo dice nadie*/
		IRCInterface_WriteChannelThread(channel,info,buff);
		IRCInterface_DeleteNickChannelThread(channel, nick);
	}
	IRC_MFree(8, &prefix, &channel, &msg,&nick, &user, &host, &server, &mynick );
}

/**
 * @brief Atiende al comando Kick
 * @param [in] comm Comando recibido
 */

void rKick(char * comm){
	char buff[128]={0};
	char *prefix,*channel,*msg,*nick,*user,*user2,*host,*server, *mynick,*farewell;
	channel=prefix=msg=nick=mynick=user=user2=host=server= farewell=NULL;
	IRCParse_Kick(comm, &prefix, &channel, &user2, &farewell);
	IRCParse_ComplexUser(prefix, &nick, &user, &host, &server);
	mynick=getNick_Thread();
	/*Nos echan*/
	if(!strcmp(mynick, user2)){
		snprintf(buff, 128, "You were kicked out of %s by %s. Reason: %s", channel, nick, farewell);
		IRCInterface_WriteSystemThread(info, buff);
		IRCInterface_RemoveChannelThread(channel);
	}else{
		snprintf(buff, 128, "%s was kicked out by %s. Reason: %s", user, nick, farewell );
		IRCInterface_WriteChannelThread(channel,NULL,buff);
		IRCInterface_DeleteNickChannelThread(channel, user2);
	}
	IRC_MFree(10, &prefix, &channel, &user, &farewell, &nick, &user, &host, &server, &server, &mynick);

}
void rQuit(char * comm){
	char buff[512]={0};
	char * pre, *msg, *nick,*user,*host,*serv,*channel;
	pre=msg=nick=user=host=serv=channel=NULL;
	IRCParse_Quit(comm, &pre ,&msg);
	IRCParse_ComplexUser(pre,&nick,&user,&host,&serv);
	channel=IRCInterface_ActiveChannelName();
	snprintf(buff,512,"%s has quit (%s)", nick, msg?msg:"");
	IRCInterface_WriteChannelThread(channel,info,buff);
	IRCInterface_DeleteNickChannel(channel,nick);
	IRC_MFree(6, &pre,&msg,&nick,&user,&host,&serv);
	

}
/**
 * @brief Atiende al comando PivMsg. Tambien contempla el inicio de envio de ficherosX
 * @param [in] comm Comando recibido
 */

void rPrivMsg(char *comm){
	char *prefix, *target, *msg, *nick, *host, *server, *user, *fnick;
	trcv_file_data *tr_data=NULL;
	taudio_data *taud_data=NULL;
	pthread_t trcv;
	prefix=target=msg=nick=host=server=user=fnick=NULL;
	IRCParse_Privmsg(comm, &prefix, &target, &msg);
	IRCParse_ComplexUser(prefix, &nick, &user, &host, &server);
	/*Recepcion de ficheros*/
	if(msg[0]==(char) 0x01){
		syslog(LOG_INFO, "IRCCli: Mensaje especial: %s", msg+1);
		//TODO Reservar memoria manualmente para quitar warnings de ISO C. De momento asi es mas facil y no da problemas.
		tr_data=malloc(sizeof(trcv_file_data));
		taud_data=malloc(sizeof(taudio_data));
		/*Oferta recibida de envio de fichero*/
		if(sscanf(msg, "\001FSEND %ms %ms %ms %li %li",
				&fnick,
				&(tr_data->fname),
				&(tr_data->ip),
				&(tr_data->port),
				&(tr_data->length)  )>0){
			free(taud_data);
			if (IRCInterface_ReceiveDialogThread(
				fnick, tr_data->fname)){
			/*Fichero aceptado*/
				pthread_create(&trcv, NULL,
				(void * (*)(void *)) trcv_file,
				 tr_data );
				pthread_detach(trcv);
	
			}else{
			/*Fichero rechazado*/
			free(tr_data);
			client_rejectfile(fnick);
			}
		
		/*Inicio de conversacion de chat*/
		}else if(sscanf(msg,"\001AUDCHAT %ms %li",
				&(taud_data->ip),
				&(taud_data->port))>0){
			free(tr_data);
			//TODO  start_aud();
			client_setauddest(taud_data->ip, taud_data->port);
			client_sendaudreply(nick); // no estoy seguro si es este nick
			client_launchaudio();
			//free(taud_data);
			
		
		/*Oferta de envio de fichero rechazada*/
		}else if(sscanf(msg, "\001REJECTED %li",&(tr_data->port))>0) {
			client_stopsnd(); 
			free(tr_data);
			free(taud_data);
		/**/
		}else if(sscanf(msg, "\001AUDREPLY %ms %li",
				&(taud_data->ip),
				&(taud_data->port))>0){
			client_setauddest(taud_data->ip, taud_data->port);
			client_launchaudio();
			free(taud_data);
			free(tr_data);
		}else{
			free(tr_data);
			free(taud_data);
		}

	/*Menasjes normales*/
	}else{
		/*Mensaje a canal*/
		if(IRCInterface_QueryChannelExist(target)){
			IRCInterface_WriteChannelThread(target, nick, msg);
		}else{
		/*Mensaje a otro usuario*/
			IRCInterface_AddNewChannelThread(nick, IRCInterface_ModeToIntMode("w"));
			IRCInterface_WriteChannelThread(nick, nick, msg);
		}
	}
	IRC_MFree(7,&prefix, &target, &msg, &nick, &host, &server, &user);

}
/**
 *TODO completar documentacion
 *@brief Funcion del hilo que se encarga de la recepcion del mensaje
 *
 */
void trcv_file (trcv_file_data * d){
	FILE *fout=NULL;
	unsigned long size, aux=0, auxw=0;	
	int sock;
	char buff[8192]={0};
	size_t bufflen = 8192;
	struct in_addr ip;
	if(!(fout=fopen(d->fname, "w"))){
 		syslog(LOG_ERR, "No se pudo escribir el fichero %s", d->fname);
 		close(sock);
		IRCInterface_ErrorDialogThread("No se pudo escribir el fichero. Tienes permiso de escritura?");
 		pthread_exit(NULL);
 	}
	syslog(LOG_INFO, "IRCCli: Se inicio el hilo de recepcion ");
	inet_pton(AF_INET, d->ip, &(ip.s_addr));
	if(tcp_connect(&sock,ip,(int)d->port, NULL)<0){
		syslog(LOG_ERR,"IRCCli: Error en client_connect, en trcv_file");
		pthread_exit(NULL);
	}
	size= (unsigned long) d->length;
	/*Abrimos fichero para escritura*/
	while(auxw<size){
		if((aux=recv(sock, buff, bufflen ,0))<0){
			syslog(LOG_ERR, "IRCCli: Error en recv %d",errno);
			fclose(fout);
			close(sock);
			remove(d->fname);
			IRCInterface_ErrorDialogThread("Conexion interrumpida. No se pudo recibir el fichero.");
			IRCInterface_WriteSystemThread(info,"No se pudo recibir el fichero");
			free(d);
			pthread_exit(NULL);
		}
		/*Si se cierra la conexion*/
		if(!aux){
			if(auxw==size) break;
			fclose(fout);
			close(sock);
			remove(d->fname);
			IRCInterface_ErrorDialogThread("Conexion interrumpida. No se pudo recibir el fichero.");
			IRCInterface_WriteSystemThread(info,"No se pudo recibir el fichero");
			free(d);
			pthread_exit(NULL);
			
		}
		fwrite(buff, 1, aux, fout);
		auxw+=aux;
		syslog(LOG_INFO,"IRCCli: Chunk recibido. %lu de %lu",auxw, size);
		aux=0;
	}
	/*Escrie datos obtenidos en fichero*/
	syslog(LOG_INFO,"IRCCli: Fichero %s recibido.", d->fname);
	IRCInterface_WriteSystemThread(info, "Fichero recibido");
	IRCInterface_ErrorDialogThread("Fichero recibido!");
	fclose(fout);
	close(sock);
	free(d);
	pthread_exit(NULL);
}


/**
 * @brief Atiende al comando Nick
 * @param [in] comm Comando recibido
 */

void rNick(char *comm){
	char *prefix, *nick, *nick2, *mynick, *msg, *user, *host, *server;
	char buff[128];
	prefix=nick=nick2=mynick=msg=user=host=server=NULL;
	IRCParse_Nick(comm, &prefix, &nick, &msg);
	IRCParse_ComplexUser(prefix, &nick2, &user, &host, &server);
	mynick=getNick_Thread();
	/*Un usuario cambia de nick*/
	if(strcmp(nick2, mynick)){
		snprintf(buff, 128, "You are now know as %s", msg);
	}else{
	/*Nos cambiamos de nick*/
		snprintf(buff,128, "%s is now known as %s", nick2,msg);
	}
	/*Mensaje en interfaz*/
	IRCInterface_WriteSystemThread(nick, buff);
	IRCInterface_ChangeNickThread(nick2, msg);
	IRC_MFree(8 ,&prefix, &nick, &msg, &nick2, &user, &host, &server, &mynick );
}
/**
 * @brief Atiende al mensaje TOPIC
 * @param [in] comm Comando recibido
 */
void rTopic(char *comm){
	char *pre, *nick, *topic, *c;
	char buff[512]={0};
	pre=nick=topic=c=NULL;
	IRCParse_Topic(comm, &pre, &c, &topic);
	snprintf(buff,512,"The topic is %s.", topic);
	IRCInterface_WriteChannelThread(c,nick,buff);
	IRC_MFree(4, &nick, &pre, &c, &topic);
}
/**
 * @brief Atiende al mensaje RplTopic
 * @param [in] comm Comando recibido
 */
void rRplTopic(char *comm){
	char *pre, *nick, *topic, *c;
	char buff[512]={0};
	pre=nick=topic=c=NULL;
	IRCParse_RplTopic(comm,&pre,&nick,&c,&topic);
	snprintf(buff,512,"The topic is %s.", topic);
	IRCInterface_WriteChannelThread(c,nick,buff);
	IRC_MFree(4, &nick, &pre, &c, &topic);
}
/**
 * @brief Atiende al mensaje RplList
 * @param [in] comm Comando recibido
 */
void rRplList(char *comm){
	char *prefix, *nick, *chan, *vi, *to;
	char buff[256]={0};
	
	prefix=nick=chan=vi=to=NULL;
	IRCParse_RplList(comm, &prefix, &nick, &chan, &vi, &to);
	snprintf(buff, 512, "%s\t\t%s\t\t%s", chan, vi, to?to:"");
	IRCInterface_WriteSystemThread(info,buff);
	IRC_MFree(5, &prefix, &nick, &chan, &vi, &to);	

}

/**
 * @brief Atiende al mensaje RplListEND
 * @param [in] comm Comando recibido
 */
void rRplListEnd(char *comm){
	/*char *prefix, *nick, *msg;
	prefix=nick=msg=NULL;
	IRCParse_RplListEnd(comm, &prefix, &nick, &msg);*/
	IRCInterface_WriteSystemThread(info, "End of LIST");
	/*IRC_MFree(3,&prefix, &nick, &msg);*/

}

/**
 * @brief Atiende al mensaje RplWhoIsUser
 * @param [in] comm Comando recibido
 */
void rRplWhoIsUser(char *comm){
	char buff[512]={0};
	char *prefix, *nick, *nick2, *name, *host, *real;
	prefix=nick=nick2=name=host=real=NULL;
	IRCParse_RplWhoIsUser(comm,&prefix,&nick,&nick2,&name,&host, &real);
	/*Este formato?*/
	snprintf(buff, 512, "[%s] (%s):%s)", nick, prefix, nick2);
	IRCInterface_WriteSystemThread(info, buff);
	IRC_MFree(6,&prefix,&nick,&nick2,&name,&host, &real);
	
}
/**
 * @brief Atiende al mensaje RplWhoIsChannels
 * @param [in] comm Comando recibido
 */
void rRplWhoIsChannels(char * comm){
	char *prefix,*nick,*nick2,*cstr;
	char buff[128]={0};
	prefix=nick=nick2=cstr=NULL;
	IRCParse_RplWhoIsChannels(comm,&prefix,&nick,&nick2,&cstr);
	if(cstr){
		snprintf(buff,128,"[%s] User channels %s",nick2,cstr);
		IRCInterface_WriteSystemThread(info, buff);	
	}
	IRC_MFree(4,&prefix, &nick, &nick2, &cstr);

}
/**
 * @brief Atiende al mensaje RplWhoIsOperator
 * @param [in] comm Comando recibido
 */
void rRplWhoIsOperator(char * comm){
	char *prefix,*nick,*nick2,*msg;
	char buff[128]={0};
	prefix=nick=nick2=msg=NULL;
	IRCParse_RplWhoIsOperator(comm, &prefix, &nick, &nick2, &msg);
	snprintf(buff,128,"[%s] is an operator",nick2);
	IRCInterface_WriteSystemThread(nick, buff);	
	IRC_MFree(4,&prefix, &nick, &nick2, &msg);
}
/**
 * @brief Atiende al mensaje RplWhoIsServer
 * @param [in] comm Comando recibido
 */
void rRplWhoIsServer(char * comm){
	char *prefix,*nick,*nick2,*server,*sinfo;
	char buff[128]={0};
	prefix=nick=nick2=server=sinfo=NULL;
	IRCParse_RplWhoIsServer(comm,  
&prefix, &nick , &nick2, &server, &sinfo);
	snprintf(buff,128,"[%s] Connected to %s: %s",nick2,server, sinfo);
	IRCInterface_WriteSystemThread(info, buff);
	IRC_MFree(5,&prefix, &nick , &nick2, &server, &sinfo);
}

/**
 * @brief Atiende al mensaje RplWhoIs
 * @param [in] comm Comando recibido
 */
void rRplEndOfWhoIs(char *comm){
	char *prefix, *nick, *name, *msg;
	char buff[512]={0};
	prefix=nick=name=msg=NULL;
	IRCParse_RplEndOfWhoIs(comm, &prefix, &nick, &name, &msg);
	snprintf(buff, 512, "[%s] End o WHOIS List", name );
	IRCInterface_WriteSystemThread(info, buff);
	IRC_MFree(4, &prefix, &nick, &name, &msg);
}
/**
 * @brief Atiende al mensaje RplEndOfNames
 * @param [in] comm Comando recibido
 */
void rRplEndOfNames(char *comm){
	/*char *prefix, *nick, *msg, *channel;
	prefix=nick=msg=channel=NULL;
	IRCParse_RplEndOfNames(comm, &prefix,&nick,&channel,&msg);*/
	IRCInterface_WriteSystemThread(info, "End of NAMES list");
	/*IRC_MFree(4,&prefix, &nick, &msg, &channel);*/

}
/**
 * @brief Atiende al mensaje RplNamReply
 * @param [in] comm Comando recibido
 */
void rRplNamReply(char *comm){
	char *prefix,*nick,*t,*ch,*msg,*n2,*user,*h,*ser,*aux;
	char buff[512]={0};
	prefix=nick=t=ch=msg=n2=user=h=ser=aux=NULL;
	IRCParse_RplNamReply(comm,&prefix,&nick,&t,&ch,&msg);
	//IRCParse_ComplexUser(prefix,&n2,&user,&h,&ser);
	snprintf(buff,512,"Users on %s: %s",ch,msg);
	IRCInterface_WriteChannelThread(ch,info,buff);
	aux=strtok(msg, " ");
	while(aux){
		switch(aux[0]){
			case '@':
				IRCInterface_AddNickChannel(ch,aux+1,aux,aux,aux,OPERATOR);
				break;
			case '+':
				IRCInterface_AddNickChannel(ch,aux+1,aux,aux,aux,VOICE);
				break;
			default:
				IRCInterface_AddNickChannel(ch,aux,aux,aux,aux,NONE);
		}		
		aux=strtok(NULL," ");	
	}
	IRC_MFree(9,&prefix,&nick,&t,&ch,&msg,&n2,&user,&h,&ser,&aux);
}
/**
 * @brief Atiende al mensaje RplWMotd
 * @param [in] comm Comando recibido
 */
void rRplMotd(char * comm){
	char *prefix, *nick, *msg;
	prefix=nick=msg=NULL;
	IRCParse_RplMotd(comm, &prefix,&nick,&msg);
	IRCInterface_WriteSystemThread(NULL, msg);
	IRC_MFree(3, &prefix, &nick, &msg);
}
/**
 * @brief Atiende al mensaje RplEndMotd
 * @param [in] comm Comando recibido
 */
void rRplEndMotd(char * comm){
	//char * prefix, *nick, *msg;
	//prefix=nick=msg=NULL;
	//IRCParse_RplEndOfMotd(comm, &prefix, &nick, &msg);
	IRCInterface_WriteSystemThread(NULL,"End of MOTD command.");
	
}
/**
 * @brief Atiende al mensaje RplWelcome
 * @param [in] comm Comando recibido
 */
void rRplWelcome(char * comm){
	
	char *prefix, *nick, *msg;
	prefix=nick=msg=NULL;
	char buff[512]={0};
	IRCParse_RplWelcome(comm, &prefix, &nick, &msg);
	snprintf(buff, 512, "Server message: %s", msg);
	IRCInterface_WriteSystemThread(info, buff);
	IRC_MFree(3, &prefix, &nick, &msg);

}
/**
 * @brief Atiende al mensaje RplAway
 * @param [in] comm Comando recibido
 */

void rRplAway(char * comm){
	char * prefix, *nick, *nick2, *msg;
	char buff[512]={0};
	prefix=nick=nick2=msg=NULL;
	IRCParse_RplAway(comm,&prefix,&nick,&nick2,&msg);
	snprintf(buff,512,"The user is away: %s", msg);
	IRCInterface_WriteChannelThread(nick2,nick2,buff);
	IRC_MFree(4,&prefix,&nick,&nick2,&msg);
}
/**
 * @brief Atiende al mensaje RplNowAway
 * @param [in] comm Comando recibido
 */

void rRplNowAway(char * comm){
	char * prefix, *nick, *c, *msg;
	prefix=nick=c=NULL;
	IRCParse_RplNowAway(comm, &prefix,&nick,&msg);
	c=IRCInterface_ActiveChannelName();
	if(strcmp(c, "System")){
		IRCInterface_WriteChannelThread(c,info,"You are now away");	
	}else{
		IRCInterface_WriteSystemThread(info,"You are now away");
	}
	IRC_MFree(3, &prefix, &nick,&msg);
}
/**
 * @brief Atiende al mensaje MODE
 * @param [in] comm Comando recibido
 */

void rMode(char *comm){
	int mint=0;
	char buff[256]={0};
	char *prefix, *cuser, *mode, *user;
	//char *host, *real, *u2;
	//nickstate ns;
	//char buff[512]={0};
	prefix=cuser=mode=user=NULL;
	//host=real=u2=NULL; 
	IRCParse_Mode(comm,&prefix,&cuser,&mode,&user);
	//IRCInterface_GetNickChannelInfoThread (user, cuser, &u2, &real, &host);
	/*En un canal*/
	if(cuser[0]=='#'){ // si el priemer  caracter es &?
		if(!user){
			/*Modos del canal*/
			switch(mode[0]){
				case '+' :
					mint= IRCInterface_ModeToIntModeThread(mode+1);
					IRCInterface_AddModeChannelThread(cuser,mint);
					break;
				case '-' :
					mint= IRCInterface_ModeToIntModeThread(mode+1);
					IRCInterface_DeleteModeChannelThread(cuser,mint);
					break;
				default:
					mint= IRCInterface_ModeToIntMode(mode);
					IRCInterface_SetModeChannelThread(cuser, mint);
			}
		/*En un usuario del canal*/
		} else {
			switch(mode[0]){
				case '+' :
					switch(mode[1]){
						case 'o':
							IRCInterface_ChangeNickStateChannelThread(cuser, user, OPERATOR);
							break;
						case 'v':
							IRCInterface_ChangeNickStateChannelThread(cuser, user, VOICE);
							break;
						default:
							syslog(LOG_ERR,"IRCCli: Opcion de mode no reconocida");
					}
					break;
				case '-' :
					switch(mode[1]){
						case 'o':
							IRCInterface_ChangeNickStateChannelThread(cuser, user, NONE);
							break;
						case 'v':
							IRCInterface_ChangeNickStateChannelThread(cuser, user, NONE);
							break;
						default:
							syslog(LOG_ERR,"IRCCli: Opcion de mode no reconocida");
					}
					break;
				default :
					syslog(LOG_ERR,"IRCCli: Opcion de mode no reconocida");
			}
		
		}
	/*En un usuario*/
	} else {
		snprintf(buff,256,"Your mode is %s.", mode);
		IRCInterface_WriteSystemThread(info, buff);
	}

}
/**
 * @brief Atiende al mensaje RplChannelModeIs
 * @param [in] comm Comando recibido
 */

void rRplChannelModeIs(char * comm){
	int mint=0;
	char *prefix, *nick, *channel, *m;
	char buff[512]={0};
	prefix=nick=channel=m=NULL;
	IRCParse_RplChannelModeIs(comm,&prefix,&nick,&channel,&m);
	mint=IRCInterface_ModeToIntMode(m);
	IRCInterface_SetModeChannelThread(channel, mint);
	snprintf(buff,512,"Channel mode is %s", m);
	IRCInterface_WriteChannelThread(channel, info, buff);
	IRC_MFree(4, &prefix, &nick, &channel, &m);
}
/**
 * @brief Atiende al mensaje PING
 * @param [in] comm Comando recibido
 */

void rPing(char * comm){
	char * prefix, *s, *s2, *m, *msg;
	prefix=s=s2=m=msg=NULL;
	IRCParse_Ping(comm,&prefix,&s,&s2,&m);
	IRCMsg_Pong(&msg, NULL, s, s2, m);
	client_send(msg);
	syslog(LOG_INFO, "IRCCli: Pong");
	IRC_MFree(5, &prefix, &s,&s2,&m,&msg);
}	

/**
 * @brief Atiende al mensaje ErrNoSuchNick
 * @param [in] comm Comando recibido
 */
void rErrNoSuchNick(char *comm){
	char *pre, *nick, *nickn, *text, *msg;
	char buff[512]={0};
	pre=nick=nickn=text=msg=NULL;
	IRCParse_ErrNoSuchNick(comm, &pre,&nick,&nickn,&text);
	snprintf(buff,512,"No such nick");
	IRCInterface_WriteSystemThread(info, buff);
	client_send(msg);
	IRC_MFree(5,&pre,&nick,&nickn,&text,&msg);
}
/**
 * @brief Atiende al mensaje ErrNoSuchChannel
 * @param [in] comm Comando recibido
 */
void rErrNoSuchChannel(char *comm){
	char *pre, *nick, *nickn, *text, *msg;
	char buff[512]={0};
	pre=nick=nickn=text=msg=NULL;
	IRCParse_ErrNoSuchChannel(comm, &pre,&nick,&nickn,&text);
	snprintf(buff,512,"No such channel");
	IRCInterface_WriteSystemThread(info, buff);
	client_send(msg);
	IRC_MFree(5,&pre,&nick,&nickn,&text,&msg);
}
/**
 * @brief Atiende a cualquier mensaje de error de conexion
 * @param [in] comm Comando recibido
 */
void rErrDefault(char * comm){
	char buff[512]={0};
	snprintf(buff,512,"Error connecting with server : %s.", comm);
	IRCInterface_ErrorDialogThread(buff);
}

/**
 *Funcion privada.
 * Wrapper de IRCInterface_GetMyUserInfoThread. 
 * Usada para conseguir tan solo el nick del usuario.
 * @return nick del usuario
 */	 
char * getNick_Thread(){
	char *nick, *user, *real, *server, *pass;
	int port, ssl;
	nick=user=real=server=pass=NULL;
	IRCInterface_GetMyUserInfoThread(&nick, &user, &real, &pass,
		&server, &port, &ssl);
	IRC_MFree(4,&user,&real,&server, &pass);
	return nick;

}
