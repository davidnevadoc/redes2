/**
 * @brief Cliente echo SSL
 * @file cliente_echo.c
 * @author Maria Prieto Gil maria.prietogil@estudiante.uam.es
 * @author David Nevado Catalan david.nevadoc@estudiante.uam.es
 * @date 02/05/2017
 */

#include "../includes/G-2302-05-P3-ssl_tools.h"
#include "../includes/G-2302-05-P2-tcp_tools.h"
#include <stdio.h>

/**
*@brief Funcion main 
*@param argc Numero de argumentos
*@param argv Argumentos
*@return 0
*/
int main(int argc, char** argv) {
	int socket;
	int port = 6111;
	char buf[MAX_MSG_SSL];
	struct in_addr ip;

	if(argc == 2){
		port = atoi(argv[1]);
	}

    inicializar_nivel_SSL();

	
	if(fijar_contexto_SSL(CLIENT_KEY , CLIENT_CERT, CA_CERT) == SSL_ERR){
		fprintf(stderr, "Error fijando contexto\n");
		exit(1);
	}
	/*Abro conexion TCP*/
	if(tcp_connect(&socket, ip, port, "localhost") == -1){
		fprintf(stderr, "Error abriendo conexion tcp\n");
		exit(1);
	}

	if(conectar_canal_seguro_SSL(socket) == SSL_ERR){
		fprintf(stderr, "Error canal no seguro\n");
		exit(1);
	}

	if(evaluar_post_connectar_SSL(socket) == SSL_ERR) {
        fprintf(stderr, "Error del certificador\n");
        exit(1);
    }

	while(1){
		fflush(stdin);
		fscanf(stdin, "%s", buf);
		enviar_datos_SSL(socket, buf);
		/*si el mensaje es exit salgo*/
		if(!strcmp(buf, "exit")) break;
        memset(buf, 0, MAX_MSG_SSL);
		recibir_datos_SSL(socket, buf);
        fprintf(stdout, "%s\n", buf);
	}

	cerrar_canal_SSL(socket);
	
	return 0;
}
