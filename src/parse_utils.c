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

void evaluate_const(node* node1, node* node2, int op, string type){
	int flag1 = 0, flag2 = 0;
	if(arithmetic_type_upgrade(node1->node_data,"long int","evaluate_const").find("int") == string::npos && arithmetic_type_upgrade(node1->node_data,"long int","evaluate_const").find("char") == string::npos){
		flag1 =1;
	}
	if(arithmetic_type_upgrade(node2->node_data,"long int","evaluate_const").find("int") == string::npos && arithmetic_type_upgrade(node2->node_data,"long int","evaluate_const").find("char") == string::npos){
		flag2 =1;
	}
	long long tmp1,tmp2;
	long double tmpp1, tmpp2;
	long long ans1;
	long double ans2;
	if(flag1){
		switch(node1->val_dt){
			case IS_FLOAT: tmpp1 = node1->val.float_const;break;
			case IS_DOUBLE: tmpp1 = node1->val.double_const;break;
			case IS_LONG_DOUBLE: tmpp1 = node1->val.long_double_const;break;
		}
	}
	else{
		switch(node1->val_dt){
			case IS_CHAR: tmp1 = node1->val.char_const; break;
			case IS_INT: tmp1 = node1->val.int_const; break;
			case IS_LONG: tmp1 = node1->val.long_const; break;
			case IS_SHORT: tmp1 = node1->val.short_const; break;
			case IS_U_INT: tmp1 = node1->val.u_int_const; break;
			case IS_U_LONG: tmp1 = node1->val.u_long_const; break;
			case IS_U_SHORT: tmp1 = node1->val.u_short_const; break;
		}
	}
	if(flag2){
		switch(node2->val_dt){
			case IS_FLOAT: tmpp2 = node2->val.float_const;break;
			case IS_DOUBLE: tmpp2 = node2->val.double_const;break;
			case IS_LONG_DOUBLE: tmpp2 = node2->val.long_double_const;break;
		}
	}
	else{
		switch(node2->val_dt){
			case IS_CHAR: tmp2 = node2->val.char_const; break;
			case IS_INT: tmp2 = node2->val.int_const; break;
			case IS_LONG: tmp2 = node2->val.long_const; break;
			case IS_SHORT: tmp2 = node2->val.short_const; break;
			case IS_U_INT: tmp2 = node2->val.u_int_const; break;
			case IS_U_LONG: tmp2 = node2->val.u_long_const; break;
			case IS_U_SHORT: tmp2 = node2->val.u_short_const; break;
		}
	}
	// printf("%c\n", op);
	switch(op){
		case '^': ans1 = (tmp1^tmp2); goto s00;
		case '|': ans1 = (tmp1|tmp2); goto s00;
		case '&': ans1 = (tmp1&tmp2); goto s00;
		case RIGHT_OP: ans1 = (tmp1 >> tmp2); goto s00;
		case LEFT_OP: ans1 = (tmp1 << tmp2); goto s00; // printf("%Ld << %Ld = %Ld\n", tmp1, tmp2, ans1); 
		case '%': if(tmp2 == 0){
				printf("\e[1;31mError [line %d]:\e[0m Remainder by 0 not allowed.\n",line);
				exit(-1);
			}
			else{
				ans1 = tmp1 % tmp2; goto s00;
			}
		case '+': if(flag1 || flag2){
				ans2 = (flag1 ? tmpp1 : tmp1) + (flag2 ? tmpp2 : tmp2); goto s11;
			}
			else{
				ans1 = tmp1 + tmp2; goto s00;
			}
		case '-':if(flag1 || flag2){
				ans2 = (flag1 ? tmpp1 : tmp1) - (flag2 ? tmpp2 : tmp2); goto s11;
			}
			else{
				ans1 = tmp1 - tmp2; goto s00;
			}
		case '/': if(!(flag2 ? tmpp2 : tmp2)){
				printf("\e[1;31mError [line %d]:\e[0m Division by 0 not allowed.\n",line);
				exit(-1);
			}
			if(flag1 || flag2){
				ans2 = (flag1 ? tmpp1 : tmp1) / (flag2 ? tmpp2 : tmp2); goto s11;
			}
			else{
				ans1 = tmp1 / tmp2; goto s00;
			}
		case '*':if(flag1 || flag2){
				ans2 = (flag1 ? tmpp1 : tmp1) * (flag2 ? tmpp2 : tmp2); goto s11;
			}
			else{
				ans1 = tmp1 * tmp2; goto s00;
			}
		case '>': ans1 = (flag1 ? tmpp1 : tmp1) > (flag2 ? tmpp2 : tmp2); goto s00;
		case '<': ans1 = (flag1 ? tmpp1 : tmp1) < (flag2 ? tmpp2 : tmp2); goto s00;
		case LE_OP: ans1 = (flag1 ? tmpp1 : tmp1) <= (flag2 ? tmpp2 : tmp2); goto s00;
		case GE_OP: ans1 = (flag1 ? tmpp1 : tmp1) >= (flag2 ? tmpp2 : tmp2); goto s00;
		case EQ_OP: ans1 = (flag1 ? tmpp1 : tmp1) == (flag2 ? tmpp2 : tmp2); goto s00;
		case NE_OP: ans1 = (flag1 ? tmpp1 : tmp1) != (flag2 ? tmpp2 : tmp2); goto s00;
		case AND_OP: ans1 = (flag1 ? tmpp1 : tmp1) && (flag2 ? tmpp2 : tmp2); goto s00;
		case OR_OP: ans1 = (flag1 ? tmpp1 : tmp1) || (flag2 ? tmpp2 : tmp2); goto s00;
	}
	s00 :;
	delete node2;
	if(type == "char"){
		node1->val_dt = IS_CHAR;
		node1->val.char_const = ans1;
		node1->node_data = "char";
	}
	if(type == "int"){
		node1->val_dt = IS_INT;
		node1->val.int_const = ans1;
		node1->node_data = "int";
	}
	if(type == "unsigned int"){
		node1->val_dt = IS_U_INT;
		node1->val.u_int_const = ans1;
		node1->node_data = "unsigned int";
	}
	if(type == "short int"){
		node1->val_dt = IS_SHORT;
		node1->val.short_const = ans1;
		node1->node_data = "short int";
	}
	if(type == "unsigned short int"){
		node1->val_dt = IS_U_SHORT;
		node1->val.u_short_const = ans1;
		node1->node_data = "unsigned short int";
	}
	if(type == "long int"){
		node1->val_dt = IS_LONG;
		node1->val.long_const = ans1;
		node1->node_data = "long int";
	}
	if(type == "unsigned long int"){
		node1->val_dt = IS_U_LONG;
		node1->val.u_long_const = ans1;
		node1->node_data = "unsigned long int";
	}
	return;
	s11:;
	delete node2;
	if(type == "float"){
		node1->val_dt = IS_FLOAT;
		node1->val.float_const = ans2;
		node1->node_data = "float";
	}
	if(type == "double"){
		node1->val_dt = IS_DOUBLE;
		node1->val.double_const = ans2;
		node1->node_data = "double";
	}
	if(type == "long double"){
		node1->val_dt = IS_LONG_DOUBLE;
		node1->val.long_double_const = ans2;
		node1->node_data = "long double";
	}
	return;
}