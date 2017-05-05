/**
  @file tcp_tools.c
  @breif Utilidades para la conexion TCP
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil
  @date 26/04/2017
  */
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>

/**
 * @brief Envia un mensaje a traves de un socket TCP
 * @param sockfd Identificador del socket 
 * @param command Mensaje a enviar
 * @return Codigo de Error o 0
 */
long tcp_send(int sockfd, char * command){
	long ret=0;
	if ((ret =send(sockfd, command, strlen(command), 0))<0){
		syslog(LOG_ERR,"IRC: Error en send: codigo %ld", ret);
		return ret;
	}
	return 0;
}

/**
 * @brief Recibe un mensaje a traves de un socket TCP
 * @param [in] sockfd Identificador del socket 
 * @param [out] msg Mensaje recibido
 * @param [in] max Tamanio maximo de msg en bytes 
 * @return Numero de bytes leidos o -1 en caso de error
 */
long tcp_recv(int sockfd, char* msg, size_t max){
	size_t aux=0;
	if(sockfd<0 || !msg || !max ) return -1;
	/*Inicializa a 0*/
	bzero(msg, max);
	aux=recv(sockfd, msg, max, 0);
	return aux;
}
/**
 * @brief Crea un socket TCP y lo pone a la escucha. Toma cualquier puerto.
 * @param [in] n_cli Numero de clientes admitidos por el socket
 * @param [out] socklisten Identificador del socket conectado
 * @return 0 si se conecto con exito, -1 en caso contrario
 */
long tcp_listen(int n_cli, int *socklisten){
	int sockl=0;
	if(n_cli<1 || !socklisten){
		return -1;
	}
	/*Incializar estructura del socket*/
	if ( (sockl=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		syslog(LOG_ERR, "IRC: Error en socket()");
		return -1;
	}
	/*No llamamos a bind*/
	/*Puerto a la escucha*/
	if (listen(sockl,n_cli)==-1){
		syslog(LOG_ERR, "IRC: Error en sockfd(): %d", errno);
		return -1;
	}
	*socklisten=sockl;
	return 0;
}
/**
 * 
 *@brief Funcion que establece la conexion del cliente con el servidor
 *@param [in] server Servidor al que conectarse
 *@param [in] port Puerto al que conectarse
 *@param [out] Socket conectado
 */

long tcp_connect( int *sock, struct in_addr ip,int port,char *server){
	int sockfd=-1;
	struct hostent * addr;
	struct sockaddr_in serv;
	/*Inicializacion a 0 de las estructuras*/
	bzero(&serv, sizeof(serv));
	bzero(&addr, sizeof(addr));
	/*Completamos la estructura del socket destino*/
	serv.sin_family=AF_INET;
	serv.sin_port=htons(port);
	if(!server){
		serv.sin_addr=ip;
	}else{	
		addr=gethostbyname(server);
		bcopy((char *)addr->h_addr_list[0], &serv.sin_addr.s_addr, addr->h_length);
	}
	//inet_pton(AF_INET,server , &serv);

	/*Incializar estructura del socket*/
	if ( (sockfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		syslog(LOG_ERR, "IRC: Error en sockfd()");
		return -1;
	}
	/*Conectar socket*/
	if ( connect(sockfd, (struct sockaddr *) &serv, sizeof(serv)) ){
		syslog(LOG_ERR, "IRC: Error en conncet(): %d", errno);
		return -1;
	
	}
	/*Devolvemos el valor del socket (sockfd) en la variable socket*/
	*sock=sockfd;
	return 0;

}


