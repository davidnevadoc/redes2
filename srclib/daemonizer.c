#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#define EXIT_SUCCESS 0
#define EIXT_FAILURE -1

int daemonizar(char *service){
	pid_t daemonpid = -1;
	pid_t sid=-1;
	
	if(!service){
		perror("Invalid input\n");
		exit(EXIT_FAILURE);
	}
	daemonpid=fork();
	if(daemonpid<0){
		perror("Could not launch daemon\n");
		exit(EXIT_FAILURE);
	}
	/*Kill daddy :( */
	if(daemonpid==0){
		printf("My job is done. Good luck my son!\n");
		exit(EXIT_SUCCESS);
	}
	/*Set default permissions*/
	umask(0)
	/*New session*/
	sid = setsid();
	if(sid<0) {
		perror("Error in setsid()");
		exit(EXIT_FAILURE);
	}
	
	/*Change working directory*/
	chdir("/");

	/*Close stdin, stderr, stdout*/
	close(STDIN_FILENO);
	close(STDERR_FILENO);
	close(STDOUT_FILENO);

	/*Main function*/
	
	/*LOG_PID -> prints PID with every message
	  LOG_USER -> default value*/
	//openlog("Daemonlog",LOG_PID ,LOG_USER);
	//syslog(LOG_INFO,"Todo bien...");
	//closelog();

	/***************/
	
	
	exit(EXIT_SUCCESS);
	
}

