#include "../include/utilities.h"



void free_data( data * d){
	free(d->mensaje);
	UFreeUser(d->usuario);
	free(d);

}

