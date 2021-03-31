#include <stdlib.h>
#include <string.h>
#include "parse_utils.h"
#include "y.tab.h"
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
/*switch(parsed.second){
	case IS_INT:
		$$->node_data = "int";
	break;
	case IS_LONG:
		$$->node_data = "long int";
	break;
	case IS_U_INT:
		$$->node_data = "unsigned int";
	break;
	case IS_U_LONG:
		$$->node_data = "unsigned long int";
	break;																		case IS_FLOAT:
		$$->node_data = "float";
	break;
	case IS_DOUBLE:
		$$->node_data = "double";
	break;
	case IS_LONG_DOUBLE:
		$$->node_data = "long double";
	break;
}*/