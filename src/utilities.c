#include "../includes/utilities.h"


User * user_init(int connfd){
	User * user = NULL;
	if( (user = (User *) malloc(sizeof(User))) ==NULL ) return NULL;
	user->user=NULL;
	user->away=NULL;
	user->host=NULL;
	user->IP=NULL;
	user->Next=NULL;
	user->nick=NULL;
	user->password=NULL;
	user->real=NULL;
	user->socket=connfd;
	user->userId =NOTCREATED;
	return user;
}

data * data_init(int connfd){
	data * d =NULL;
	if ( (d = malloc(sizeof(data))) == NULL) return NULL;
	d->socket=connfd;
	d->IP=NULL;
	d->mensaje =NULL;
	d->stop=0;
	return d;


}

void free_data( data * d){
	free(d->mensaje); 
	free(d->IP);
	//UFreeUser(d->usuario);
	free(d);

}


