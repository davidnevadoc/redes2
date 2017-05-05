/**
  @file udp_tools.c
  @breif Utilidades para la conexion UDP
  @author David Nevado Catalan <david.nevadoc@estudiante.uam.es>
  @author Maria Prieto Gil  <maria.prietogil@estudiante.uam.es>
  @date 26/04/2017
  */

#include "../includes/G-2302-05-P2-udp_tools.h"
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <redes2/ircxchat.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/**
 * @brief Abre un socket UDP en un puerto
 * @param [out] psockfd Descriptor del socket que se conecta
 * @param [in] port Puerto al que conectar el socket
 * @return UDP_OK si se conecta con exito, UDP_ERR en otro caso
 */

int udp_open(int *psockfd, uint16_t port){
	
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	if(!psockfd || port<0){
		return UDP_ERR;
	}
	if((*psockfd=socket(AF_INET, SOCK_DGRAM, 0))==-1){
		syslog(LOG_ERR, "UDP: Error en socket(): %d", errno);
		return UDP_ERR;
	}
	addr.sin_family=AF_INET;
	addr.sin_port= htons(port);
	
	if(bind(*psockfd,(struct sockaddr*)&addr, sizeof(addr))==-1){
		syslog(LOG_ERR, "UDP: Error en bind(): %d", errno);
		return UDP_ERR;
	}
	return UDP_OK;
}
/**
 * @brief Envia datos a traves de un socket UDP
 * @param [in] sockfd Descriptor del socket por el que se envian los datos
 * @param [in] ip Direccion de destino en formato x.x.x.x
 * @param [in] port Puerto de destino
 * @param [in] data Datos para enviar
 * @param [in] size Tamanio en bytes de los datos
 * @return UDP_OK si se conecta con exito, UDP_ERR en otro caso
 */

int udp_send(int sockfd, char* ip, uint16_t port, void* data, unsigned long size){
	unsigned long rest=0, aux=0;
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	if(sockfd<0 || !ip || port<0 || !data || size<0){
		return UDP_ERR;
	}

	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &addr.sin_addr);
	addr.sin_port = htons(port);

	while(rest < size){
		if((aux=sendto(sockfd,data,size,0,
		   (struct sockaddr*)&addr, sizeof(addr)))<=0){
			syslog(LOG_ERR, "UDP Sendto() Error:%d", errno);
			return UDP_ERR;	
		}
		rest+=aux;

	}
	return UDP_OK;	
}
/**
 * @brief Recibe datos a traves de un socket UDP
 * @param [in] sockfd Descriptor del socket por el que se envian los datos
 * @param [in] ip Direccion de destino en formato x.x.x.x
 * @param [in] port Puerto de destino
 * @param [out] data Datos recibidos
 * @param [in] data_max Tamanio maximo del buffer de recepcion (data)
 * @param [out] len Tamanio de los datos recibidos en bytes
 * @return UDP_OK si se conecta con exito, UDP_ERR en otro caso
 */
int udp_rcv(int sockfd, char *ip, uint16_t port, void *data, unsigned long data_max, unsigned long *len ){
	struct sockaddr_in addr;
	socklen_t address_len;
	bzero(&addr, sizeof(addr));
	
	if(sockfd<0 || !ip || port<0 || !data || data_max<0 || !len){
		return UDP_ERR;
	}
	addr.sin_family=AF_INET;
	inet_pton(AF_INET, ip, &addr.sin_addr);
	addr.sin_port=htons(port);
	
	if ((*len=recvfrom(sockfd, data, data_max, 0,(struct sockaddr*) &addr , &address_len ))<0){
		syslog(LOG_INFO, "UDP: recvfrom() error: %d", errno);
		*len=0;
		return UDP_ERR;
	}
	return UDP_OK;

}
