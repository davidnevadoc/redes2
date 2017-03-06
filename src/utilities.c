#include "../include/utilities.h"


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
	if ( (d->usuario = user_init(connfd)) ==NULL){
		free(d);
		return NULL;
	}
	d->mensaje =NULL;
	return d;


}

void free_data( data * d){
	//free(d->mensaje);
	UFreeUser(d->usuario);
	free(d);

}

