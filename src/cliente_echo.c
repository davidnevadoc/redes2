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
	char buf[100];
	puts("Inicializando nivel SSL...");
    inicializar_nivel_SSL();
    puts("Fijando contexto SSL...");
	if(fijar_contexto_SSL("certs/client/clientkey.pem" , "certs/cliente.pem") != SSL_OK){
		fprintf(stderr, "Error fijando contexto\n");
		exit(1);
	}
	puts("Abriendo conexión TCP...");
	/*Abro conexion TCP*/
	tcp_listen(5, &socket);

	if(conectar_canal_seguro_SSL(socket)!=SSL_OK){
		fprintf(stderr, "error al aceptar canal\n");
		exit(1);
	}

	if(evaluar_post_connectar_SSL(socket)) {
        fprintf(stderr, "Error del certificador\n");
        exit(1);
    }

	puts("Cliente echo...");
	//while(1){ TODO de momento solo una vez pa probar
	enviar_datos_SSL(socket, "Probando cliente echo");
	recibir_datos_SSL(socket, buf, sizeof(buf));
	fprintf(stderr, "--> %s\n", buf);

	cerrar_canal_SSL(socket);
	
	return 0;
}
