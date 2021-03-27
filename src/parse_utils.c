#include <stdlib.h>
#include <string.h>
#include "parse_utils.h"
using namespace std;

node* node_(int x, char * s, int token){
	node * tmp = new node;
	if(x)
		tmp->v = vector<node*>(x);
	else
		tmp->v = vector<node*>();
	tmp->token = token;
	tmp->sz = x;
	tmp->name = strdup(s);
	return tmp;
}

void add_node(node* par, node* add){
	par->v.push_back(add);
	par->sz++;
	return;
}

void push_front(node* par, node* add){
	par->v.insert(par->v.begin(), 1, add);
	par->sz++;
	return;
}