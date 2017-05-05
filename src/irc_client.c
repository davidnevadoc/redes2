#include <redes2/ircxchat.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <stdlib.h>
#include <syslog.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "../includes/G-2302-05-P2-client_tools.h"
#include "../includes/G-2302-05-P2-tcp_tools.h"
#include "../includes/G-2302-05-P2-udp_tools.h"
#include "../includes/G-2302-05-P2-ucommands.h"
#include "../includes/G-2302-05-P2-rcommands.h"
/*
#ifndef ifr_newname
#define ifr_newname     ifr_ifru.ifru_slave
#endif
*/
/** 
 * @defgroup IRCInterface Interface
 *
 */

/** 
 * @defgroup IRCInterfaceCallbacks Callbaks
 * @ingroup IRCInterface
 *
 */

/** 
 * @addtogroup IRCInterfaceCallbacks
 * Funciones que van a ser llamadas desde el interface y que deben ser implementadas por el usuario.
 * Todas estas funciones pertenecen al hilo del interfaz.
 *
 * El programador puede, por supuesto, separar todas estas funciones en múltiples ficheros a
 * efectos de desarrollo y modularización.
 *
 * <hr>
 */
#define AUD_BUFF 1000
/** Variables globales*/
/*Socket de la conexion*/
int sockfd=-1;
/*Variable que regula la recepcion de mensajes*/
int stop=1;
/*Keep alive, queremos que la conexion no se cierre*/
int ka=0;
/*Alive. True si se mando un mensaje en los ultimos 20 segundos*/
bool alive =FALSE;
/*Variable que determina si usamos ssl*/
int ssl_global=0;
/*Mutex para el control de envio por el socket*/
pthread_mutex_t mutex_send;

pthread_t tsend_file;
int active_audio =0;
int playing_audio=0;

/*Lista de comandos de usuario*/
void (*ucommList[MAX_UCOMM])(char *comm );
/*Lista de mensajes recividos por el usuario*/
void (*rcommList[MAX_RCOMM])(char *comm);

/**
 *MI_Ip
 * Variable global que contiene la Ip de la interfaz que queremos usar.
 *
 * Define el comportamiento de la funcion get_ip(). Utilizada en el envio de 
 * ficheros y en las llamadas de voz.
 * Si esta variable esta definida a 000.000.000.000 entonces get_ip() devolvera la 
 * direccion de la primera interfaz proporcionada por el comando hostname -I
 * Si se desea utilizar una Ip diferente, mi_ip debe igualarse a la ip deseada
 * en el formato xxx.xxx.xxx.xxx.
 * Ejemplo:
 * 	char mi_ip[16]=192.168.1.25
 **/
char mi_ip[16]="000.000.000.000";
/**
 *Wrapper para funcion tcp_send y ssl_send.
 *Hace transparente el uso de ssl en el resto de la aplicacion
 *
 * @brief Envia un mensaje por el socket
 * @param comm Mensaje para enviar por el socket
 */
void client_send(char *comm){
	if(!comm) return;
	/*if(ssl_global) Enviar con ssl*/
	/*Desbloqueamos mutex de envio*/
	pthread_mutex_lock(&mutex_send);
	if( tcp_send(sockfd, comm)<0){
		syslog(LOG_ERR, "IRCCli: Error en tcp_send. Mensaje: %s", comm);
	} else {
		alive=TRUE;
	}
	/*Desbloqueamos mutex de envio*/
	pthread_mutex_unlock(&mutex_send);
	
}
void client_connect(int* socket, char *server, int port){
	long ret=0;
	struct in_addr dummy;
	bzero(&dummy, sizeof(dummy));
	/*if(ssl_global) Concectar con ssl*/
	if((ret=tcp_connect( socket,dummy, port, server))<0){
		syslog(LOG_ERR,"IRCCli: Error en tcp_connect. Error: %ld",ret);
	}
}


void client_disconnect(){
	stop=1;
	ka=FALSE;
	syslog(LOG_INFO,"IRCCli: Cliente desconectado");
	close(sockfd);
	pthread_mutex_destroy(&mutex_send);
	IRCInterface_RemoveAllChannels();
	IRCInterface_ChangeConectionSelected();
	IRCInterface_WriteSystem(NULL, "Disconnected()");
}
void client_rejectfile(char *nick){
	char buff[64]={0};
	char *pmsg=NULL;
	snprintf(buff,64,"\001REJECTED 1");
	IRCMsg_Privmsg(&pmsg, NULL, nick, buff);
	client_send(pmsg);
	free(pmsg);

}
void client_stopsnd(){
	IRCInterface_ErrorDialogThread("El fichero fue rechazado.");
	pthread_cancel(tsend_file);

}
/**
 * Funcion especifica que solo devuelve el nick utilizando la funcion
 * IRCInterface_GetMyUserInfo.
 *@brief Devuelve el nick del cliente
 *@return nick del cliente
 */
char * get_nick(){
	char *nick, *user, *real, *server, *pass;
	int port, ssl;
	nick=user=real=server=NULL;
	IRCInterface_GetMyUserInfo(&nick, &user, &real, &pass, &server, &port, &ssl);
	IRC_MFree(4, &user, &real, &server, &pass);
	return nick;

}
/**
 * Funcion que consigue la ip de la interfaz sobre la que esta abierto 
 * el socket
 * 
 * @brief Devuelve la ip del cliente
 * @return TODO
 */
char * get_ip(){
	FILE *f;
	char buff[256]={0};
	char *aux =NULL, *ip=NULL;
	ip=malloc(16*sizeof(char));	
	if(strcmp(mi_ip,"000.000.000.000")){
		strncpy(ip,mi_ip,16);
		return ip;
	}
	
	f=popen("hostname -I", "r");
	fgets(buff, 64, f);
	aux=strtok(buff," ");
	strncpy(ip,aux,16);
	pclose(f);
	return ip;

}

/**
 * Manda un ping cada 20 segundos si el usuario
 * ha estado inactivo
 *@bief Mantiene el cliente conectado
 *
 */
void tkeep_alive(){
	while(ka){
		if(alive){
			alive=FALSE;
		}else{
			client_send("PING LAG7734773477\r\n");
			syslog(LOG_INFO,"IRCCli: Ping enviado");
		}
		sleep(20);
	}
	syslog(LOG_INFO,"IRCCli: Keep Alive thread exit");
	pthread_exit(NULL);
}
/**
 * Funcion que espera y procesa comandos recibidos desde el servidor.
 * Esta funcion se ejecuta en un hilo separado de la interfaz.
 * 
 * @brief Recibe comandos del servidor
 *
 */
void atiende_comandos(void){
	long len =-1, num_com=-1;
	char buff[8192]={0};
	char *comm, *caux;
	comm=caux=NULL;
	syslog(LOG_INFO, "IRCCli: Recibiendo mensajes");
	while(!stop){
		//TODO annadir wrapper de esta funcion con semaforos. Lo annadire posiblmente al implementar client_rcv para meter tambien ssl
		len= tcp_recv(sockfd, buff, (size_t) 8192);
		switch(len){
			case 0:  
				syslog(LOG_INFO, "IRCCli:  Conexion cerrada por el servidor");
				client_disconnect();
				break;
			case -1:
				syslog(LOG_ERR, "IRCCli: Error en atiende_comandos %ld", len);
				return;
		}
		
		/*Separar comandos y procesar uno a uno*/
		caux = buff;
		do{
			caux = IRC_UnPipelineCommands(caux, &comm);
			syslog(LOG_INFO, "IRCCli: Comando: %s",comm);
			if((num_com=IRC_CommandQuery(comm))<0){
				syslog(LOG_ERR, "IRCCli: Comando invalido %ld", num_com);

			}else{
				if(comm) rcommList[num_com](comm);
			}
			free(comm);
		}while(caux);
	}

}


/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ActivateChannelKey IRCInterface_ActivateChannelKey
 *
 * @brief Llamada por el botón de activación de la clave del canal.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ActivateChannelKey (char *channel, char * key)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de activación de la clave del canal. El segundo parámetro es
 * la clave del canal que se desea poner. Si es NULL deberá impedirse la activación
 * con la función implementada a tal efecto. En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leídas.
 *
 * @param[in] channel canal sobre el que se va a activar la clave.
 * @param[in] key clave para el canal indicado.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_ActivateChannelKey(char *channel, char *key)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+k", key);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ActivateExternalMessages IRCInterface_ActivateExternalMessages
 *
 * @brief Llamada por el botón de activación de la recepción de mensajes externos.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ActivateExternalMessages (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de activación de la recepción de mensajes externos.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se activará la recepción de mensajes externos.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_ActivateExternalMessages(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+n", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ActivateInvite IRCInterface_ActivateInvite
 *
 * @brief Llamada por el botón de activación de canal de sólo invitación.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ActivateInvite (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de activación de canal de sólo invitación.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se activará la invitación.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_ActivateInvite(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+i", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ActivateModerated IRCInterface_ActivateModerated
 *
 * @brief Llamada por el botón de activación de la moderación del canal.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ActivateModerated (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de activación de la moderación del canal.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se activará la moderación.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_ActivateModerated(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+m", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ActivateNicksLimit IRCInterface_ActivateNicksLimit
 *
 * @brief Llamada por el botón de activación del límite de usuarios en el canal.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ActivateNicksLimit (char *channel, int * limit)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de activación del límite de usuarios en el canal. El segundo es el
 * límite de usuarios que se desea poner. Si el valor es 0 se sobrentiende que se desea eliminar
 * este límite.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se activará el límite de usuarios.
 * @param[in] limit límite de usuarios en el canal indicado.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_ActivateNicksLimit(char *channel, int limit)
{
	char *msg=NULL;
	char buff[32]={0};
	//itoa(limit, buff, 10);
	snprintf(buff, 32, "%d",limit);
	IRCMsg_Mode(&msg, NULL, channel, "+l", buff);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ActivatePrivate IRCInterface_ActivatePrivate
 *
 * @brief Llamada por el botón de activación del modo privado.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ActivatePrivate (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de activación del modo privado.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a activar la privacidad.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_ActivatePrivate(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+p", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ActivateProtectTopic IRCInterface_ActivateProtectTopic
 *
 * @brief Llamada por el botón de activación de la protección de tópico.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ActivateProtectTopic (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de activación de la protección de tópico.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a activar la protección de tópico.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/

void IRCInterface_ActivateProtectTopic(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+t", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ActivateSecret IRCInterface_ActivateSecret
 *
 * @brief Llamada por el botón de activación de canal secreto.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ActivateSecret (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de activación de canal secreto.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a activar el estado de secreto.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_ActivateSecret(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+s", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_BanNick IRCInterface_BanNick
 *
 * @brief Llamada por el botón "Banear".
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_BanNick (char *channel, char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Banear". Previamente debe seleccionarse un nick del
 * canal para darle voz a dicho usuario.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leídas.
 *
 * @param[in] channel canal sobre el que se va a realizar el baneo. En principio es un valor innecesario.
 * @param[in] nick nick del usuario que va a ser baneado
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_BanNick(char *channel, char *nick)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+b", nick);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_Connect IRCInterface_Connect
 *
 * @brief Llamada por los distintos botones de conexión.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	long IRCInterface_Connect (char *nick, char * user, char * realname, char * password, char * server, int port, boolean ssl)
 * @endcode
 * 
 * @description 
 * Función a implementar por el programador.
 * Llamada por los distintos botones de conexión. Si implementará la comunicación completa, incluido
 * el registro del usuario en el servidor.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leída.
 *
 *
 * @param[in] nick nick con el que se va a realizar la conexíón.
 * @param[in] user usuario con el que se va a realizar la conexión.
 * @param[in] realname nombre real con el que se va a realizar la conexión.
 * @param[in] password password del usuario si es necesaria, puede valer NULL.
 * @param[in] server nombre o ip del servidor con el que se va a realizar la conexión.
 * @param[in] port puerto del servidor con el que se va a realizar la conexión.
 * @param[in] ssl puede ser TRUE si la conexión tiene que ser segura y FALSE si no es así.
 *
 * @retval IRC_OK si todo ha sido correcto (debe devolverlo).
 * @retval IRCERR_NOSSL si el valor de SSL es TRUE y no se puede activar la conexión SSL pero sí una 
 * conexión no protegida (debe devolverlo).
 * @retval IRCERR_NOCONNECT en caso de que no se pueda realizar la comunicación (debe devolverlo).
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
long IRCInterface_Connect(char *nick, char *user, char *realname, char *password, char *server, int port, boolean ssl)
{
	char * comm=NULL; /*Comando de respuesta*/
	char mode=0;
	pthread_t rthread , kathread;
	/*Control de parametros*/
	/*Ver cuales son estrictamente necesarios*/
	if(!nick || !user || !realname || port<0 || !server){
		return IRCERR_NOCONNECT;
	}
	/*Variable para mantener activa la conexion*/
	ka=2;
	/*Hacer la variable ssl global*/
	ssl_global=ssl;
	/*Mutex para envio de mensajes*/
	pthread_mutex_init(&mutex_send, NULL);
		/*Desbloqueamos mutex de envio*/
	pthread_mutex_unlock(&mutex_send);
	client_connect( &sockfd, server ,port);
		syslog(LOG_INFO,
		 "IRCCli: Conexion establecida correctamente");
	/*Activar el hilo de recepcion de mensajes*/
	stop=0;
	/*Hilo de recepcion de mensajes*/
	pthread_create(&rthread, NULL,
		(void * (*)(void *)) atiende_comandos, NULL);
	pthread_detach(rthread);
	/*Registro en el servidor irc*/
	if(password && strlen(password) > 0){
		IRCMsg_Pass(&comm,NULL ,password);
		client_send( comm);
		free(comm);
	}
	IRCMsg_Nick(&comm, NULL, nick, NULL);
	client_send( comm);
	free(comm);
	mode=0x01;/*1 es el modo por defecto*/
	IRCMsg_User(&comm, NULL, user, &mode, realname);
	client_send(comm);
	free(comm);
	/*Lanzar hilo keep alive*/
	pthread_create(&kathread, NULL,
		(void * (*)(void *)) tkeep_alive, NULL);
	pthread_detach(kathread);	
	return IRC_OK;
}


/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DeactivateChannelKey IRCInterface_DeactivateChannelKey
 *
 * @brief Llamada por el botón de desactivación de la clave del canal.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_DeactivateChannelKey (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de desactivación de la clave del canal.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a desactivar la clave.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_DeactivateChannelKey(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-k", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DeactivateExternalMessages IRCInterface_DeactivateExternalMessages
 *
 * @brief Llamada por el botón de desactivación de la recepción de mensajes externos.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_DeactivateExternalMessages (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de desactivación de la recepción de mensajes externos.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a deactivar la recepción de mensajes externos.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_DeactivateExternalMessages(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-n", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DeactivateInvite IRCInterface_DeactivateInvite
 *
 * @brief Llamada por el botón de desactivación de canal de sólo invitación.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_DeactivateInvite (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de desactivación de canal de sólo invitación.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a desactivar la invitación.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_DeactivateInvite(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-i", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DeactivateModerated IRCInterface_DeactivateModerated
 *
 * @brief Llamada por el botón de desactivación  de la moderación del canal.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_DeactivateModerated (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de desactivación  de la moderación del canal.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a desactivar la moderación.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_DeactivateModerated(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-m", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DeactivateNicksLimit IRCInterface_DeactivateNicksLimit
 *
 * @brief Llamada por el botón de desactivación de la protección de tópico.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_DeactivateNicksLimit (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de desactivación  del límite de usuarios en el canal.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a desactivar el límite de usuarios.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_DeactivateNicksLimit(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-l", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DeactivatePrivate IRCInterface_DeactivatePrivate
 *
 * @brief Llamada por el botón de desactivación del modo privado.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_DeactivatePrivate (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de desactivación del modo privado.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @param[in] channel canal sobre el que se va a desactivar la privacidad.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_DeactivatePrivate(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-p", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DeactivateProtectTopic IRCInterface_DeactivateProtectTopic
 *
 * @brief Llamada por el botón de desactivación de la protección de tópico.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_DeactivateProtectTopic (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de desactivación de la protección de tópico.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a desactivar la protección de tópico.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/

void IRCInterface_DeactivateProtectTopic(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-t", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DeactivateSecret IRCInterface_DeactivateSecret
 *
 * @brief Llamada por el botón de desactivación de canal secreto.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_DeactivateSecret (char *channel)
 * @endcode
 * 
 * @description 
 * Llamada por el botón de desactivación de canal secreto.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] channel canal sobre el que se va a desactivar la propiedad de canal secreto.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_DeactivateSecret(char *channel)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-s", NULL);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_DisconnectServer IRCInterface_DisconnectServer
 *
 * @brief Llamada por los distintos botones de desconexión.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	boolean IRCInterface_DisconnectServer (char * server, int port)
 * @endcode
 * 
 * @description 
 * Llamada por los distintos botones de desconexión. Debe cerrar la conexión con el servidor.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.

 * @param[in] server nombre o ip del servidor del que se va a realizar la desconexión.
 * @param[in] port puerto sobre el que se va a realizar la desconexión.
 *
 * @retval TRUE si se ha cerrado la conexión (debe devolverlo).
 * @retval FALSE en caso contrario (debe devolverlo).
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
boolean IRCInterface_DisconnectServer(char *server, int port)
{
	//TODO
	client_disconnect();
	return TRUE;
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_ExitAudioChat IRCInterface_ExitAudioChat
 *
 * @brief Llamada por el botón "Cancelar" del diálogo de chat de voz.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_ExitAudioChat (char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Parar" del diálogo de chat de voz. Previamente debe seleccionarse un nick del
 * canal para darle voz a dicho usuario. Esta función cierrala comunicación. Evidentemente tiene que
 * actuar sobre el hilo de chat de voz.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] nick nick del usuario que solicita la parada del chat de audio.
 *
 * @retval TRUE si se ha cerrado la comunicación (debe devolverlo).
 * @retval FALSE en caso contrario (debe devolverlo).
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
boolean IRCInterface_ExitAudioChat(char *nick)
{
	active_audio=0;
	playing_audio=0;
	return TRUE;
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_GiveOp IRCInterface_GiveOp
 *
 * @brief Llamada por el botón "Op".
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_GiveOp (char *channel, char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Op". Previamente debe seleccionarse un nick del
 * canal para darle "op" a dicho usuario.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leídas.
 *
 * @param[in] channel canal sobre el que se va dar op al usuario.
 * @param[in] nick nick al que se le va a dar el nivel de op.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_GiveOp(char *channel, char *nick)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+o", nick);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_GiveVoice IRCInterface_GiveVoice
 *
 * @brief Llamada por el botón "Dar voz".
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_GiveVoice (char *channel, char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Dar voz". Previamente debe seleccionarse un nick del
 * canal para darle voz a dicho usuario.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leídas.
 *
 * @param[in] channel canal sobre el que se va dar voz al usuario.
 * @param[in] nick nick al que se le va a dar voz.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_GiveVoice(char *channel, char *nick)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "+v", nick);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_KickNick IRCInterface_KickNick
 *
 * @brief Llamada por el botón "Echar".
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_KickNick (char *channel, char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Echar". Previamente debe seleccionarse un nick del
 * canal para darle voz a dicho usuario.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leídas.
 *
 * @param[in] channel canal sobre el que se va a expulsar al usuario.
 * @param[in] nick nick del usuario que va a ser expulsado.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_KickNick(char *channel, char *nick)
{
	char * comm;
	IRCMsg_Kick(&comm,NULL, channel, nick, "This is Sparta! (kicked haha)");
	client_send( comm);
	free(comm);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_NewCommandText IRCInterface_NewCommandText
 *
 * @brief Llamada la tecla ENTER en el campo de texto y comandos.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_NewCommandText (char *command)
 * @endcode
 * 
 * @description
 * Llamada de la tecla ENTER en el campo de texto y comandos. El texto deberá ser
 * enviado y el comando procesado por las funciones de "parseo" de comandos de
 * usuario.
 *
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] comando introducido por el usuario.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_NewCommandText(char *command)
{
	long ret=-1;
	char * pmsg=NULL, *nick=NULL;
	nick=get_nick();
	/*Mensaje normal*/
	if((ret=IRCUser_CommandQuery (command)) == IRCERR_NOUSERCOMMAND){
		IRCInterface_WriteChannel(IRCInterface_ActiveChannelName(),nick, command);
		IRCMsg_Privmsg(&pmsg, NULL, IRCInterface_ActiveChannelName(), command);
		client_send(pmsg);
		free(pmsg);
		free(nick);
	/*Error en el parseo*/
	}else if(ret<0){
			syslog(LOG_ERR, "IRCCli: IRCUser_CommandQuery error %ld", ret);
			return;
	/*Otro comando*/
	}else{
		if(command){
			ucommList[ret](command);
			alive=TRUE;
		}
	}
	
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_NewTopicEnter IRCInterface_NewTopicEnter
 *
 * @brief Llamada cuando se pulsa la tecla ENTER en el campo de tópico.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_NewTopicEnter (char * topicdata)
 * @endcode
 * 
 * @description 
 * Llamada cuando se pulsa la tecla ENTER en el campo de tópico.
 * Deberá intentarse cambiar el tópico del canal.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * param[in] topicdata string con el tópico que se desea poner en el canal.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_NewTopicEnter(char *topicdata)
{
	char *msg=NULL;
	IRCMsg_Topic(&msg, NULL, IRCInterface_ActiveChannelName(), topicdata);
	client_send( msg);
	free(msg);
}


typedef struct{
	int sockfd;
	char * file; /*Cuidado, es binario*/
	long unsigned int size;

} thread_send_data;

void thread_send(thread_send_data * data){
	struct sockaddr_in client;
	socklen_t clilen;
	int p;
	int connfd=-1;
	syslog(LOG_INFO,"IRCCli: Esperando conexion para envio del fichero");
	clilen=sizeof(client);
	if((connfd=accept(data->sockfd,(struct sockaddr *) &client, &clilen))==-1){
		syslog(LOG_ERR, "IRCli: Error en accept() en thread_send() %d", errno);
		close(data->sockfd);
		free(data);
		pthread_exit(NULL);
	}
	syslog(LOG_INFO,"IRCCli: Conexion establecida. Enviando...");
	/*Enviar data en bucle*/
	do{
		if((p=send(connfd, data->file, (size_t) data->size, 0))==-1){
			syslog(LOG_ERR, "IRCli: Error en send() en thread_send() %d", errno);
			close(data->sockfd);
			free(data);
			pthread_exit(NULL);
		}
		data->file+=p;
		syslog(LOG_INFO, "IRCCli: Chunk enviado %dB", p);
	}while(p<data->size);
	syslog(LOG_INFO, "IRCCli: Fichero enviado");
	/*Cerrar socket*/
	close(data->sockfd);
	free(data);
	pthread_exit(NULL);
}
/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_SendFile IRCInterface_SendFile
 *
 * @brief Llamada por el botón "Enviar Archivo".
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_SendFile (char * filename, char *nick, char *data, long unsigned int length)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Enviar Archivo". Previamente debe seleccionarse un nick del
 * canal para darle voz a dicho usuario. Esta función como todos los demás callbacks bloquea el interface
 * y por tanto es el programador el que debe determinar si crea un nuevo hilo para enviar el archivo o
 * no lo hace.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leídas.
 *
 * @param[in] filename nombre del fichero a enviar.
 * @param[in] nick nick del usuario que enviará el fichero.
 * @param[in] data datos a ser enviados.
 * @param[in] length longitud de los datos a ser enviados.
 *
 * @retval TRUE si se ha establecido la comunicación (debe devolverlo).
 * @retval FALSE en caso contrario (debe devolverlo).
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
boolean IRCInterface_SendFile(char *filename, char *nick, char *data, long unsigned int length)
{	
	int sockl =0;
	struct sockaddr_in my_addr;
	socklen_t slen;
	char msg[512]={0};
	char *pmsg=NULL;
	char *ip_cutre=NULL;
	thread_send_data* tsdata=NULL;
	/*Abrir nuevo socket esperando  conexion*/
	if(tcp_listen(1, &sockl)!=IRC_OK){
		syslog(LOG_ERR, "IRCCli: Error en tcp_listen");
		return FALSE;
	}
	/*Obtener puerto asignado*/
	slen = sizeof(my_addr);
	getsockname(sockl, (struct sockaddr*)&my_addr,&slen );
	//Lanzar hilo que atienda en el puerto
	if ((tsdata=malloc(sizeof(thread_send_data)))==NULL){
		syslog(LOG_ERR, "IRCCli: Error al reservar memoria en IRCInterface_SendFile");
		return FALSE;
	}
	tsdata->sockfd = sockl;
	tsdata->file = data;
	tsdata->size = length;
	pthread_create(&tsend_file, NULL,(void * (*)(void *)) thread_send, tsdata );
	//tsdata=NULL; /*Eliminamos puntero loco. Libera el hilo*/
	pthread_detach(tsend_file);
	ip_cutre=get_ip();
	/*Construir mensaje especial de envio de fichero*/
	snprintf(msg, 512, "\001FSEND %s %s %s %d %lu\r\n",
 		get_nick(),filename,
		ip_cutre,
 		//inet_ntoa(my_addr.sin_addr),
		ntohs(my_addr.sin_port), length );
	free(ip_cutre);
	IRCMsg_Privmsg(&pmsg, NULL, nick, msg);
	client_send(pmsg);
	free(pmsg);
	return TRUE;
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_StartAudioChat IRCInterface_StartAudioChat
 *
 * @brief Llamada por el botón "Iniciar" del diálogo de chat de voz.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_StartAudioChat (char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Iniciar" del diálogo de chat de voz. Previamente debe seleccionarse un nick del
 * canal para darle voz a dicho usuario. Esta función como todos los demás callbacks bloquea el interface
 * y por tanto para mantener la funcionalidad del chat de voz es imprescindible crear un hilo a efectos
 * de comunicación de voz.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] nick nick del usuario con el que se desea conectar.
 *
 * @retval TRUE si se ha establecido la comunicación (debe devolverlo).
 * @retval FALSE en caso contrario (debe devolverlo).
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
boolean IRCInterface_StartAudioChat(char *nick)
{
	int aud_sock=-1;
	char *pmsg=NULL, *ip_cutre=NULL;
	char msg[512]={0};
	struct sockaddr_in addr;
	socklen_t slen=sizeof(addr);
	if(playing_audio==0){
		playing_audio=1;
		syslog(LOG_INFO, "IRCCli: Audio reanudado");
		return TRUE;
	}
	/*Abrimos socket udp en puerto cualqueira*/
	if (udp_open(&aud_sock, 0)==UDP_ERR) return FALSE;
	/*sin bind*/
	getsockname(aud_sock,(struct sockaddr*)&addr,&slen);
	syslog(LOG_INFO, "IRCCli: Socket udp abierto en %d",
		ntohs(addr.sin_port));
	ip_cutre=get_ip();
	/*Construir mensaje especial de envio de fichero*/
	snprintf(msg, 512, "\001AUDCHAT %s %d \r\n",
		ip_cutre,
		ntohs(addr.sin_port));
	IRCMsg_Privmsg(&pmsg, NULL, nick, msg);
	client_send(pmsg);
	free(pmsg);
	active_audio=1;
	playing_audio =1;
	return TRUE;
}
void taudio_send(){
	char buff[AUD_BUFF]={0};
	IRCSound_RecordFormat(PA_SAMPLE_S16BE,2);
	IRCSound_OpenRecord();
	while(active_audio){
		if(playing_audio){
			IRCSound_RecordSound(buff,AUD_BUFF);
			//udp_semd
		}
	}
	IRCSound_CloseRecord();
}

void taudio_rcv(){
	char buff[AUD_BUFF]={0};
	IRCSound_PlayFormat(PA_SAMPLE_S16BE,2);
	IRCSound_OpenPlay();
	while(active_audio){
		if(playing_audio){
			IRCSound_PlaySound(buff,AUD_BUFF);
			//udp_rcv
		}
	}
	IRCSound_ClosePlay();
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_StopAudioChat IRCInterface_StopAudioChat
 *
 * @brief Llamada por el botón "Parar" del diálogo de chat de voz.
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_StopAudioChat (char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Parar" del diálogo de chat de voz. Previamente debe seleccionarse un nick del
 * canal para darle voz a dicho usuario. Esta función sólo para la comunicación que puede ser reiniciada. 
 * Evidentemente tiene que actuar sobre el hilo de chat de voz.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * La string recibida no debe ser manipulada por el programador, sólo leída.
 *
 * @param[in] nick nick del usuario con el que se quiere parar el chat de voz.
 *
 * @retval TRUE si se ha parado la comunicación (debe devolverlo).
 * @retval FALSE en caso contrario (debe devolverlo).
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
boolean IRCInterface_StopAudioChat(char *nick)
{
	playing_audio=0;
	return TRUE;
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_TakeOp IRCInterface_TakeOp
 *
 * @brief Llamada por el botón "Quitar Op". 
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_TakeOp (char *channel, char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Quitar Op". Previamente debe seleccionarse un nick del
 * canal para quitarle "op" a dicho usuario.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leídas.
 *
 * @param[in] channel canal sobre el que se va a quitar op al usuario.
 * @param[in] nick nick del usuario al que se le va a quitar op.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_TakeOp(char *channel, char *nick)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-o", nick);
	client_send( msg);
	free(msg);
}

/**
 * @ingroup IRCInterfaceCallbacks
 *
 * @page IRCInterface_TakeVoice IRCInterface_TakeVoice
 *
 * @brief Llamada por el botón "Quitar voz". 
 *
 * @synopsis
 * @code
 *	#include <redes2/ircxchat.h>
 *
 * 	void IRCInterface_TakeVoice (char *channel, char *nick)
 * @endcode
 * 
 * @description 
 * Llamada por el botón "Quitar voz". Previamente debe seleccionarse un nick del
 * canal para darle voz a dicho usuario.
 * 
 * En cualquier caso sólo se puede realizar si el servidor acepta la orden.
 * Las strings recibidas no deben ser manipuladas por el programador, sólo leídas.
 *
 * @param[in] channel canal sobre el que se le va a quitar voz al usuario.
 * @param[in] nick nick del usuario al que se va a quitar la voz.
 *
 * @warning Esta función debe ser implementada por el alumno.
 *
 * @author
 * Eloy Anguiano (eloy.anguiano@uam.es)
 *
 *<hr>
*/
 
void IRCInterface_TakeVoice(char *channel, char *nick)
{
	char *msg=NULL;
	IRCMsg_Mode(&msg, NULL, channel, "-v", nick);
	free(msg);
}

void init_uComList(){
	int i=0;
	for(i=0; i<MAX_UCOMM; i++){
		ucommList[i]= udefault;
	}
	ucommList[UJOIN]=uJoin;
	ucommList[UPART]=uPart;
	ucommList[UNICK]=uNick;
	ucommList[UAWAY]=uAway;
	ucommList[UINVITE]=uInvite;
	ucommList[UKICK]=uKick;
	ucommList[ULIST]=uList;
	ucommList[UWHO]=uWho;
	ucommList[UWHOIS]=uWhois;
	ucommList[UTOPIC]=uTopic;
	ucommList[UNAMES]=uNames;
	ucommList[UHELP]=uHelp;
	ucommList[UMODE]=uMode;
	ucommList[UQUIT]=uQuit;
	ucommList[UMSG]=uPrivmsg;
}

void init_rComList(){
	int i=0;
	for(i=0; i<MAX_RCOMM; i++){
		rcommList[i]= rdefault;
	}
	rcommList[JOIN]=rJoin;
	rcommList[NICK]=rNick;
	rcommList[PART]=rPart;
	rcommList[PRIVMSG]=rPrivMsg;
	rcommList[KICK]=rKick;
	rcommList[QUIT]=rQuit;
	rcommList[MODE]=rMode;
	rcommList[RPL_WHOISCHANNELS]=rRplWhoIsChannels;
	rcommList[RPL_WHOISOPERATOR]=rRplWhoIsOperator;
	rcommList[RPL_WHOISSERVER]=rRplWhoIsServer;
	rcommList[RPL_WHOISUSER]=rRplWhoIsUser;
	rcommList[RPL_ENDOFWHOIS]=rRplEndOfWhoIs;
	rcommList[RPL_LIST]=rRplList;
	rcommList[RPL_LISTEND]=rRplListEnd;
	rcommList[RPL_NAMREPLY]=rRplNamReply;
	rcommList[RPL_ENDOFNAMES]=rRplEndOfNames;
	rcommList[RPL_MOTD]=rRplMotd;
	rcommList[RPL_ENDOFMOTD]=rRplEndMotd;
	rcommList[RPL_AWAY]=rRplWelcome;
	rcommList[RPL_NOWAWAY]=rRplNowAway;
	rcommList[RPL_CHANNELMODEIS]=rRplChannelModeIs;
	rcommList[PING]=rPing;
	/*Errores*/
	rcommList[ERR_NOSUCHNICK]=rErrNoSuchNick;
	rcommList[ERR_NOSUCHCHANNEL]=rErrNoSuchChannel;
	rcommList[ERR_NICKCOLLISION]=rErrDefault;
	//rcommList[ERR_ERRONEOUSNICKNAME]=rErrDefault;
	//rcommList[ERR_ALREADYREGISTERED]=rErrDefault;
	rcommList[ERR_NICKNAMEINUSE]=rErrDefault;
	rcommList[ERR_PASSWDMISMATCH]=rErrDefault;
	
}
/***************************************************************************************************/
/***************************************************************************************************/
/**                                                                                               **/
/** MMMMMMMMMM               MMMMM           AAAAAAA           IIIIIII NNNNNNNNNN          NNNNNN **/
/**  MMMMMMMMMM             MMMMM            AAAAAAAA           IIIII   NNNNNNNNNN          NNNN  **/
/**   MMMMM MMMM           MM MM            AAAAA   AA           III     NNNNN NNNN          NN   **/
/**   MMMMM  MMMM         MM  MM            AAAAA   AA           III     NNNNN  NNNN         NN   **/
/**   MMMMM   MMMM       MM   MM           AAAAA     AA          III     NNNNN   NNNN        NN   **/
/**   MMMMM    MMMM     MM    MM           AAAAA     AA          III     NNNNN    NNNN       NN   **/
/**   MMMMM     MMMM   MM     MM          AAAAA       AA         III     NNNNN     NNNN      NN   **/
/**   MMMMM      MMMM MM      MM          AAAAAAAAAAAAAA         III     NNNNN      NNNN     NN   **/
/**   MMMMM       MMMMM       MM         AAAAA         AA        III     NNNNN       NNNN    NN   **/
/**   MMMMM        MMM        MM         AAAAA         AA        III     NNNNN        NNNN   NN   **/
/**   MMMMM                   MM        AAAAA           AA       III     NNNNN         NNNN  NN   **/
/**   MMMMM                   MM        AAAAA           AA       III     NNNNN          NNNN NN   **/
/**  MMMMMMM                 MMMM     AAAAAA            AAAA    IIIII   NNNNNN           NNNNNNN  **/
/** MMMMMMMMM               MMMMMM  AAAAAAAA           AAAAAA  IIIIIII NNNNNNN            NNNNNNN **/
/**                                                                                               **/
/***************************************************************************************************/
/***************************************************************************************************/


int main (int argc, char *argv[])
{
	/* La función IRCInterface_Run debe ser llamada al final      */
	/* del main y es la que activa el interfaz gráfico quedándose */
	/* en esta función hasta que se pulsa alguna salida del       */
	/* interfaz gráfico.                                          */
	init_uComList();
	init_rComList();
	IRCInterface_Run(argc, argv);

	return 0;
}
