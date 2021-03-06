#include <stdlib.h>
#include <string.h>
#include "parse_utils.h"

node* node_(int x, char * s, int token){
	node * tmp = (node*)malloc(sizeof(node));
	if(x)
		tmp->v = (node**)malloc(x*sizeof(node*));
	else
		tmp->v = NULL;
	tmp->token = token;
	tmp->sz = x;
	tmp->name = (char*)malloc((strlen(s)+20)*sizeof(char));
	strcpy(tmp->name,s);
	return tmp;
}

void add_node(node* par, node* add){
	par->v = (node**)realloc((void*)par->v,(par->sz+1)*sizeof(node*));
	par->v[par->sz++] = add;
}