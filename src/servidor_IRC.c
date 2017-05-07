/**
 * @brief Servidor IRC con SSL
 * @file servidor_iIRC.c
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#include "../includes/G-2302-05-P1-main.h"
#include "../includes/G-2302-05-P3-ssl_tools.h"
#include "../includes/G-2302-05-P2-tcp_tools.h"
#include <unistd.h> 
int stop=0;
void manejador_SIGINT(int sig){
	stop=1;
}


/**
 * Funcion Main del servidor. Gestiona los parametros pasados por el usuario
 * y abre un socket para ponerse a la espera de mensajes en el puerto deseado.
 * @brief Main del servidor
 * @return ERROR si se produce un fallo que provoca una salida inseperada
 * 	   OK salida controlada con el cierre del servidor
 */
int main(int argc, char *argv[]){
	
	exit(OK);
}




