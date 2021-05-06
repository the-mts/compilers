%{
	#include<string.h>
	#include<stdlib.h>
	#include<numeric>
	#include<iostream>
	#include<stdlib.h>
	#include "parse_utils.h"
	#include "3AC.h"
	using namespace std;
	struct node* root;
	void yyerror(const char*s);
	extern int yylex();
	extern int yyparse();
	extern int line;
	int unnamed_var = 0;
	string data_type = "";
	string params = "";
	vector<pair<pair<string, string>, tt_entry*	>> func_params;
	int break_level=0;
	int continue_level=0;
	string next_name;
	string func_ret_type = "";
	int temp_var;
%}
%union {
	struct node* nodes;
	char * id;
	int instr;
}

%token <id> IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token <id> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token <id> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token <id> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token <id> XOR_ASSIGN OR_ASSIGN TYPE_NAME
%token <id> TYPEDEF EXTERN STATIC AUTO REGISTER
%token <id> CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token <id> STRUCT UNION ENUM ELLIPSIS
%token <id> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN
%token <id> ',' '^' '|' ';' '{' '}' '[' ']' '(' ')' '+' '-' '%' '/' '*' '.' '>' '<' '&' '=' '!' '~' ':' '?'

%type <id> unary_operator storage_class_specifier struct_or_union 
%type <nodes> assignment_operator primary_expression postfix_expression argument_expression_list unary_expression type_specifier 
%type <nodes> cast_expression multiplicative_expression additive_expression shift_expression relational_expression 
%type <nodes> equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression 
%type <nodes> logical_or_expression conditional_expression assignment_expression expression 
%type <nodes> constant_expression declaration declaration_specifiers init_declarator_list init_declarator 
%type <nodes> struct_or_union_specifier struct_declaration_list pointer	
%type <nodes> struct_declaration specifier_qualifier_list struct_declarator_list struct_declarator enum_specifier 
%type <nodes> enumerator_list enumerator type_qualifier declarator direct_declarator 
%type <nodes> type_qualifier_list parameter_type_list parameter_list parameter_declaration 
%type <nodes> identifier_list type_name abstract_declarator direct_abstract_declarator initializer 
%type <nodes> initializer_list statement labeled_statement compound_statement declaration_list 
%type <nodes> statement_list expression_statement selection_statement iteration_statement jump_statement 
%type <nodes> translation_unit external_declaration function_definition augment_state

%type<id> M6 M8
%type <nodes> M1
%type <instr> M7 M9 M11

%start augment_state
%define parse.error verbose
%%

primary_expression
	: IDENTIFIER 												{
																	$$ = node_(0,$1,IDENTIFIER);
																	st_entry * entry = lookup(string((const char *)$1));
																	if(entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Undeclared identifier '%s'.\n",line, $1);
																		exit(-1);
																	}
																	$$->node_name = $1;
																	$$->node_data = entry->type;
																	if(entry->type.back() == ']' && entry->type.find("[]")==string::npos){
																		$$->value_type = RVALUE;
																		$$->place = getNewTemp(entry->type, entry->ttentry);
																		emit("UNARY&", {string((const char*)$1), entry}, {"", NULL}, $$->place);
																	}
																	else {
																		$$->value_type = LVALUE;
																		$$->place = { string((const char*)$1), entry };
																	}
																	$$->ttentry = entry->ttentry;

																	//////////////// 3AC ////////////////
																	/////////////////////////////////////
																}

	| CONSTANT 													{
																	$$ = node_(0,$1,CONSTANT);
																	pair<constant, enum const_type> parsed = parse_constant(string((const char*)$1));
																	switch(parsed.second){
																		case IS_INT:
																			$$->node_data = "int";
																			$$->val_dt = IS_INT;
																		break;
																		case IS_LONG:
																			$$->node_data = "long int";
																			$$->val_dt = IS_LONG;
																		break;
																		case IS_U_INT:
																			$$->node_data = "unsigned int";
																			$$->val_dt = IS_U_INT;
																		break;
																		case IS_U_LONG:
																			$$->node_data = "unsigned long int";
																			$$->val_dt = IS_U_LONG;
																		break;																		
																		case IS_FLOAT:
																			$$->node_data = "float";
																			$$->val_dt = IS_FLOAT;
																		break;
																		case IS_DOUBLE:
																			$$->node_data = "double";
																			$$->val_dt = IS_DOUBLE;
																		break;
																		case IS_LONG_DOUBLE:
																			$$->node_data = "long double";
																			$$->val_dt = IS_LONG_DOUBLE;
																		break;
																		case IS_CHAR:
																			$$->node_data = "char";
																			$$->val_dt = IS_CHAR;
																		break;
																	}
																	$$->val = parsed.first;
																	$$->value_type = RVALUE;
																	//////////////// 3AC ////////////////
																	
																	/////////////////////////////////////
																}

	| STRING_LITERAL											{
																	$$ = node_(0,$1,STRING_LITERAL); $$->node_data = "char ["+to_string(strlen($1)+1)+"]";
																	string tmp = gen_new_const_label();
																	constLabels[tmp] = {".string", ".string "+string((const char*) $1)};
																	$$->place = {tmp, NULL};
																}

	| '(' expression ')'										{
																	$$ = $2;
																	backpatch($$->nextlist, nextquad);
																}
	;

postfix_expression
	: primary_expression										{
																	$$ = $1;
																}
	| postfix_expression '[' expression ']'						{
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m Array subscript is not an integer.\n",line);
																		exit(-1);
																	}
																	$$ = node_(2, "+", -1);
																	$$->v[0] = $1; $$->v[1] = $3;
																	pair<string,int> p = get_equivalent_pointer($1->node_data);
																	if(p.second == 0){
																		printf("\e[1;31mError [line %d]:\e[0m Subscripted value is neither array nor pointer.\n",line);
																		exit(-1);
																	}
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(p2.second != 0){
																		printf("\e[1;31mError [line %d]:\e[0m Array subscript is not an integer.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($3->node_data) || ($3->node_data.find("int")==string::npos && $3->node_data.find("char")==string::npos)){
																		printf("\e[1;31mError [line %d]:\e[0m Array subscript is not an integer.\n",line);
																		exit(-1);
																	}
																	$$->node_data = reduce_pointer_level($1->node_data);
																	$$->ttentry = $1->ttentry;
																	if($$->node_data.back() == ']'){
																		$$->value_type = RVALUE;
																	}
																	else {
																		$$->value_type = LVALUE;
																	}

																	//////////////// 3AC ////////////////
																	if($3->token == CONSTANT){
																		$3->place = emitConstant($3);
																	}
																	$$->place = getNewTemp($$->node_data, $$->ttentry);
																	backpatch($3->nextlist, nextquad);
																	// int x = emit("[]", $1->place, $3->place, $$->place);
																	// backpatch($3->truelist, x);		// Have to check
																	// backpatch($3->falselist, x);	// check

																	if($$->node_data.back()==']'){
																		emit("+int", $1->place, $3->place, $$->place);
																	}
																	else{
																		auto tempnode = $$;
																		$$ = node_(1, "UNARY*", -1);
																		$$->v[0] = tempnode;
																		$$->node_data = tempnode->node_data;
																		if($1->node_data.back()=='*'){
																			tempnode->node_data = $1->node_data+'*';
																		}
																		else{
																			tempnode->node_data = $1->node_data+" *";
																		}
																		$$->value_type = LVALUE;
																		$$->ttentry = tempnode->ttentry;
																		$$->place = tempnode->place;
																		tempnode->place = getNewTemp(tempnode->node_data, tempnode->ttentry);

																		emit("+int", $1->place, $3->place, tempnode->place);
																		emit("UNARY*", tempnode->place, {"", NULL}, $$->place);
																	}
																	/////////////////////////////////////
																}
	| postfix_expression '(' ')'								{
																	$$ = node_(1, "()", -1);
																	$$->v[0] = $1;
																	if($1->token != IDENTIFIER){
																		printf("\e[1;31mError [line %d]:\e[0m Invalid function call.\n",line);
																		exit(-1);
																	}
																	st_entry * entry = lookup(string((const char *)$1->name));
																	if(entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Invalid function name '%s'.\n",line,$1->name);
																		exit(-1);
																	}
																	if(entry->type_name != IS_FUNC){
																		printf("\e[1;31mError [line %d]:\e[0m Called object '%s' is not a function.\n",line,$1->name);
																		exit(-1);
																	}
																	// if(entry->is_init != 1){
																	// 	printf("\e[1;31mError [line %d]:\e[0m Function '%s' declared but not defined.\n",line,$1->name);
																	// 	exit(-1);
																	// }
																	if((int)entry->arg_list->size() != 0){
																		printf("\e[1;31mError [line %d]:\e[0m Function '%s' needs %d arguments but 0 provided.\n",line,$1->name,(int)entry->arg_list->size());
																		exit(-1);
																	}
																	$$->node_data = entry->type;
																	$$->value_type = RVALUE;
																	$$->ttentry = entry->ttentry;

																	//////////////// 3AC ////////////////
																	$$->place = getNewTemp($$->node_data, $$->ttentry);
																	emit("CALL", $1->place, {"", NULL}, $$->place, 0);
																	/////////////////////////////////////
																}
	| postfix_expression '(' argument_expression_list ')'		{
																	$$ = node_(2, "(args)", -1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	if($1->token != IDENTIFIER){
																		printf("\e[1;31mError [line %d]:\e[0m Invalid function call.\n",line);
																		exit(-1);
																	}
																	st_entry * entry = lookup(string((const char *)$1->name));
																	if(entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Invalid function name '%s'.\n",line,$1->name);
																		exit(-1);
																	}

																	vector<qi> arg_names;
																	vector<pair<pair<string,string>,tt_entry*>> arg_list;
																	if(entry->type_name == IS_BUILTIN_FUNC){
																		// arg_names = $3->v;
																		for(auto i : $3->v) arg_names.push_back(i->place);
																		goto skip_arg_check;
																	}

																	if(entry->type_name != IS_FUNC){
																		printf("\e[1;31mError [line %d]:\e[0m Called object '%s' is not a function.\n",line,$1->name);
																		exit(-1);
																	}
																	// if(entry->is_init != 1){
																	// 	printf("\e[1;31mError [line %d]:\e[0m Function '%s' declared but not defined.\n",line,$1->name);
																	// 	exit(-1);
																	// }
																	if((int)entry->arg_list->size() != $3->sz){
																		printf("\e[1;31mError [line %d]:\e[0m Function '%s' needs %d arguments but %d provided.\n",line,$1->name,(int)entry->arg_list->size(),$3->sz);
																		exit(-1);
																	}
																	arg_list = *(entry->arg_list);
																	for(int i = 0; i < $3->sz; i++){
																		string type = $3->v[i]->node_data;

																		if(arg_list[i].first.first.back() == ']'){
																			if($3->v[i]->node_data.back() != ']'){
																				printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line,$1->name,i+1,arg_list[i].first.first.c_str(),type.c_str());
																				exit(-1);
																			}
																			type = increase_array_level(reduce_pointer_level($3->v[i]->node_data));
																			if(type != arg_list[i].first.first){
																				printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line,$1->name,i+1,arg_list[i].first.first.c_str(),type.c_str());
																				exit(-1);
																			}
																			arg_names.push_back($3->v[i]->place);
																		}
																		else if($3->v[i]->node_data.back() == ']'){
																			printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line,$1->name,i+1,arg_list[i].first.first.c_str(),type.c_str());
																			exit(-1);
																		}

																		else{
																			string tmp1 = $3->v[i]->node_data;
																			string tmp2 = arg_list[i].first.first;
																			string type1, type2;
																			pair<string, int> p1 = get_equivalent_pointer(tmp1);
																			pair<string, int> p2 = get_equivalent_pointer(tmp2);
																			type1 = p1.first, type2 = p2.first;


																			string type11=type1, type22=type2;
																			while(type11.back()=='*'){
																				type11 = reduce_pointer_level(type11);
																			}
																			while(type22.back()=='*'){
																				type22 = reduce_pointer_level(type22);
																			}
																			tt_entry* entry1 = $3->v[i]->ttentry;
																			tt_entry* entry2 = global_type_lookup(type22);
																			

																			if(entry1 && entry2){
																				if(entry1 != entry2){
																					printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line, $1->name, i+1, tmp2.c_str(), tmp1.c_str());
																					exit(-1);
																				}
																				if((p1.second && !p2.second) || (p2.second && !p1.second)){
																					printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line, $1->name, i+1, tmp2.c_str(), tmp1.c_str());
																					exit(-1);
																				}
																			}
																			else if(entry1 || entry2){
																				printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line, $1->name, i+1, tmp2.c_str(), tmp1.c_str());
																				exit(-1);
																			}
																			//if((tmp1.find("[") != string::npos && tmp1.find("[]") == string::npos)){
																			//	printf("\e[1;31mError [line %d]:\e[0m Array variables cannot be reassigned.\n",line);
																			//	exit(-1);
																			//}
																			if(p1.second && p2.second){
																				if(p1.first != p2.first){
																					printf("\e[1;35mWarning [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line, $1->name, i+1, tmp2.c_str(), tmp1.c_str());
																				}
																			}
																			else if(p1.second){
																				if(tmp2 == "float" || tmp2 == "double" || tmp2 == "long double"){
																					printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line, $1->name, i+1, tmp2.c_str(), tmp1.c_str());
																					exit(-1);
																				}
																				printf("\e[1;35mWarning [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line, $1->name, i+1, tmp2.c_str(), tmp1.c_str());
																			}
																			else if(p2.second){
																				if(tmp1 == "float" || tmp1 == "double" || tmp1 == "long double"){
																					printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line, $1->name, i+1, tmp2.c_str(), tmp1.c_str());
																					exit(-1);
																				}
																				printf("\e[1;35mWarning [line %d]:\e[0m For function '%s', argument %d should be of type '%s', '%s' provided.\n",line, $1->name, i+1, tmp2.c_str(), tmp1.c_str());
																			}

																			//////////////// 3AC ////////////////
																			if(tmp1 != tmp2){
																				string op2 = "("+tmp1 + "-to-" + tmp2+")"; // Modify
																				qi tmp = getNewTemp(tmp2, entry1);  
																				int x = emit(op2, $3->v[i]->place, {"", NULL}, tmp);
																				arg_names.push_back(tmp);
																			}
																			else{
																				arg_names.push_back($3->v[i]->place);
																			}
																			/////////////////////////////////////
																		}

																	}

																	skip_arg_check:
																	$$->node_data = entry->type;
																	$$->value_type = RVALUE;
																	$$->ttentry = entry->ttentry;

																	//////////////// 3AC ////////////////
																	for(auto i: arg_names){
																		emit("PARAM", i, {"", NULL}, {"", NULL});
																	}
																	$$->place = getNewTemp($$->node_data, $$->ttentry);
																	emit("CALL", $1->place, {"", NULL}, $$->place, $3->sz);
																	/////////////////////////////////////
																}
	| postfix_expression '.' IDENTIFIER							{
																	$$ = node_(2,".",'.');
																	$$->v[0] = $1;
																	$$->v[1] = node_(0,$3,IDENTIFIER);
																	tt_entry * type_entry = NULL;
																	if($1->node_data.back() != '*' && $1->node_data.back()!=']'){
																		type_entry = $1->ttentry;
																	}
																	if(type_entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m '.' operator applied on non-struct or non-union type.\n",line);
																		exit(-1);
																	}
																	
																	int flag = 0;
																	string name = string((const char*)$3);
																	string type;
																	tt_entry* entry = NULL;
																	for(auto it : *(type_entry->mem_list))
																		if(it.first.second == name){
																			type = it.first.first;
																			flag = 1;
																			entry = it.second;
																			break;
																		}
																	if(!flag){
																		printf("\e[1;31mError [line %d]:\e[0m '%s' has no member named '%s'.\n",line, $1->node_data.c_str(),$3);/*typedef changes*/
																		exit(-1);
																	}
																	$$->node_data = type;
																	if($1->value_type == RVALUE)
																		$$->value_type = RVALUE;
																	else
																		$$->value_type = LVALUE;
																	$$->ttentry = entry;

																	//////////////// 3AC ////////////////
																	
																	/////////////////////////////////////
																}
	| postfix_expression PTR_OP IDENTIFIER 						{
																	$$ = node_(2,"->",PTR_OP);
																	$$->v[0] = $1;
																	$$->v[1] = node_(0,$3,IDENTIFIER);
																	string type = $1->node_data;
																	pair<string,int> p = get_equivalent_pointer(type);
																	int p_level = p.second;
																	if(p_level != 1){
																		printf("\e[1;31mError [line %d]:\e[0m '->' operator applied on non-pointer or multilevel-pointer type.\n",line);
																		exit(-1);
																	}
																	type = p.first.substr(0,type.length() - 2);
																	tt_entry * type_entry = $1->ttentry;
																	if(type_entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m '->' operator applied on non-struct or non-union pointer type.\n",line);
																		exit(-1);
																	}
																	int flag = 0;
																	tt_entry* tmp_entry = NULL;
																	string type1 = type;
																	string name = string((const char*)$3);
																	for(auto it : *(type_entry->mem_list))
																		if(it.first.second == name){
																			type = it.first.first;
																			flag = 1;
																			tmp_entry = it.second;
																			break;
																		}
																	if(!flag){
																		printf("\e[1;31mError [line %d]:\e[0m '%s' has no member named '%s'.\n",line, type1.c_str(),$3);/*typedef changes*/
																		exit(-1);
																	}
																	$$->node_data = type;
																	if($1->value_type == RVALUE)
																		$$->value_type = RVALUE;
																	else
																		$$->value_type = LVALUE;
																	$$->ttentry = tmp_entry;
																	
																	//////////////// 3AC ////////////////

																	/////////////////////////////////////
																}
	| postfix_expression INC_OP									{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	$$ = node_(1, "exp++", -1);
																	$$->v[0] = $1;
																	$$->node_data = $1->node_data;
																	tt_entry* type_entry = NULL;
																	if($1->node_data.back()!='*' && $1->node_data.back()!=']'){
																		type_entry = $1->ttentry;
																	}
																	if(type_entry != NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Increment operator cannot be applied on non-integer, non-floating point and non pointer types.\n",line);
																		exit(-1);
																	}/*array*/
																	if($1->value_type == RVALUE){
																		printf("\e[1;31mError [line %d]:\e[0m Increment operator cannot be applied on rvalue.\n",line);
																		exit(-1);
																	}
																	$$->value_type = RVALUE;
																	$$->ttentry = $1->ttentry;

																	//////////////// 3AC ////////////////
																	$$->place = getNewTemp($$->node_data, $$->ttentry);
																	// int x = emit("x++", $1->place, {"", NULL}, $$->place);

																	string type = $1->node_data;
																	if(!strcmp($1->name, "UNARY*")){
																		emit("=", $1->place, {"", NULL}, $$->place);
																		auto consttmp = getNewTemp("char");
																		emit("=", {"$1", NULL}, {"", NULL}, consttmp);

																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.back()=='*'){
																			auto tmp = getNewTemp($1->node_data, $1->ttentry);
																			int x = emit("+int", $1->place, consttmp, tmp);
																			emit("ADDR=", tmp, {"", NULL}, $1->v[0]->place);
																		}
																		else{
																			auto tmp = getNewTemp(type, type_entry);
																			auto tmp2 = getNewTemp(type, type_entry);
																			int x = emit("inttoreal", consttmp, {"", NULL}, tmp);
																			emit("+real", $1->place, tmp, tmp2);
																			emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																		}
																	}
																	else{
																		int x = emit("x++", $1->place, {"", NULL}, $$->place);
																	}
																	/////////////////////////////////////
																}
	| postfix_expression DEC_OP									{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	$$ = node_(1, "exp--", -1); $$->v[0] = $1;
																	$$->node_data = $1->node_data;
																	tt_entry* type_entry = NULL;
																	if($1->node_data.back()!='*' && $1->node_data.back()!=']'){
																		type_entry = $1->ttentry;
																	}
																	if(type_entry != NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Decrement operator cannot be applied on non-integer, non-floating point and non pointer types.\n",line);
																		exit(-1);
																	}/*array*/
																	if($1->value_type == RVALUE){
																		printf("\e[1;31mError [line %d]:\e[0m Decrement operator cannot be applied on rvalue.\n",line);
																		exit(-1);
																	}
																	$$->value_type = RVALUE;
																	$$->ttentry = $1->ttentry;

																	//////////////// 3AC ////////////////
																	$$->place = getNewTemp($$->node_data, $$->ttentry);
																	// int x = emit("x--", $1->place, {"", NULL}, $$->place);

																	
																	string type = $1->node_data;
																	if(!strcmp($1->name, "UNARY*")){
																		emit("=", $1->place, {"", NULL}, $$->place);
																		auto consttmp = getNewTemp("char");
																		emit("=", {"$1", NULL}, {"", NULL}, consttmp);
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.back() == '*'){
																			auto tmp = getNewTemp($1->node_data, $1->ttentry);
																			int x = emit("-int", $1->place, consttmp, tmp);
																			emit("ADDR=", tmp, {"", NULL}, $1->v[0]->place);
																		}
																		else{
																			auto tmp = getNewTemp(type, type_entry);
																			auto tmp2 = getNewTemp(type, type_entry);
																			int x = emit("inttoreal", consttmp, {"", NULL}, tmp);
																			emit("-real", $1->place, tmp, tmp2);
																			emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																		}
																	}
																	else{
																		int x = emit("x--", $1->place, {"", NULL}, $$->place);
																	}
																	/////////////////////////////////////
																}
	;

argument_expression_list
	: assignment_expression										{
																	$$ = node_(1,"arg_list",-1); $$->v[0] = $1;

																	//////////////// 3AC ////////////////
																	if($1->token == CONSTANT){
																		$1->place = emitConstant($1);
																	}
																	/////////////////////////////////////
																}
	| argument_expression_list ',' assignment_expression		{
																	$$ = $1; add_node($$,$3);

																	//////////////// 3AC ////////////////
																	if($3->token == CONSTANT){
																		$3->place = emitConstant($3);
																	}
																	/////////////////////////////////////
																}
	;

unary_expression
	: postfix_expression										{$$ = $1;}
	| INC_OP unary_expression									{
																	if($2->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	$$ = node_(1,"++exp",-1);
																	$$->v[0] = $2;
																	$$->node_data = $2->node_data;
																	tt_entry* type_entry = NULL;
																	if($2->node_data.back()!='*' && $2->node_data.back()!=']'){
																		type_entry = $2->ttentry;
																	}
																	if(type_entry != NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Increment operator cannot be applied on non-integer, non-floating point and non pointer types.\n",line);
																		exit(-1);
																	}/*array*/
																	if($2->value_type == RVALUE){
																		printf("\e[1;31mError [line %d]:\e[0m Increment operator cannot be applied on rvalue.\n",line);
																		exit(-1);
																	}
																	$$->value_type = RVALUE;
																	$$->ttentry = $2->ttentry;

																	//////////////// 3AC ////////////////
																	$$->place = getNewTemp($$->node_data, $$->ttentry);
																	// int x = emit("++x", $2->place, {"", NULL}, $$->place);
																	
																	string type = $2->node_data;
																	if(!strcmp($2->name, "UNARY*")){
																		auto consttmp = getNewTemp("char");
																		emit("=", {"$1", NULL}, {"", NULL}, consttmp);
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.back()=='*'){
																			auto tmp = getNewTemp($2->node_data, $2->ttentry);
																			int x = emit("+int", $2->place, consttmp, tmp);
																			emit("ADDR=", tmp, {"", NULL}, $2->v[0]->place);
																			emit("=", tmp, {"", NULL}, $$->place);
																		}
																		else{
																			auto tmp = getNewTemp(type, type_entry);
																			auto tmp2 = getNewTemp(type, type_entry);
																			int x = emit("inttoreal", consttmp, {"", NULL}, tmp);
																			emit("+real", $2->place, tmp, tmp2);
																			emit("ADDR=", tmp2, {"", NULL}, $2->v[0]->place);
																			emit("=", tmp2, {"", NULL}, $$->place);
																		}
																	}
																	else{
																		int x = emit("++x", $2->place, {"", NULL}, $$->place);
																	}
																	/////////////////////////////////////
																}
	| DEC_OP unary_expression									{
																	if($2->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	$$ = node_(1,"++exp",-1);
																	$$->v[0] = $2;
																	$$->node_data = $2->node_data;
																	tt_entry* type_entry = NULL;
																	if($2->node_data.back()!='*' && $2->node_data.back()!=']'){
																		type_entry = $2->ttentry;
																	}
																	if(type_entry != NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Decrement operator cannot be applied on non-integer, non-floating point and non pointer types.\n",line);
																		exit(-1);
																	}/*array*/
																	if($2->value_type == RVALUE){
																		printf("\e[1;31mError [line %d]:\e[0m Decrement operator cannot be applied on rvalue.\n",line);
																		exit(-1);
																	}
																	$$->value_type = RVALUE;
																	$$->ttentry = $2->ttentry;

																	//////////////// 3AC ////////////////
																	$$->place = getNewTemp($$->node_data, $$->ttentry);
																	// int x = emit("--x", $2->place, {"", NULL}, $$->place);

																	string type = $2->node_data;
																	if(!strcmp($2->name, "UNARY*")){
																		auto consttmp = getNewTemp("char");
																		emit("=", {"$1", NULL}, {"", NULL}, consttmp);
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.back()=='*'){
																			auto tmp = getNewTemp($2->node_data, $2->ttentry);
																			int x = emit("-int", $2->place, consttmp, tmp);
																			emit("ADDR=", tmp, {"", NULL}, $2->v[0]->place);
																			emit("=", tmp, {"", NULL}, $$->place);
																		}
																		else{
																			auto tmp = getNewTemp(type, type_entry);
																			auto tmp2 = getNewTemp(type, type_entry);
																			int x = emit("inttoreal", consttmp, {"", NULL}, tmp);
																			emit("-real", $2->place, tmp, tmp2);
																			emit("ADDR=", tmp2, {"", NULL}, $2->v[0]->place);
																			emit("=", tmp2, {"", NULL}, $$->place);
																		}
																	}
																	else{
																		int x = emit("--x", $2->place, {"", NULL}, $$->place);
																	}
																	/////////////////////////////////////
																}
	| unary_operator cast_expression							{
																	if($2->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	string ch = "UNARY";
																	ch += *($1);
																	$$ = node_(1,(char*)ch.c_str(),*($1));
																	$$->v[0] = $2;
																	string temp_data = get_equivalent_pointer($2->node_data).first;
																	switch(*($1)){
																		case '&':{
																			if($2->value_type == LVALUE){
																				$$->value_type = RVALUE;
																				if($2->node_data.back() == '*'){
																					$$->node_data = $2->node_data+'*';
																				}
																				else if($2->node_data.back() == ']'){
																					$$->node_data = increase_array_level($2->node_data);
																				}
																				else{
																					$$->node_data = $2->node_data + " *";
																				}
																				$$->ttentry = $2->ttentry;
																			}
																			else{
																				printf("\e[1;31mError [line %d]:\e[0m Address-of operator cannot be applied on rvalue.\n",line);
																				exit(-1);
																			}

																			//////////////// 3AC ////////////////
																			if(!strcmp($2->name, "UNARY*")){
																				$$->place = $2->v[0]->place;
																			}
																			else{
																				$$->place = getNewTemp($$->node_data, $$->ttentry);
																				emit("UNARY" + string((const char*)$1), $2->place, {"", NULL}, $$->place);
																			}
																			/////////////////////////////////////																			
																		}
																		break;
																		case '*':{
																			if(temp_data.back() != '*'){
																				printf("\e[1;31mError [line %d]:\e[0m Indirection operator cannot be applied on non-pointer type.\n",line);
																				exit(-1);
																			}
																			else{
																				$$->node_data = reduce_pointer_level($2->node_data);
																				$$->value_type = LVALUE;
																				$$->ttentry = $2->ttentry;


																				//////////////// 3AC ////////////////
																				$$->place = getNewTemp($$->node_data, $$->ttentry);
																				int x = emit("UNARY"+string((const char*) $1), $2->place, {"", NULL}, $$->place);
																				/////////////////////////////////////
																			}
																		}
																		break;
																		case '!':{
																			if($2->token==CONSTANT){
																				$$ = $2;
																				switch($2->val_dt){
																					case IS_INT: $$->val.int_const = !$2->val.int_const; break;
																					case IS_LONG: $$->val.int_const = !$2->val.long_const; break;
																					case IS_SHORT: $$->val.int_const = !$2->val.short_const; break;
																					case IS_U_INT: $$->val.int_const = !$2->val.u_int_const; break;
																					case IS_U_LONG: $$->val.int_const = !$2->val.u_long_const; break;
																					case IS_U_SHORT: $$->val.int_const = !$2->val.u_short_const; break;
																					case IS_FLOAT: $$->val.int_const = !$2->val.float_const; break;
																					case IS_DOUBLE: $$->val.int_const = !$2->val.double_const; break;
																					case IS_LONG_DOUBLE: $$->val.int_const = !$2->val.long_double_const; break;
																					case IS_CHAR: $$->val.int_const = !$2->val.char_const; break;
																					default: printf("\e[1;31mError [line %d]:\e[0m Invalid argument to unary operator '%s'.\n",line, $1); exit(-1);
																				}
																				$$->val_dt = IS_INT;
																				break;
																			}
																			if(temp_data.back() == '*'){
																				$$->value_type = RVALUE;
																				$$->node_data = "int";
																				$$->ttentry = NULL;

																				//////////////// 3AC ////////////////
																				$$->place = getNewTemp($$->node_data, $$->ttentry);
																				int x = emit("UNARY"+string((const char*) $1), $2->place, {"", NULL}, $$->place);
																				/////////////////////////////////////

																				break;
																			}
																		}
																		case '-':
																		case '+':{
																			if($2->token==CONSTANT && *($1)=='+'){
																				switch($2->val_dt){
																					case IS_INT:$2->val.int_const = $2->val.int_const; break;
																					case IS_LONG:$2->val.long_const = $2->val.long_const; break;
																					case IS_SHORT:$2->val.short_const = $2->val.short_const; break;
																					case IS_U_INT:$2->val.u_int_const = $2->val.u_int_const; break;
																					case IS_U_LONG:$2->val.u_long_const = $2->val.u_long_const; break;
																					case IS_U_SHORT:$2->val.u_short_const = $2->val.u_short_const; break;
																					case IS_FLOAT: $2->val.float_const = $2->val.float_const; break;
																					case IS_DOUBLE: $2->val.double_const = $2->val.double_const; break;
																					case IS_LONG_DOUBLE: $2->val.long_double_const = $2->val.long_double_const; break;
																					case IS_CHAR: $2->val.char_const = $2->val.char_const; break;
																					default: printf("\e[1;31mError [line %d]:\e[0m Invalid argument to unary operator '%s'.\n",line, $1); exit(-1);
																				}
																				$$ = $2;
																				break;
																			}
																			else if($2->token==CONSTANT && *($1)=='-'){
																				switch($2->val_dt){
																					case IS_INT: $2->val.int_const = -$2->val.int_const; break;
																					case IS_LONG: $2->val.long_const = -$2->val.long_const; break;
																					case IS_SHORT: $2->val.short_const = -$2->val.short_const; break;
																					case IS_U_INT: $2->val.u_int_const = -$2->val.u_int_const; break;
																					case IS_U_LONG: $2->val.u_long_const = -$2->val.u_long_const; break;
																					case IS_U_SHORT: $2->val.u_short_const = -$2->val.u_short_const; break;
																					case IS_FLOAT: $2->val.float_const = -$2->val.float_const; break;
																					case IS_DOUBLE: $2->val.double_const = -$2->val.double_const; break;
																					case IS_LONG_DOUBLE: $2->val.long_double_const = -$2->val.long_double_const; break;
																					case IS_CHAR: $2->val.char_const = -$2->val.char_const; break;
																					default: printf("\e[1;31mError [line %d]:\e[0m Invalid argument to unary operator '%s'.\n",line, $1); exit(-1);
																				}
																				$$ = $2;
																				break;
																			}
																			else{
																				if(temp_data.back()=='*'){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible type for %c operator.\n",line,*($1));
																					exit(-1);
																				}
																				if(is_struct_or_union(temp_data) || (temp_data.find("int") == string::npos && temp_data.find("double") == string::npos && temp_data.find("float") == string::npos && temp_data.find("char") == string::npos)){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible type for %c operator.\n",line,*($1));
																					exit(-1);
																				}
																				$$->node_data = temp_data;
																				$$->value_type = RVALUE;

																				//////////////// 3AC ////////////////
																				$$->place = getNewTemp($$->node_data, $$->ttentry);
																				int x = emit("UNARY"+string((const char*) $1), $2->place, {"", NULL}, $$->place);
																				/////////////////////////////////////
																			}
																		}
																		break;
																		case '~':{
																			if($2->token==CONSTANT){
																				switch($2->val_dt){
																					case IS_INT: $2->val.int_const = ~$2->val.int_const; break;
																					case IS_LONG: $2->val.long_const = ~$2->val.long_const; break;
																					case IS_SHORT: $2->val.short_const = ~$2->val.short_const; break;
																					case IS_U_INT: $2->val.u_int_const = ~$2->val.u_int_const; break;
																					case IS_U_LONG: $2->val.u_long_const = ~$2->val.u_long_const; break;
																					case IS_U_SHORT: $2->val.u_short_const = ~$2->val.u_short_const; break;
																					case IS_FLOAT: 
																					case IS_DOUBLE: 
																					case IS_LONG_DOUBLE: printf("\e[1;31mError [line %d]:\e[0m ~ cannot be applied to non-integer types.\n",line);
																											exit(-1); break;
																					case IS_CHAR: $2->val.char_const = ~$2->val.char_const; break;
																					default: printf("\e[1;31mError [line %d]:\e[0m Invalid argument to unary operator '%s'.\n",line, $1); exit(-1);
																				}
																				$$ = $2;
																				break;
																			}
																			else{
																				if(is_struct_or_union(temp_data) || temp_data.substr((int)temp_data.size() - 3,3) != "int"){
																					printf("\e[1;31mError [line %d]:\e[0m ~ cannot be applied to non-integer types.\n",line);
																					exit(-1);
																				}
																				$$->node_data = temp_data;
																				$$->value_type = RVALUE;

																				//////////////// 3AC ////////////////
																				$$->place = getNewTemp($$->node_data, $$->ttentry);
																				int x = emit("UNARY"+string((const char*) $1), $2->place, {"", NULL}, $$->place);
																				/////////////////////////////////////

																			}
																		}
																	}
																}
	| SIZEOF unary_expression									{
																	long sz = get_size($2->node_data);
																	$$ = node_(0,(char*)to_string(sz).c_str(),CONSTANT);
																	$$->node_data = "long int";
																	$$->val_dt = IS_LONG;
																	$$->val.long_const = sz;
																}
	| SIZEOF '(' type_name ')'									{
																	long sz = get_size($3->node_data, $3->ttentry);
																	$$ = node_(0,(char*)to_string(sz).c_str(),CONSTANT);
																	$$->node_data = "long int";
																	$$->val_dt = IS_LONG;
																	$$->val.long_const = sz;
																}
	;

unary_operator
	: '&'														{$$ = $1;}
	| '*'														{$$ = $1;}
	| '+'														{$$ = $1;}
	| '-'														{$$ = $1;}
	| '~'														{$$ = $1;}
	| '!'														{$$ = $1;}
	;

cast_expression
	: unary_expression											{$$ = $1;}
	| '(' type_name ')' cast_expression							{
																	$$ = node_(2,"()_typecast",-1);
																	$$->v[0] = $2;
																	$$->v[1] = $4;
																	pair<string,int> p1 = get_equivalent_pointer($2->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($4->node_data);
																	//tt_entry * entry1 = type_lookup(p1.first);
																	//tt_entry * entry2 = type_lookup(p2.first);
																	tt_entry * entry1 = NULL;
																	tt_entry * entry2 = NULL;
																	if(p1.second == 0){
																		entry1 = $2->ttentry;
																	}
																	if(p2.second == 0){
																		entry2 = $4->ttentry;
																	}
																	if(entry1 || entry2){
																		printf("\e[1;31mError [line %d]:\e[0m Typecasts forbidden for non-scalar types.\n",line);
																		exit(-1);
																	}
																	if($2->node_data.find("[") != string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Cast specifies array type.\n",line);
																		exit(-1);
																	}
																	if(p1.second && p2.second){
																	}
																	else if(p1.second){
																		if($4->node_data == "float" || $4->node_data == "double" || $4->node_data == "long double"){
																			printf("\e[1;31mError [line %d]:\e[0m Cannot cast floating point values to pointers.\n",line);
																			exit(-1);
																		}
																		printf("\e[1;35mWarning [line %d]:\e[0m Cast from '%s' to incompatible type '%s'.\n",line, $4->node_data.c_str(), $2->node_data.c_str());
																	}
																	else if(p2.second){
																		if($2->node_data == "float" || $2->node_data == "double" || $2->node_data == "long double"){
																			printf("\e[1;31mError [line %d]:\e[0m Cannot cast pointers to floating point values.\n",line);
																			exit(-1);
																		}
																		printf("\e[1;35mWarning [line %d]:\e[0m Cast from '%s' to incompatible type '%s'.\n",line, $4->node_data.c_str(), $2->node_data.c_str());
																	}
																	$$->node_data = $2->node_data;
																	$$->ttentry = $2->ttentry;

																	//////////////// 3AC ////////////////
																	$$->place = getNewTemp($$->node_data, $$->ttentry);
																	string op2 = "("+p2.first + "-to-" + p1.first+")"; // Modify
																	int x = emit(op2, $4->place, {"", NULL}, $$->place);
																	/////////////////////////////////////
																}
	;

multiplicative_expression
	: cast_expression											{$$ = $1;}
	| multiplicative_expression '*' cast_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,"*",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;
																		
																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type);
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos){
																			int x = emit("*int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit("*real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit("*real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit("*real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	| multiplicative_expression '/' cast_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,"/",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type);
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos){
																			int x = emit("/int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit("/real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit("/real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit("/real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	| multiplicative_expression '%' cast_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") == string::npos && type.find("char") == string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,"%",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type);
																		int x = emit("%", $1->place, $3->place, $$->place);
																		/////////////////////////////////////
																		
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

additive_expression
	: multiplicative_expression									{$$ = $1;}
	| additive_expression '+' multiplicative_expression			{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	
																	if($1->ttentry){
																		$$->ttentry = $1->ttentry;
																	}
																	else if($3->ttentry){
																		$$->ttentry = $3->ttentry;
																	}

																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,"+",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type, $$->ttentry);
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.back()=='*'){
																			int x = emit("+int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type, $1->ttentry);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit("+real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type, $3->ttentry);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit("+real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit("+real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	| additive_expression '-' multiplicative_expression			{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	if(type.back() == '*' && get_equivalent_pointer($1->node_data).first.back() != '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, "-");
																		exit(-1);
																	}

																	if($1->ttentry){
																		$$->ttentry = $1->ttentry;
																	}
																	else if($3->ttentry){
																		$$->ttentry = $3->ttentry;
																	}

																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,"-",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type, $$->ttentry);
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.back()=='*'){
																			int x = emit("-int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit("-real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit("-real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit("-real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

shift_expression
	: additive_expression										{$$ = $1;}
	| shift_expression LEFT_OP additive_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	// printf("TYPE: %s\n", type.c_str());
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") == string::npos && type.find("char") == string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, LEFT_OP, type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,"<<",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type);
																		int x = emit("<<", $1->place, $3->place, $$->place);
																		/////////////////////////////////////
																	}
																	$$->node_data = $1->node_data;
																	$$->value_type = RVALUE;
																}
	| shift_expression RIGHT_OP additive_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") == string::npos && type.find("char") == string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, RIGHT_OP, type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,">>",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type);
																		int x = emit(">>", $1->place, $3->place, $$->place);
																		/////////////////////////////////////
																	}
																	$$->node_data = $1->node_data;
																	$$->value_type = RVALUE;
																}
	;

relational_expression
	: shift_expression											{$$ = $1;}
	| relational_expression '<' shift_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	//if((p1.second || p2.second) && (!p1.second || !p2.second)){
																		//printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		//exit(-1);
																	//}

																	string type;
																	if(!p1.second && !p2.second){
																		type = arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	}
																	else if(p1.second && p2.second){
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p1.second){
																		if($3->node_data.find("float")!=string::npos || $3->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p2.second){
																		if($1->node_data.find("float")!=string::npos || $1->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	//if(!(p1.second && p2.second))
																	//	type = arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	//else{
																	//	if($1->node_data != $3->node_data){
																	//		printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																	//	}
																	//}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), "int");
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,"<",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp("int");
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.find("*")!=string::npos){
																			int x = emit("<int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit("<real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type); 
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit("<real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit("<real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	| relational_expression '>' shift_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	//if((p1.second || p2.second) && (!p1.second || !p2.second)){
																	//	printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																	//	exit(-1);
																	//}

																	//string type;
																	//if(!(p1.second && p2.second))
																	//	type = arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	//else{
																	//	if($1->node_data != $3->node_data){
																	//		printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																	//	}
																	//}
																	string type;
																	if(!p1.second && !p2.second){
																		type = arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	}
																	else if(p1.second && p2.second){
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p1.second){
																		if($3->node_data.find("float")!=string::npos || $3->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p2.second){
																		if($1->node_data.find("float")!=string::npos || $1->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), "int");
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,">",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp("int");
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.find("*")!=string::npos){
																			int x = emit(">int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit(">real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit(">real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit(">real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	| relational_expression LE_OP shift_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	
																	string type;
																	if(!p1.second && !p2.second){
																		type = arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	}
																	else if(p1.second && p2.second){
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p1.second){
																		if($3->node_data.find("float")!=string::npos || $3->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p2.second){
																		if($1->node_data.find("float")!=string::npos || $1->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}

																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, LE_OP, "int");
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,"<=",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp("int");
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.find("*")!=string::npos){
																			int x = emit("<=int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit("<=real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit("<=real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit("<=real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	| relational_expression GE_OP shift_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	
																	string type;
																	if(!p1.second && !p2.second){
																		type = arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	}
																	else if(p1.second && p2.second){
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p1.second){
																		if($3->node_data.find("float")!=string::npos || $3->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p2.second){
																		if($1->node_data.find("float")!=string::npos || $1->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, GE_OP, "int");
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,">=",-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp("int");
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.find("*")!=string::npos){
																			int x = emit(">=int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit(">=real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit(">=real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit(">=real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	;

equality_expression
	: relational_expression										{$$ = $1;}
	| equality_expression EQ_OP relational_expression			{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	
																	string type;
																	if(!p1.second && !p2.second){
																		type = arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	}
																	else if(p1.second && p2.second){
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p1.second){
																		if($3->node_data.find("float")!=string::npos || $3->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p2.second){
																		if($1->node_data.find("float")!=string::npos || $1->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}

																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, EQ_OP, "int");
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,$2,-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp("int");
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.find("*")!=string::npos){
																			int x = emit("==int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit("==real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit("==real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit("==real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	| equality_expression NE_OP relational_expression			{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	
																	string type;
																	if(!p1.second && !p2.second){
																		type = arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	}
																	else if(p1.second && p2.second){
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p1.second){
																		if($3->node_data.find("float")!=string::npos || $3->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}
																	else if(p2.second){
																		if($1->node_data.find("float")!=string::npos || $1->node_data.find("double")!=string::npos){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																			exit(-1);
																		}
																		if($1->node_data != $3->node_data){
																			printf("\e[1;35mWarning [line %d]:\e[0m Comparison of pointers or types '%s' and '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																		}
																		type = "int*";
																	}

																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, NE_OP, "int");
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,$2,-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp("int");
																		if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.find("*")!=string::npos){
																			int x = emit("!=int", $1->place, $3->place, $$->place);
																		}
																		else {
																			if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																				emit("!=real", tmp, $3->place, $$->place);
																			}
																			else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																				auto tmp = getNewTemp(type);
																				int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																				emit("!=real", $1->place, tmp, $$->place);
																			}
																			else {
																				emit("!=real", $1->place, $3->place, $$->place);
																			}
																		}
																		/////////////////////////////////////
																	}
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	;

and_expression
	: equality_expression										{$$ = $1;}
	| and_expression '&' equality_expression					{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") == string::npos && type.find("char") == string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,$2,-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type);
																		int x = emit("&", $1->place, $3->place, $$->place);
																		/////////////////////////////////////
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

exclusive_or_expression
	: and_expression											{$$ = $1;}
	| exclusive_or_expression '^' and_expression				{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") == string::npos && type.find("char") == string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), type);
																		$$ = $1;
																	}
																	else{	$$ = node_(2,$2,-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type);
																		int x = emit("^", $1->place, $3->place, $$->place);
																		/////////////////////////////////////
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

inclusive_or_expression
	: exclusive_or_expression									{$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression		{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($3->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if(is_struct_or_union($1->node_data) || is_struct_or_union($3->node_data)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2),$1->ttentry,$3->ttentry);
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") == string::npos && type.find("char") == string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2);
																		exit(-1);
																	}
																	if($1->token == CONSTANT && $3->token == CONSTANT){
																		evaluate_const($1, $3, *($2), type);
																		$$ = $1;
																	}
																	else{
																		$$ = node_(2,$2,-1);
																		$$->v[0] = $1;
																		$$->v[1] = $3;

																		//////////////// 3AC ////////////////
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		else if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}

																		$$->place = getNewTemp(type);
																		int x = emit("|", $1->place, $3->place, $$->place);
																		/////////////////////////////////////
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

logical_and_expression
	: inclusive_or_expression									{$$ = $1;}
	| logical_and_expression AND_OP 							{
																	if ($1->truelist.empty() && $1->falselist.empty()){
																	    if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		int x = emit("IF_TRUE_GOTO", $1->place, {"", NULL}, {"", NULL}, nextquad+2);
																		int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
																		$1->falselist.push_back(y);
																	}
																	else {
																		backpatch($1->truelist, nextquad);
																	}
																}


	inclusive_or_expression										{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($4->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($4->node_data);
																	arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2),$1->ttentry,$4->ttentry);
																	
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $4;

																	//////////////// 3AC ////////////////
																	
																	 if($4->token == CONSTANT){
																		$4->place = emitConstant($4);
																	}

																	int x = emit("IF_TRUE_GOTO", $4->place, {"", NULL}, {"", NULL});
																	int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
																	$$->falselist.push_back(y);
																	$$->truelist.push_back(x);
																	$$->falselist.insert($$->falselist.end(), $1->falselist.begin(), $1->falselist.end());

																	/////////////////////////////////////
																	
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	;

logical_or_expression
	: logical_and_expression									{$$ = $1;}
	| logical_or_expression OR_OP								{
																	if ($1->falselist.empty() && $1->truelist.empty()){
																		if($1->token == CONSTANT){
																			$1->place = emitConstant($1);
																		}
																		int x = emit("IF_TRUE_GOTO", $1->place, {"", NULL}, {"", NULL});
																		//int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL}, nextquad+1);
																		$1->truelist.push_back(x);
																	}
																	else {
																		backpatch($1->falselist, nextquad);
																	}
																}

 

	logical_and_expression		{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	if($4->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($4->node_data);
																	arithmetic_type_upgrade(p1.first,p2.first,string((const char*)$2),$1->ttentry,$4->ttentry);
																	
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $4;

																	//////////////// 3AC ////////////////
																	
																	if($4->token == CONSTANT){
																		$4->place = emitConstant($4);
																	}

																	//$$->place = getNewTemp("int");
																	//emit("||", $1->place, $3->place, $$->place);

																	if ($4->truelist.empty()){
																		int x = emit("IF_TRUE_GOTO", $4->place, {"", NULL}, {"", NULL});
																		int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
																		$4->truelist.push_back(x);
																		$4->falselist.push_back(y);
																	}
																	$$->falselist = $4->falselist;
																	$$->truelist.insert($$->truelist.end(), $1->truelist.begin(), $1->truelist.end());
																	$$->truelist.insert($$->truelist.end(), $4->truelist.begin(), $4->truelist.end());

																		/////////////////////////////////////
																	
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	;

M7
	: /* empty */ 										{	
															//////////////// 3AC ////////////////
															node* tmp = $<nodes>-1;
															if(tmp->token == CONSTANT){
																tmp->place = emitConstant(tmp);
															}
															int x = emit("=", tmp->place, {"", NULL}, {"", NULL}); /////// See this again after assignment
															int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
															tmp->nextlist.push_back(y);
															backpatch($<nodes>-4->falselist, nextquad);
															//backpatch($<nodes>-4->falselist, nextquad);
															$$ = x;
															/////////////////////////////////////
														}

conditional_expression
	: logical_or_expression								{
															if ($1->truelist.empty() && $1->falselist.empty()) $$ = $1;
															else {
																$$->place = getNewTemp("int");
																int x = emit("=", {"$1", NULL}, {"", NULL}, $$->place);
																emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL}, nextquad+2);
																int y = emit("=", {"$0", NULL}, {"", NULL}, $$->place);
																backpatch($1->truelist, x);
																backpatch($1->falselist, y);
															}
															$$->ttentry = $1->ttentry;
														}
	| logical_or_expression '?'	 						{
															//////////////// 3AC ////////////////
															if ($1->truelist.empty() && $1->falselist.empty())
															{
															if($1->token == CONSTANT){
																$1->place = emitConstant($1);
																// printf("Ternary condition constant. %s %d\n", $1->place.first.c_str(), $1->val.int_const);
															}
															int x = emit("IF_TRUE_GOTO", $1->place, {"", NULL}, {"", NULL});
															int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
															$1->truelist.push_back(x);
															$1->falselist.push_back(y);
															}
															backpatch($1->truelist, nextquad);
															/////////////////////////////////////
														}
	expression ':' M7 conditional_expression					{
																	if($1->node_data == "void"){
																		printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																		exit(-1);
																	}
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($4->node_data);
																	pair<string,int> p3 = get_equivalent_pointer($7->node_data);
																	if(p1.second != 0){
																		arithmetic_type_upgrade(p1.first,"int", "ternary");
																	}
																	if((p2.second || p3.second) && (!p2.second || !p3.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for ternary operator.\n",line);
																		exit(-1);
																	}
																	/* // if($1->token == CONSTANT){
																	// 	int tmp;
																	// 	switch($1->val_dt){
																	// 		case IS_INT: tmp = $1->val.int_const; break;
																	// 		case IS_LONG: tmp = $1->val.long_const; break;
																	// 		case IS_SHORT: tmp = $1->val.short_const; break;
																	// 		case IS_U_INT: tmp = $1->val.u_int_const; break;
																	// 		case IS_U_LONG: tmp = $1->val.u_long_const; break;
																	// 		case IS_U_SHORT: tmp = $1->val.u_short_const; break;
																	// 		case IS_FLOAT: tmp = $1->val.float_const; break;
																	// 		case IS_DOUBLE: tmp = $1->val.double_const; break;
																	// 		case IS_LONG_DOUBLE: tmp = $1->val.long_double_const; break;
																	// 		case IS_CHAR: tmp = $1->val.char_const; break;
																	// 	}
																	// 	if(tmp){
																	// 		$$ = $4;
																	// 	}
																	// 	else{
																	// 		$$ = $7;
																	// 	}
																	// 	$$->value_type = RVALUE;
																	// }
																	// else{ */
																	
																	$$ = node_(3,"ternary",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $4;
																	$$->v[2] = $7;
																	if(p3.second && p2.second){
																		$$->node_data = "void *";
																	}
																	else if(p2.first == p3.first){
																		$$->node_data = p3.first;
																	}else{
																		$$->node_data = arithmetic_type_upgrade(p2.first, p3.first, "ternary");
																	}
																	$$->value_type = RVALUE;
																	// }

																	//////////////// 3AC ////////////////
																	if($7->token == CONSTANT){
																		$7->place = emitConstant($7);
																	}
																	$$->place = getNewTemp($$->node_data);
																	emit("=", $7->place, {"", NULL}, $$->place);
																	code_array[$6].res = $$->place;
																	$$->nextlist.insert($$->nextlist.end(), $4->nextlist.begin(), $4->nextlist.end());
																	//backpatch($4->nextlist, nextquad);
																	$$->nextlist.insert($$->nextlist.end(), $7->nextlist.begin(), $7->nextlist.end());
																	/////////////////////////////////////
																}
	;

assignment_expression
	: conditional_expression										{$$ = $1;}
	| unary_expression assignment_operator assignment_expression	{
																			if($1->node_data == "void"){
																				printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																				exit(-1);
																			}
																			if($3->node_data == "void"){
																				printf("\e[1;31mError [line %d]:\e[0m void value not ignored as it ought to be.\n",line);
																				exit(-1);
																			}
																			$$ = $2;
																			add_node($$, $1);
																			add_node($$, $3);
																			if($1->value_type == RVALUE){
																				printf("\e[1;31mError [line %d]:\e[0m lvalue required as left operand of '%s'.\n",line, $2->name);
																				exit(-1);
																			}
																			string type;
																			string type1, type2;
																			pair<string, int> p1 = get_equivalent_pointer($1->node_data);
																			pair<string, int> p2 = get_equivalent_pointer($3->node_data);
																			type1 = p1.first, type2 = p2.first;
																			//tt_entry* entry1 = type_lookup(type1);
																			//tt_entry* entry2 = type_lookup(type2);
																			tt_entry* entry1 = NULL;
																			tt_entry* entry2 = NULL;
																			if(p1.second==0){
																				entry1 = $1->ttentry;
																			}
																			if(p2.second==0){
																				entry2 = $3->ttentry;
																			}
																			if(entry1 && entry2){
																				if(entry1 != entry2 || $2->name[0] != '='){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operators '%s'.\n",line, $2->name);
																					exit(-1);
																				}
																			}
																			else if(entry1 || entry2){
																				printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2->name);
																				exit(-1);
																			}
																			if(($1->node_data.find("[") != string::npos && $1->node_data.find("[]") == string::npos)){
																				printf("\e[1;31mError [line %d]:\e[0m Array variables cannot be reassigned.\n",line);
																				exit(-1);
																			}
																			string op;
																			switch($2->token){
																			case '=':
																				if(p1.second && p2.second){
																					if(p1.first != p2.first){
																						printf("\e[1;35mWarning [line %d]:\e[0m Assignment to '%s' from incompatible pointer type '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																					}
																				}
																				else if(p1.second){
																					if($3->node_data == "float" || $3->node_data == "double" || $3->node_data == "long double"){
																						printf("\e[1;31mError [line %d]:\e[0m Cannot assign floating point values to pointers.\n",line);
																						exit(-1);
																					}
																					printf("\e[1;35mWarning [line %d]:\e[0m Assignment to '%s' from incompatible type '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																				}
																				else if(p2.second){
																					if($1->node_data == "float" || $1->node_data == "double" || $1->node_data == "long double"){
																						printf("\e[1;31mError [line %d]:\e[0m Cannot assign pointers to floating point values.\n",line);
																						exit(-1);
																					}
																					printf("\e[1;35mWarning [line %d]:\e[0m Assignment to '%s' from incompatible type '%s'.\n",line, $1->node_data.c_str(), $3->node_data.c_str());
																				}



																				//////////////// 3AC ////////////////
																				backpatch($3->nextlist, nextquad); // CHECK THIS!!!!!


																				if($3->token == CONSTANT){
																					$3->place = emitConstant($3);
																				}

																				if(type1 != type2){
																					qi tmpvar = getNewTemp($1->node_data, $1->ttentry);
																					string cast = "("+ type2 +"-to-"+ type1 +")";
																					emit(cast, $3->place, {"", NULL}, tmpvar);

																					if(!strcmp($1->name, "UNARY*")){
																						emit("ADDR=", tmpvar, {"", NULL}, $1->v[0]->place);
																					}
																					else{
																						emit("=", tmpvar, {"", NULL}, $1->place);
																					}
																				}
																				else{
																					if(!strcmp($1->name, "UNARY*")){
																						emit("ADDR=", $3->place, {"", NULL}, $1->v[0]->place);
																					}
																					else{
																						emit("=", $3->place, {"", NULL}, $1->place);
																					}
																				}
																				$$->place = $3->place;
																				$$->nextlist.insert($$->nextlist.end(), $3->nextlist.begin(), $3->nextlist.end());
																				/////////////////////////////////////
																			break;
																			case MUL_ASSIGN:
																			case DIV_ASSIGN:
																				type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2->name),$1->ttentry,$3->ttentry);
																				if(type.back() == '*'){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2->name);
																					exit(-1);
																				}
																				//////////////// 3AC ////////////////
																				backpatch($3->nextlist, nextquad); // CHECK THIS!!!!!
																				if($3->token == CONSTANT){
																					$3->place = emitConstant($3);
																				}

																				$$->place = $1->place;
																				op = string((const char*) $2->name);
																				op.pop_back();
																				if(type.find("int")!=string::npos || type.find("char")!=string::npos){
																					auto tmp = getNewTemp(type);
																					int x = emit(op+"int", $1->place, $3->place, tmp);

																					// Typecasting before assignment
																					if($1->node_data!=type){
																						auto tmp2 = getNewTemp($1->node_data);
																						string cast = "("+ type +"-to-"+ $1->node_data +")";
																						emit(cast, tmp, {"", NULL}, tmp2);
																						// emit("=", tmp2, {"", NULL}, $1->place);

																						if(!strcmp($1->name, "UNARY*")){
																							emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																						}
																						else{
																							emit("=", tmp2, {"", NULL}, $1->place);
																						}
																					}
																					else{
																						// emit("=", tmp, {"", NULL}, $1->place);
																						if(!strcmp($1->name, "UNARY*")){
																							emit("ADDR=", tmp, {"", NULL}, $1->v[0]->place);
																						}
																						else{
																							emit("=", tmp, {"", NULL}, $1->place);
																						}
																					}
																				}
																				else {
																					if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																						auto tmp = getNewTemp(type);
																						auto tmp2 = getNewTemp(type);
																						int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																						emit(op+"real", tmp, $3->place, tmp2);
																						
																						// Typecasting before assignment
																						if($1->node_data!=type){
																							auto tmp3 = getNewTemp($1->node_data);
																							string cast = "("+ type +"-to-"+ $1->node_data +")";
																							emit(cast, tmp2, {"", NULL}, tmp3);
																							// emit("=", tmp3, {"", NULL}, $1->place);
																							if(!strcmp($1->name, "UNARY*")){
																								emit("ADDR=", tmp3, {"", NULL}, $1->v[0]->place);
																							}
																							else{
																								emit("=", tmp3, {"", NULL}, $1->place);
																							}
																						}
																						else{
																							// emit("=", tmp2, {"", NULL}, $1->place);
																							if(!strcmp($1->name, "UNARY*")){
																								emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																							}
																							else{
																								emit("=", tmp2, {"", NULL}, $1->place);
																							}
																						}
																					}
																					else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																						auto tmp = getNewTemp(type);
																						auto tmp2 = getNewTemp(type);
																						int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																						emit(op+"real", $1->place, tmp, tmp2);
																						if(!strcmp($1->name, "UNARY*")){
																							emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																						}
																						else{
																							emit("=", tmp2, {"", NULL}, $1->place);
																						}
																					}
																					else {
																						/* emit(op+"real", $1->place, $3->place, $1->place); */
																						auto tmp = getNewTemp(type);
																						int x = emit(op+"real", $1->place, $3->place, tmp);

																						// Typecasting before assignment
																						if($1->node_data!=type){
																							auto tmp2 = getNewTemp($1->node_data);
																							string cast = "("+ type +"-to-"+ $1->node_data +")";
																							emit(cast, tmp, {"", NULL}, tmp2);
																							// emit("=", tmp2, {"", NULL}, $1->place);
																							if(!strcmp($1->name, "UNARY*")){
																								emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																							}
																							else{
																								emit("=", tmp2, {"", NULL}, $1->place);
																							}
																						}
																						else{
																							// emit("=", tmp, {"", NULL}, $1->place);
																							if(!strcmp($1->name, "UNARY*")){
																								emit("ADDR=", tmp, {"", NULL}, $1->v[0]->place);
																							}
																							else{
																								emit("=", tmp, {"", NULL}, $1->place);
																							}
																						}
																					}
																				}
																				/////////////////////////////////////
																			break;
																			case ADD_ASSIGN:
																			case SUB_ASSIGN:
																				type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2->name),$1->ttentry,$3->ttentry);
																				
																				//////////////// 3AC ////////////////
																				backpatch($3->nextlist, nextquad); // CHECK THIS!!!!!
																				if($3->token == CONSTANT){
																					$3->place = emitConstant($3);
																				}

																				$$->place = $1->place;
																				op = string((const char*) $2->name);
																				op.pop_back();
																				if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.back()=='*'){
																					/* int x = emit(op+"int", $1->place, $3->place, $1->place); */
																					auto tmp = getNewTemp(type, $1->ttentry);
																					int x = emit(op+"int", $1->place, $3->place, tmp);

																					// Typecasting before assignment
																					if($1->node_data!=type){
																						auto tmp2 = getNewTemp($1->node_data, $1->ttentry);
																						string cast = "("+ type +"-to-"+ $1->node_data +")";
																						emit(cast, tmp, {"", NULL}, tmp2);
																						// emit("=", tmp2, {"", NULL}, $1->place);
																						if(!strcmp($1->name, "UNARY*")){
																							emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																						}
																						else{
																							emit("=", tmp2, {"", NULL}, $1->place);
																						}
																					}
																					else{
																						// emit("=", tmp, {"", NULL}, $1->place);
																						if(!strcmp($1->name, "UNARY*")){
																							emit("ADDR=", tmp, {"", NULL}, $1->v[0]->place);
																						}
																						else{
																							emit("=", tmp, {"", NULL}, $1->place);
																						}
																					}
																				}
																				else {
																					if($1->node_data.find("int")!=string::npos || $1->node_data.find("char")!=string::npos){
																						auto tmp = getNewTemp(type);
																						auto tmp2 = getNewTemp(type);
																						int x = emit("inttoreal", $1->place, {"", NULL}, tmp);
																						emit(op+"real", tmp, $3->place, tmp2);

																						// Typecasting before assignment
																						if($1->node_data!=type){
																							auto tmp3 = getNewTemp($1->node_data);
																							string cast = "("+ type +"-to-"+ $1->node_data +")";
																							emit(cast, tmp2, {"", NULL}, tmp3);
																							// emit("=", tmp3, {"", NULL}, $1->place);
																							if(!strcmp($1->name, "UNARY*")){
																								emit("ADDR=", tmp3, {"", NULL}, $1->v[0]->place);
																							}
																							else{
																								emit("=", tmp3, {"", NULL}, $1->place);
																							}
																						}
																						else{
																							// emit("=", tmp2, {"", NULL}, $1->place);
																							if(!strcmp($1->name, "UNARY*")){
																								emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																							}
																							else{
																								emit("=", tmp2, {"", NULL}, $1->place);
																							}
																						}
																					}
																					else if ($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
																						auto tmp = getNewTemp(type);
																						auto tmp2 = getNewTemp(type);
																						int x = emit("inttoreal", $3->place, {"", NULL}, tmp);
																						emit(op+"real", $1->place, tmp, tmp2);
																						if(!strcmp($1->name, "UNARY*")){
																							emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																						}
																						else{
																							emit("=", tmp2, {"", NULL}, $1->place);
																						}
																					}
																					else {
																						/* emit(op+"real", $1->place, $3->place, $1->place); */
																						auto tmp = getNewTemp(type);
																						int x = emit(op+"real", $1->place, $3->place, tmp);

																						// Typecasting before assignment
																						if($1->node_data!=type){
																							auto tmp2 = getNewTemp($1->node_data);
																							string cast = "("+ type +"-to-"+ $1->node_data +")";
																							emit(cast, tmp, {"", NULL}, tmp2);
																							// emit("=", tmp2, {"", NULL}, $1->place);
																							if(!strcmp($1->name, "UNARY*")){
																								emit("ADDR=", tmp2, {"", NULL}, $1->v[0]->place);
																							}
																							else{
																								emit("=", tmp2, {"", NULL}, $1->place);
																							}
																						}
																						else{
																							// emit("=", tmp, {"", NULL}, $1->place);
																							if(!strcmp($1->name, "UNARY*")){
																								emit("ADDR=", tmp, {"", NULL}, $1->v[0]->place);
																							}
																							else{
																								emit("=", tmp, {"", NULL}, $1->place);
																							}
																						}
																						
																					}
																				}
																				/////////////////////////////////////
																			break;
																			case MOD_ASSIGN:
																			case LEFT_ASSIGN:
																			case RIGHT_ASSIGN:
																			case AND_ASSIGN:
																			case XOR_ASSIGN:
																			case OR_ASSIGN:{
																				type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2->name),$1->ttentry,$3->ttentry);
																				if(type.back() == '*'){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2->name);
																					exit(-1);
																				}
																				if(type.find("int") == string::npos && type.find("char") == string::npos){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '%s'.\n",line, $2->name);
																					exit(-1);
																				}

																				//////////////// 3AC ////////////////
																				else if($3->token == CONSTANT){
																					$3->place = emitConstant($3);
																				}

																				$$->place = $1->place;
																				op = string((const char*) $2->name);
																				op.pop_back();
																				/* int x = emit(op, $1->place, $3->place, $1->place); */

																				auto tmp = getNewTemp(type);
																				int x = emit(op+"int", $1->place, $3->place, tmp);

																				// Typecasting before assignment
																				if($1->node_data!=$3->node_data){
																					auto tmp2 = getNewTemp($1->node_data);
																					string cast = "("+ $3->node_data +"-to-"+ $1->node_data +")";
																					emit(cast, tmp, {"", NULL}, tmp2);
																					emit("=", tmp2, {"", NULL}, $1->place);
																				}
																				else{
																					emit("=", tmp, {"", NULL}, $1->place);
																				}
																				/////////////////////////////////////
																			}
																		}
																		$$->ttentry = $1->ttentry;
																		$$->node_data = $1->node_data;
																		$$->value_type = RVALUE;
																	}
	;

assignment_operator
	: '='														{$$ = node_(0,"=",'=');}
	| MUL_ASSIGN												{$$ = node_(0,$1,MUL_ASSIGN);}
	| DIV_ASSIGN												{$$ = node_(0,$1,DIV_ASSIGN);}
	| MOD_ASSIGN												{$$ = node_(0,$1,MOD_ASSIGN);}
	| ADD_ASSIGN												{$$ = node_(0,$1,ADD_ASSIGN);}
	| SUB_ASSIGN												{$$ = node_(0,$1,SUB_ASSIGN);}
	| LEFT_ASSIGN												{$$ = node_(0,$1,LEFT_ASSIGN);}
	| RIGHT_ASSIGN												{$$ = node_(0,$1,RIGHT_ASSIGN);}
	| AND_ASSIGN												{$$ = node_(0,$1,AND_ASSIGN);}
	| XOR_ASSIGN												{$$ = node_(0,$1,XOR_ASSIGN);}
	| OR_ASSIGN													{$$ = node_(0,$1,OR_ASSIGN);}
	;

expression
	: assignment_expression						{$$ = $1;}
	| expression ',' 							{
													//////////////// 3AC ////////////////
													backpatch($1->nextlist, nextquad);
													/////////////////////////////////////
												}
	assignment_expression						{
													$$ = node_(2,$2,-1);
													$$->v[0] = $1;
													$$->v[1] = $4;
													$$->node_data = $4->node_data;
													$$->value_type = $4->value_type;
													$$->val = $4->val;
													$$->val_dt = $4->val_dt;
													$$->token = $4->token;
													$$->ttentry = $4->ttentry;

													//////////////// 3AC ////////////////
													$$->nextlist = $4->nextlist;
													$$->place = $4->place;
													/////////////////////////////////////
												}
	;

constant_expression
	: conditional_expression									{
																	$$ = $1;
																	if($$->token != CONSTANT){
																		printf("\e[1;31mError [line %d]:\e[0m Expression used here must be a constant.\n", line);
																		exit(-1);
																	}
																}
	;

declaration
	: declaration_specifiers ';'								{
																	$$ = NULL;
																	string tmp = $1->node_data;
																	tmp = get_eqtype(tmp,0);
																	vector<string> a;
																	string tmp2="";
																	for(auto ch: tmp){
																		if(ch==' '){
																			break;
																		}
																		else{
																			tmp2+=ch;
																		}
																	}
																	if(tmp2=="struct" || tmp2=="union"){
																		tt_entry * entry = current_type_lookup(tmp);
																		if(entry==NULL){
																			entry = add_type_entry(tmp,tmp2);
																		}
																	}
																	else if(tmp2=="enum"){
																		/*Not handled Now*/
																	}
																}
	| declaration_specifiers M3 init_declarator_list ';'		{$$ = $3; data_type = "";}
	;

declaration_specifiers
	: storage_class_specifier									{}
	| storage_class_specifier declaration_specifiers			{}
	| type_specifier											{
																	$$ = node_(1,"type_name",-1);
																	$$->v[0] = $1;
																	// $$->node_data+=" ";
																	$$->node_data=$1->node_data;
																}
	| type_specifier declaration_specifiers						{
																	push_front($2,$1);
																	$$ = $2;
																	// $$->node_data+=" ";
																	if($$->node_data!="") $1->node_data += ' ';
																	$1->node_data += $$->node_data;
																	$$->node_data = $1->node_data;
																}
	| type_qualifier											{}
	| type_qualifier declaration_specifiers						{}
	;

init_declarator_list
	: init_declarator											{if($1){$$ = node_(1,"init_list",-1); $$->v[0] = $1; $$->nextlist = $1->nextlist;}}
	| init_declarator_list ',' 					{
													//////////////// 3AC ////////////////
													// printf("%d\n", $1->nextlist.size());
													if ($1) backpatch($1->nextlist, nextquad);
													/////////////////////////////////////
												}
	init_declarator								{
													if($4 == NULL) $$ = $1;
													else if($1 == NULL){
														$1 = node_(1,"init_list",-1);
														$1->v[0] = $4;
														$$ = $1;
													}
													else{
														add_node($1,$4);
														$$ = $1;
													}

													//////////////// 3AC ////////////////
													if ($$ && $4){
														$$->nextlist = $4->nextlist;
														$$->place = $4->place; // IS THIS NEEDED?
													}
													/////////////////////////////////////
												}
	;

init_declarator
	: declarator												{
																	string data_type_ = data_type;
																	
																	if($1->node_data!=""){
																		data_type_ += " ";
																		data_type_ += $1->node_data;
																	}
																	if(current_lookup($1->node_name)!=NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Redeclaration of '%s'.\n", line, $1->node_name.c_str());
																		exit(-1);
																	}
																	check_valid_array(data_type_);
																	if($1->node_type == 1){
																		st_entry* tmp = add_entry($1->node_name, data_type_,get_size(data_type_),accumulate(offset.begin()+1, offset.end(), 0),IS_FUNC);
																		tmp->type_name = IS_FUNC;
																		check_param_list(func_params);
																		vector<pair<pair<string, string>,tt_entry*>> *temp = new vector<pair<pair<string, string>,tt_entry*>>(func_params);
																		tmp->arg_list = temp;
																		tmp->ttentry = type_lookup(data_type);
																	}
																	else{
																		st_entry* tmp = add_entry($1->node_name, data_type_, get_size(data_type_),accumulate(offset.begin()+1, offset.end(), 0),IS_VAR);
																		if(data_type_ == "void"){
																			printf("\e[1;31mError [line %d]:\e[0m Variable or field '%s' declared void.\n", line, $1->node_name.c_str());
																			exit(-1);
																		}
																		tmp->type_name = IS_VAR;
																		tmp->ttentry = type_lookup(data_type);
																	}
																	$$ = NULL; free($1);
																}
	| declarator '=' initializer								{	
																	$$ = node_(2,"=",-1); $$->v[0] = $1; $$->v[1] = $3;
																	if($1->node_type == 1){
																		printf("Function pointers not supported\n");
																		exit(-1);
																	}
																	else{
																		string data_type_ = data_type;
																		if($1->node_data!=""){
																			data_type_ += " ";
																			data_type_ += $1->node_data;
																		}
																		
																		if(current_lookup($1->node_name)!=NULL){
																			printf("\e[1;31mError [line %d]:\e[0m Redeclaration of '%s'.\n", line, $1->node_name.c_str());
																			exit(-1);
																		}
																		if(table_scope.back()==&global && $3->token != CONSTANT){
																			printf("\e[1;31mError [line %d]:\e[0m Initializer for '%s' must be a constant.\n", line, $1->node_name.c_str());
																			exit(-1);
																		}
																		st_entry* tmp = add_entry($1->name, data_type_, get_size(data_type_), accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);/*change IS_VAR*/
																		$1->place = {$1->node_name, tmp};
																		tmp->ttentry = type_lookup(data_type);
																		$1->ttentry = tmp->ttentry;
																		if(data_type_ == "void"){
																			printf("\e[1;31mError [line %d]:\e[0m Variable or field '%s' declared void.\n", line, $1->node_name.c_str());
																			exit(-1);
																		}
																		string type;
																		string type1, type2;
																		pair<string, int> p1 = get_equivalent_pointer(data_type_);
																		pair<string, int> p2 = get_equivalent_pointer($3->node_data);
																		type1 = p1.first, type2 = p2.first;
																		tt_entry* entry1 = type_lookup(type1);
																		tt_entry* entry2 = NULL;
																		if($3->node_data.back() != '*'){
																			entry2 = $3->ttentry;
																		}
																		if(entry1 && entry2){
																			if(entry1 != entry2){
																				printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '='.\n",line);
																				exit(-1);
																			}
																		}
																		else if(entry1 || entry2){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator '='.\n",line);
																			exit(-1);
																		}
																		if((data_type_.find("[") != string::npos && data_type_.find("[]") == string::npos)){
																			printf("\e[1;31mError [line %d]:\e[0m Array variables cannot be reassigned.\n",line);
																			exit(-1);
																		}
																		if(p1.second && p2.second){
																			if(p1.first != p2.first){
																				printf("\e[1;35mWarning [line %d]:\e[0m Assignment to '%s' from incompatible pointer type '%s'.\n",line, data_type_.c_str(), $3->node_data.c_str());
																			}
																		}
																		else if(p1.second){
																			if($3->node_data == "float" || $3->node_data == "double" || $3->node_data == "long double"){
																				printf("\e[1;31mError [line %d]:\e[0m Cannot assign floating point values to pointers.\n",line);
																				exit(-1);
																			}
																			printf("\e[1;35mWarning [line %d]:\e[0m Assignment to %s from incompatible type %s.\n",line, data_type_.c_str(), $3->node_data.c_str());
																		}
																		else if(p2.second){
																			if(data_type_ == "float" || data_type_ == "double" || data_type_ == "long double"){
																				printf("\e[1;31mError [line %d]:\e[0m Cannot assign pointers to floating point values.\n",line);
																				exit(-1);
																			}
																			printf("\e[1;35mWarning [line %d]:\e[0m Assignment to '%s' from incompatible type '%s'.\n",line, data_type_.c_str(), $3->node_data.c_str());
																		}
																		$$->node_data = data_type_;
																		$$->value_type = RVALUE;

																		//////////////// 3AC ////////////////

																		if($3->token == CONSTANT){
																			$3->place = emitConstant($3);
																		}
																		if(type1 != type2){
																			qi tmpvar = getNewTemp($$->node_data, $$->ttentry);

																			string cast = "("+ type2 +"-to-"+ type1 +")";
																			emit(cast, $3->place, {"", NULL}, tmpvar);

																			if(!strcmp($1->name, "UNARY*")){
																				emit("ADDR=", tmpvar, {"", NULL}, $1->v[0]->place);
																			}
																			else{
																				emit("=", tmpvar, {"", NULL}, $1->place);
																			}
																		}
																		else{
																			if(!strcmp($1->name, "UNARY*")){
																				emit("ADDR=", $3->place, {"", NULL}, $1->v[0]->place);
																			}
																			else{
																				emit("=", $3->place, {"", NULL}, $1->place);
																			}
																		}
																		
																		// int x = emit("=", $3->place, {"", NULL}, $1->place);
																		$$->place = $1->place;
																		$$->nextlist.insert($$->nextlist.end(), $3->nextlist.begin(), $3->nextlist.end());
																		/////////////////////////////////////
																	}

																}
	;

storage_class_specifier
	: TYPEDEF													{printf("\e[1;31mError [line %d]:\e[0m No support for storage class specifiers.\n", line);exit(-1);}
	| EXTERN													{printf("\e[1;31mError [line %d]:\e[0m No support for storage class specifiers.\n", line);exit(-1);}
	| STATIC													{printf("\e[1;31mError [line %d]:\e[0m No support for storage class specifiers.\n", line);exit(-1);}
	| AUTO														{printf("\e[1;31mError [line %d]:\e[0m No support for storage class specifiers.\n", line);exit(-1);}
	| REGISTER													{printf("\e[1;31mError [line %d]:\e[0m No support for storage class specifiers.\n", line);exit(-1);}
	;

type_specifier
	: VOID														{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| CHAR														{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| SHORT														{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| INT														{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| LONG														{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| FLOAT														{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| DOUBLE													{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| SIGNED													{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| UNSIGNED													{$$ = node_(0,$1,-1); $$->node_data = $1;}
	| struct_or_union_specifier									{$$ = $1;}
	| enum_specifier											{$$ = $1;}
	| TYPE_NAME													{printf("\e[1;31mError [line %d]:\e[0m Typedef not handled\n", line);exit(-1);}
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER M4 '{' struct_declaration_list M5 '}'
																	{	
																		char* ch = (char*)malloc(sizeof(char)*(strlen($1)+strlen($2)+2)); 
																		strcpy(ch,$1); strcat(ch," "); strcat(ch,$2);
																		$$ = node_(1,ch,-1); free(ch);
																		$$->v[0] = $5;
																		$$->node_data = string((const char*)$1) + " " + string((const char*)$2);
																		string temp1((const char*)$1);
																		string temp2((const char*)$2);
																		tt_entry* tmp2 = current_type_lookup(temp2+" "+temp1); 
																		$$->ttentry = tmp2;
																	}	

	| struct_or_union M6 M4 '{' struct_declaration_list M5 '}'		{
																		$$ = node_(1,$1,-1); $$->v[0]=$5;
																		$$->node_data = string((const char*)$1) + " " + string((const char*)$2);
																		string temp1((const char*)$1);
																		string temp2((const char*)$2);
																		tt_entry* tmp2 = current_type_lookup(temp2+" "+temp1); 
																		$$->ttentry = tmp2;
																	}

	| struct_or_union IDENTIFIER 									{
																		char* ch = (char*)malloc(sizeof(char)*(strlen($1)+strlen($2)+2)); 
																		strcpy(ch,$1); strcat(ch," "); strcat(ch,$2);
																		$$ = node_(0,ch,-1); free(ch);
																		$$->node_data = string((const char*)$1)+" "+string((const char*)$2);
																		$$->ttentry = type_lookup($$->node_data);
																	}
	;

M4
	:	/* empty */													{
																		string temp1((const char*)$<id>0);
																		string temp2((const char*)$<id>-1);
																		tt_entry* tmp2 = current_type_lookup(temp2+" "+temp1); 
																		if(tmp2==NULL){
																			tt_entry* tmp = add_type_entry(temp2+" "+temp1, temp2);
																		}
																		else if(tmp2->is_init == 1){
																			printf("\e[1;31mError [line %d]:\e[0m Redefinition of '%s'.\n", line, (temp2+" "+temp1).c_str());
																			exit(-1);
																		}
																	}
	;

M5
	:	/* empty */													{
																		node* temp = $<nodes>0;
																		tt_entry* struct_entry = type_lookup(string($<id>-4)+" "+string($<id>-3));
																		//string tmp2;
																		for(auto i : temp->v){
																			for(auto j : i->v[1]->v){
																				string type = i->v[0]->node_data;
																				if(type!="" && j->node_data!="") type+= ' ';
																				type += j->node_data;
																				string name = j->node_name;
																				if(struct_entry->mem_list == NULL){
																					vector<pair<pair<string, string>,tt_entry*>> *tmp =  new vector<pair<pair<string, string>,tt_entry*>>;
																					struct_entry->mem_list = tmp;
																				}
																				struct_init_check(type);
																				i->v[1]->ttentry = type_lookup(type);
																				struct_entry->mem_list->push_back({{type, name},i->v[1]->ttentry});
																			}
																		}
																		check_mem_list(*(struct_entry->mem_list), string($<id>-4)+" "+string($<id>-3));
																		struct_entry->is_init = 1;
																	}			
	;

M6
	:	/* empty */													{
																		string tmp = to_string(unnamed_var++);
																		$$ = (char*) malloc(sizeof(char) * (tmp.length()+1));
																		for(int i=0; i<tmp.length(); i++){
																			$$[i] = tmp[i];
																		}
																		$$[tmp.length()]='\0';
																	}
	;
																

struct_or_union
	: STRUCT													{$$ = $1;}
	| UNION														{$$ = $1;}
	;

struct_declaration_list
	: struct_declaration										{$$ = node_(1,"struct_members",-1); $$->v[0] = $1;}
	| struct_declaration_list struct_declaration				{add_node($1, $2); $$ = $1;}
	;

struct_declaration
	: specifier_qualifier_list M3 struct_declarator_list ';'	{
																	$1->node_data = get_eqtype($1->node_data);
																	$$ = node_(2,"struct_decl",-1); $$->v[0] = $1; $$->v[1] = $3;
																}
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list		{
														$$ = $2; push_front($$,$1);
														if($$->node_data != "")
															$$->node_data = " " + $$->node_data;
														$$->node_data = $1->node_data + $$->node_data;
														$$->ttentry = type_lookup($$->node_data);
													}

	| type_specifier								{
														$$ = node_(1,"type_list",-1); $$->v[0] = $1;
														$$->node_data = $1->node_data;
														$$->ttentry = type_lookup($$->node_data);
													}

	| type_qualifier specifier_qualifier_list		{$$ = $2; push_front($$, $1); /* Not propagating in node_data */}
	| type_qualifier								{$$ = node_(1,"type_list",-1); $$->v[0] = $1; /* Not propagating in node_data */}
	;

struct_declarator_list
	: struct_declarator								{$$ = node_(1,"struct_decl_list",-1); $$->v[0]=$1;}
	| struct_declarator_list ',' struct_declarator	{add_node($1,$3); $$ = $1;}
	;

struct_declarator
	: declarator 									{
														$$ = $1;
														string data_type_ = data_type;
														while(data_type_.back() == '*')
															data_type_ = reduce_pointer_level(data_type_);
													}
	| ':' constant_expression						{$$ = node_(2,"bit_field",-1); $$->v[0]=node_(0,$1,-1); $$->v[1]=$2; printf("Bit-fields are not supported\n"); exit(-1);}
	| declarator ':' constant_expression			{$$ = node_(3,"bit_field",-1); $$->v[0]=$1; $$->v[1]=node_(0,$2,-1); $$->v[2]=$3; printf("Bit-fields are not supported\n"); exit(-1);}
	;

enum_specifier
	: ENUM '{' enumerator_list '}'					{printf("Enumerators are not supported\n"); exit(-1); $$ = node_(0,"enum",-1);}
	| ENUM IDENTIFIER '{' enumerator_list '}'		{printf("Enumerators are not supported\n"); exit(-1); char * ch = (char*)malloc((strlen($1)+strlen($2)+2)*sizeof(char));
													strcpy(ch,$1), strcat(ch," "), strcat(ch,$2);
													$$ = node_(0,ch,-1);}
	| ENUM IDENTIFIER								{printf("Enumerators are not supported\n"); exit(-1); char * ch = (char*)malloc((strlen($1)+strlen($2)+2)*sizeof(char));
													strcpy(ch,$1), strcat(ch," "), strcat(ch,$2);
													$$ = node_(0,ch,-1);}
	;

enumerator_list
	: enumerator									{}
	| enumerator_list ',' enumerator				{}
	;

enumerator
	: IDENTIFIER									{}
	| IDENTIFIER '=' constant_expression			{}
	;

type_qualifier
	: CONST											{$$ = node_(0,$1,CONST); printf("type qualifiers are not supported\n"); exit(-1);}
	| VOLATILE										{$$ = node_(0,$1,VOLATILE); printf("type qualifiers are not supported\n"); exit(-1);}
	;

declarator /**/
	: pointer direct_declarator						{
														$$ = $2; add_node($$,$1);
														// string data_type_ = data_type;
														// data_type_+=" ";
														string data_type_ = "";
														for(int i = 0; i < $$->v[$$->sz-1]->node_type; i++)
															data_type_+="*";
														if(data_type_!="" && $2->node_data!="") data_type_+= " ";
														data_type_ += $2->node_data;
														$$->node_data = data_type_;
													}

	| direct_declarator								{
														$$ = $1;
													}
	;

direct_declarator
	: IDENTIFIER										{$$ = node_(0,$1,IDENTIFIER); $$->node_name = $1;}
	| '(' declarator ')'								{$$ = $2;/**/printf("\e[1;31mError [line %d]:\e[0m direct_declarator -> ( declarator ) reduction not handled.\n", line ); exit(-1);}
	| direct_declarator '[' constant_expression ']'		{
															$$ = node_(2,"[]",-1); $$->v[0] = $1; $$->v[1] = $3; $$->node_name = $1->node_name;
															$$->node_type = 2;
															long long tmp;
															switch($3->val_dt){
																case IS_INT: tmp = $3->val.int_const; break;
																case IS_LONG: tmp = $3->val.long_const; break;
																case IS_SHORT: tmp = $3->val.short_const; break;
																case IS_U_INT: tmp = $3->val.u_int_const; break;
																case IS_U_LONG: tmp = $3->val.u_long_const; break;
																case IS_U_SHORT: tmp = $3->val.u_short_const; break;
																default:
																	printf("\e[1;31mError [line %d]:\e[0m Incompatible data type for size in array declaration.\n", line);
																	exit(-1);
															}
															if(tmp<=0){
																printf("\e[1;31mError [line %d]:\e[0m Array size cannot be zero or negative.\n", line);
																exit(-1);
															}
															$$->node_data = $1->node_data;
															if($$->node_data.back() != ']' && $$->node_data != "")
																$$->node_data+=" ";
															$$->node_data += "[";
															$$->node_data += to_string(tmp);
															$$->node_data += "]";
														}
	| direct_declarator '[' ']'							{
															$$ = $1;
															$$->node_type = 2;
															if($$->node_data.back() != ']' && $$->node_data != "")
																$$->node_data+=" ";
															//else if($$->node_data!=""){
															//	printf("\e[1;31mError [line %d]:\e[0m Incorrect array format. \n", line );
															//	exit(-1);
															//}
															$$->node_data += "[]"; }
	| direct_declarator '(' {func_params.clear();}
	parameter_type_list ')'								{
															if($1->node_type == 1){
																printf("\e[1;31mError [line %d]:\e[0m Wrong declaration of function '%s'.\n", line , ($1->node_name).c_str());
																exit(-1);
															}
															$$ = node_(2,"()",-1); 
															$$->v[0] = $1, $$->v[1] = $4;
															$$->node_type = 1;
															$$->node_name = $1->node_name;
														}
	| direct_declarator '(' {func_params.clear();} identifier_list ')'			
														{
															if($1->node_type == 1){
																printf("\e[1;31mError [line %d]:\e[0m Wrong declaration of function '%s'.\n", line , ($1->node_name).c_str());
																exit(-1);
															}
															$$ = node_(2,"()",-1); 
															$$->v[0] = $1, $$->v[1] = $4; 
															$$->node_type = 1;
															$$->node_name = $1->node_name;
														}
	| direct_declarator '(' {func_params.clear();} ')'	{
															if($1->node_type == 1){
																printf("\e[1;31mError [line %d]:\e[0m Wrong declaration of function '%s'.\n", line , ($1->node_name).c_str()); //marked
																exit(-1);
															}
															$$ = node_(1,"()",-1);
															$$->v[0] = $1; 
															$$->node_type = 1;
															$$->node_name = $1->node_name;
														}
	;

pointer
	: '*'											{$$ = node_(1,"pointer",-1); $$->v[0] = node_(0,"*",-1); $$->node_type = 1;}
	| '*' type_qualifier_list						{$$ = node_(2,"pointer",-1); $$->v[0] = node_(0,"*",-1); $$->v[1] = $2;$$->node_type++;}
	| '*' pointer									{node* x = node_(0,"*",-1); $$ = $2; push_front($$, x); $$->node_type++;}
	| '*' type_qualifier_list pointer				{$$ = $3; node* x = node_(0,"*",-1); push_front($$, $2); push_front($$, x);$$->node_type++;}
	;

type_qualifier_list
	: type_qualifier								{$$ = node_(1, "type_qual",-1); $$->v[0] = $1;}
	| type_qualifier_list type_qualifier			{$$ = $1; add_node($$, $2);}
	;


parameter_type_list
	: parameter_list								{$$ = $1;}
/*	| parameter_list ',' ELLIPSIS					{$$ = $1; add_node($$,node_(0,$3,ELLIPSIS));}*/
	;

parameter_list
	: parameter_declaration							{$$ = node_(1,"param_list",-1); $$->v[0] = $1;}
	| parameter_list ',' parameter_declaration		{$$ = $1; add_node($$,$3);}
	;

parameter_declaration
	: declaration_specifiers declarator				{
														$1->node_data = get_eqtype($1->node_data);
														if($2->node_type == 1){
															printf("\e[1;31mError [line %d]:\e[0m Functions as parameters are not supported.\n", line );
															exit(-1);
														}
														$$ = $2;
														tt_entry * entry = type_lookup($1->node_data);
														string data_type_ = get_eqtype($1->node_data);
														if($2->node_data!=""){
															data_type_+=" ";
															data_type_+=$2->node_data;
														}
														if(data_type_.back()==']'){
															data_type_ = reduce_pointer_level(data_type_);
															check_valid_array(data_type_);
															data_type_ = increase_array_level(data_type_);
														}
														func_params.push_back({{data_type_,$2->node_name},entry});
														check_param_list(func_params);
													}
	| declaration_specifiers abstract_declarator	{
														$1->node_data = get_eqtype($1->node_data);
														$$ = $2;
														string data_type_ = get_eqtype($1->node_data);
														tt_entry * entry = type_lookup($1->node_data);
														if($2->node_data!=""){
															data_type_+=" ";
															data_type_+=$2->node_data;
														}
														if(data_type_.back()==']'){
															data_type_ = reduce_pointer_level(data_type_);
															check_valid_array(data_type_);
															data_type_ = increase_array_level(data_type_);
														}
														func_params.push_back({{data_type_,""}, entry});
														check_param_list(func_params);
														//$$ = $1;
														//if($2->node_data != ""){
														//	$$->node_data += " "+ $2->node_data;
														//} 
														//func_params.push_back({$$->node_data, ""});
													}
	| declaration_specifiers						{
														$$ = NULL;
														$1->node_data = get_eqtype($1->node_data);
														string data_type_ = get_eqtype($1->node_data);
														func_params.push_back({{data_type_,""},NULL});
													}
	;

identifier_list
	: IDENTIFIER									{
														$$ = node_(1,"id_list",-1); 
														$$->v[0] = node_(0,$1,IDENTIFIER);
														func_params.push_back({{"", $1},NULL});
													}
	| identifier_list ',' IDENTIFIER				{
														add_node($1,node_(0,$3,IDENTIFIER)); 
														$$ = $1;
														func_params.push_back({{"", $3},NULL});
													}
	;

type_name
	: specifier_qualifier_list						{
														$1->node_data = get_eqtype($1->node_data);
														$$ = $1;
													}
	| specifier_qualifier_list abstract_declarator	{
														$1->node_data = get_eqtype($1->node_data);
														if($2->node_data != ""){
															$$->node_data+=" ";
															$$->node_data+=$2->node_data;
														}
														$$ = $1; add_node($$,$2);
													}
	;

abstract_declarator
	: pointer										{
														$$ = $1;
														string data_type_ = "";
														for(int i = 0; i < $1->node_type; i++)
															data_type_+="*";
														$$->node_data = data_type_;
													}
	| direct_abstract_declarator					{$$ = $1;}
	| pointer direct_abstract_declarator			{
														$$ = node_(2,"abs_decl",-1); 
														$$->v[0] = $1; $$->v[1] = $2;
														//$$->node_data = $1->node_data+" "+$2->node_data;
														string data_type_ = "";
														for(int i = 0; i < $1->node_type; i++)
															data_type_+="*";
														if(data_type_!="" && $2->node_data!="") data_type_+= " ";
														data_type_ += $2->node_data;
														$$->node_data = data_type_;
													}
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'								{$$ = node_(1,"()",-1); $$->v[0]=$2;printf("\e[1;35mWarning [line %d]:\e[0m Abstract function declarations ignored.", line);}
	| '[' ']'													{
																	$$ = node_(0,"[]",-1);
																	$$->node_type = 2;
																	$$->node_data += "[";
																	$$->node_data += "]";
																}
	| '[' constant_expression ']'								{
																	$$ = node_(1,"[]",-1); $$->v[0] = $2;
																	$$->node_type = 2;
																	long long tmp;
																	switch($2->val_dt){
																		case IS_INT: tmp = $2->val.int_const; break;
																		case IS_LONG: tmp = $2->val.long_const; break;
																		case IS_SHORT: tmp = $2->val.short_const; break;
																		case IS_U_INT: tmp = $2->val.u_int_const; break;
																		case IS_U_LONG: tmp = $2->val.u_long_const; break;
																		case IS_U_SHORT: tmp = $2->val.u_short_const; break;
																		default:
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible data type for size in array declaration.\n", line);
																			exit(-1);
																	}
																	if(tmp<=0){
																		printf("\e[1;31mError [line %d]:\e[0m Array size cannot be zero or negative.\n", line);
																		exit(-1);
																	}
																	if($$->node_data.back() != ']' && $$->node_data != "")
																		$$->node_data+=" ";
																	$$->node_data += "[";
																	$$->node_data += to_string(tmp);
																	$$->node_data += "]";
																}
	| direct_abstract_declarator '[' ']'						{
																	$$ = $1;
																	$$->node_type = 2;
																	if($$->node_data.back() != ']' && $$->node_data != "")
																		$$->node_data+=" ";
																	$$->node_data += "[";
																	$$->node_data += "]";
																}
	| direct_abstract_declarator '[' constant_expression ']'	{
																	$$ = $1;
																	$$->node_type = 2;
																	long long tmp;
																	switch($3->val_dt){
																		case IS_INT: tmp = $3->val.int_const; break;
																		case IS_LONG: tmp = $3->val.long_const; break;
																		case IS_SHORT: tmp = $3->val.short_const; break;
																		case IS_U_INT: tmp = $3->val.u_int_const; break;
																		case IS_U_LONG: tmp = $3->val.u_long_const; break;
																		case IS_U_SHORT: tmp = $3->val.u_short_const; break;
																		default:
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible data type for size in array declaration.\n", line);
																			exit(-1);
																	}
																	if(tmp<=0){
																		printf("\e[1;31mError [line %d]:\e[0m Array size cannot be zero or negative.\n", line);
																		exit(-1);
																	}
																	if($$->node_data.back() != ']' && $$->node_data != "")
																		$$->node_data+=" ";
																	$$->node_data += "[";
																	$$->node_data += to_string(tmp);
																	$$->node_data += "]";
																}
	| '(' ')'													{$$ = node_(0,"()",-1);printf("\e[1;35mWarning [line %d]:\e[0m Abstract function declarations ignored.", line);}
	| '(' parameter_type_list ')'								{$$ = node_(0,"()",-1); /*$$->v[0] = $2;*/printf("\e[1;35mWarning [line %d]:\e[0m Abstract function declarations ignored.", line);}
	| direct_abstract_declarator '(' ')'						{$$ = $1; add_node($$, node_(0,"()",-1));printf("\e[1;35mWarning [line %d]:\e[0m Abstract function declarations ignored.", line);}
	| direct_abstract_declarator '(' parameter_type_list ')'	{$$ = $1; add_node($$, node_(0,"()",-1));/* $$->v[$$->sz-1]->v[0] = $3;*/printf("\e[1;35mWarning [line %d]:\e[0m Abstract function declarations ignored.", line);}
	;

initializer
	: assignment_expression										{$$ = $1;}
	| '{' initializer_list '}'									{
																	$$ = $2;
																	printf("\e[1;31mError [line %d]:\e[0m Array initialization not supported.\n", line);
																	exit(-1);
																}
	| '{' initializer_list ',' '}'								{
																	$$ = $2;
																	printf("\e[1;31mError [line %d]:\e[0m Array initialization not supported.\n", line);
																	exit(-1);
																}
	;

initializer_list
	: initializer												{$$ = node_(1,"init_list",-1); $$->v[0] = $1;}
	| initializer_list ',' initializer							{$$ = $1; add_node($$,$3);}
	;

statement
	: labeled_statement											{$$ = $1;}
	| compound_statement										{$$ = $1;}
	| expression_statement										{$$ = $1;}
	| selection_statement										{$$ = $1;}
	| iteration_statement										{$$ = $1;}
	| jump_statement											{$$ = $1;}
	;

labeled_statement
	: IDENTIFIER ':' statement									{$$ = node_(1,$1,-1); $$->v[0] = $3;}
	| CASE constant_expression ':' M9 statement					{
																	$$ = node_(2,$1,-1); $$->v[0] = $2; $$->v[1] = $5;
																	$$->caselist.push_back({$4, $2});
																	$$->nextlist = $5->nextlist;
																	$$->contlist = $5->contlist;
																	$$->breaklist = $5->breaklist;
																}
	| DEFAULT ':' M9 statement									{
																	$$ = node_(1,$1,-1); $$->v[0] = $4;
																	$$->defaultlist.push_back({$3, NULL});
																	$$->nextlist = $4->nextlist;
																	$$->contlist = $4->contlist;
																	$$->breaklist = $4->breaklist;
																}
	;

compound_statement
	: M1 '{' '}' M2												{$$ = node_(0,"{}",-1);}
	| M1 '{' statement_list '}' M2								{
																	$$ = $3;
																	// backpatch($3->nextlist, nextquad);
																}
																
	| M1 '{' declaration_list '}' M2							{
																	if($3 == NULL)
																		$$ = node_(0,"{}",-1);
																	else{
																		//$$ = node_(1,"{}",-1);
																		$$ = $3;
																	}
																}
																
	| M1 '{' declaration_list statement_list '}' M2				{
																	if($3 == NULL)
																		$$ = $4;
																	else{
																		$$ = node_(2,"{}",-1);
																		$$->v[0] = $3;
																		$$->v[1] = $4;
																		$$->nextlist = $4->nextlist;
																		$$->breaklist = $4->breaklist;
																		$$->contlist = $4->contlist;
																		$$->breaklist.insert($$->breaklist.end(), $3->breaklist.begin(), $3->breaklist.end());
																		$$->contlist.insert($$->contlist.end(), $3->contlist.begin(), $3->contlist.end());
																		
																		$$->caselist = $4->caselist;
																		$$->defaultlist = $4->defaultlist;
																	}
																	// backpatch($4->nextlist, nextquad);
																}
	;

M1
	: 	/* empty */	 											{	
																	new_scope();
																	check_param_list(func_params);
																	offset.push_back(0);
																	int int_char = 0, double_float = 0;
																	int curr_offset = -16;
																	for(auto p: func_params){
																		int flag = 0;
																		if(!is_struct_or_union(p.first.first) && (p.first.first.find("int") != string::npos || p.first.first.find("char") != string::npos || p.first.first.back() == ']' || p.first.first.back() == '*')){
																			flag = 1;
																		}
																		else if(!is_struct_or_union(p.first.first) && (p.first.first.find("double") != string::npos || p.first.first.find("float") != string::npos)){
																			flag = 2;
																		}
																		else{
																			flag = 3;
																		}
																		if(flag == 1 && int_char<6){
																			st_entry* st = add_entry(p.first.second, p.first.first, get_size(p.first.first), accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
																			int_char++;
																		}
																		else if(flag == 2 && double_float<8){
																			st_entry* st = add_entry(p.first.second, p.first.first, get_size(p.first.first), accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
																			double_float++;
																		}
																		else if(flag == 1 && int_char == 6){
																			st_entry* st = add_entry(p.first.second, p.first.first, 0, accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
																			st->size = get_size(p.first.first);
																			curr_offset -= (8-(-curr_offset)%8)%8;
																			st->offset = curr_offset;
																			curr_offset -= 8;
																		}
																		else if(flag == 2 && double_float == 8){
																			st_entry* st = add_entry(p.first.second, p.first.first, 0, accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
																			st->size = get_size(p.first.first);
																			if(p.first.first == "long double"){
																				curr_offset -= (16-(-curr_offset)%16)%16;
																				st->offset = curr_offset;
																				curr_offset-=16;
																			}
																			else{
																				curr_offset -= (8-(-curr_offset)%8)%8;
																				st->offset = curr_offset;
																				curr_offset -= 8;
																			}
																		}
																		else if(flag == 3){
																			st_entry* st = add_entry(p.first.second, p.first.first, 0, accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
																			st->ttentry = p.second;
																			st->size = get_size(p.first.first);
																			curr_offset -= (8-(-curr_offset)%8)%8;
																			st->offset = curr_offset;
																			curr_offset-=get_size(p.first.first);
																		}
																	}
																	func_params.clear();
																}
	;

M2
	: 	/* empty */												{
																	scope_cleanup();
																	long long temp = offset.back();
																	offset.pop_back();
																}
	;

declaration_list
	: declaration												{
																	if($1){
																		$$ = node_(1,"decl_list",-1); 
																		$$->v[0] = $1;
																	} 
																	else $$ = NULL;
																}
	| declaration_list declaration								{
																	if($2 == NULL) 
																		$$ = $1;
																	else if($1 == NULL){
																		$1 = node_(1,"decl_list",-1);
																		$1->v[0] = $2;
																		$$ = $1;
																	}
																	else{
																		add_node($1,$2);
																		$$ = $1;
																	}
																}
	;

statement_list
	: statement			{
							$$ = node_(1,"stmt_list",-1); $$->v[0] = $1;
							$$->nextlist = $1->nextlist;
							$$->breaklist = $1->breaklist;
							$$->contlist = $1->contlist;
							$$->caselist = $1->caselist;
							$$->defaultlist = $1->defaultlist;
						}
	| statement_list 	{
							backpatch($1->nextlist, nextquad);
						}
	statement			{
							$$ = $1; add_node($$,$3);
							$$->nextlist = $3->nextlist;
							$$->breaklist = $1->breaklist;
							$$->contlist = $1->contlist;
							$$->caselist = $1->caselist;
							$$->defaultlist = $1->defaultlist;
							$$->breaklist.insert($$->breaklist.end(), $3->breaklist.begin(), $3->breaklist.end());
							$$->contlist.insert($$->contlist.end(), $3->contlist.begin(), $3->contlist.end());
							$$->caselist.insert($$->caselist.end(), $3->caselist.begin(), $3->caselist.end());
							$$->defaultlist.insert($$->defaultlist.end(), $3->defaultlist.begin(), $3->defaultlist.end());
						}
	;

expression_statement
	: ';'														{$$ = node_(0,$1,';');}
	| expression ';'											{$$ = $1;}
	;

selection_statement
	: IF '(' expression ')' M8 statement				{
															$$ = node_(2,$1,-1); $$->v[0] = $3; $$->v[1] = $6;
															$$->nextlist = $3->falselist;
															$$->nextlist.insert($$->nextlist.end(), $6->nextlist.begin(), $6->nextlist.end());
															$$->breaklist = $6->breaklist;
															$$->contlist = $6->contlist;

														}

	| IF '(' expression ')' M8 statement ELSE 			{
															int x = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
															$6->nextlist.push_back(x);
															backpatch($3->falselist, nextquad);
														}
	statement			{
							$$ = node_(3, "if-else", -1); $$->v[0] = $3; $$->v[1] = $6; $$->v[2] = $9;
							$$->nextlist.insert($$->nextlist.end(), $6->nextlist.begin(), $6->nextlist.end());
							$$->nextlist.insert($$->nextlist.end(), $9->nextlist.begin(), $9->nextlist.end());
							$$->breaklist = $6->breaklist;
							$$->contlist = $6->contlist;
							$$->breaklist.insert($$->breaklist.end(), $9->breaklist.begin(), $9->breaklist.end());
							$$->contlist.insert($$->contlist.end(), $9->contlist.begin(), $9->contlist.end());
						}
	| SWITCH '(' expression ')' M11 /* {break_level++;} */ 
	statement			{
							$$ = node_(2, $1, -1); $$->v[0] = $3; $$->v[1] = $6; break_level--;


							if(is_struct_or_union($3->node_data) || $3->node_data.back()=='*' || $3->node_data.back()==']' || $3->node_data.find("float")!=string::npos || $3->node_data.find("double")!=string::npos){
								printf("\e[1;31mError [line %d]:\e[0m Switch quantity not an integer.\n", line);
								exit(-1);
							}

							if($6->defaultlist.size()>1){
								printf("\e[1;31mError [line %d]:\e[0m Multiple default statements in switch.\n", line);
								exit(-1);
							}


							qi tmp2 = getNewTemp("int");
							int x = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
							$$->nextlist.push_back(x);

							code_array[$5].goto_addr = nextquad;

							for(auto i : $6->caselist){
								pair<string,int> p1 = get_equivalent_pointer($3->node_data);
								pair<string,int> p2 = get_equivalent_pointer(i.second->node_data);
								string type = arithmetic_type_upgrade(p1.first,p2.first, "==",$3->ttentry,i.second->ttentry);
								qi tmp = emitConstant(i.second);
								if(type.find("int")!=string::npos || type.find("char")!=string::npos || type.back()=='*'){
									int x = emit("==int", $3->place, tmp, tmp2);
								}
								else {
									if($3->node_data.find("int")!=string::npos || $3->node_data.find("char")!=string::npos){
										auto tmp3 = getNewTemp(type);
										int x = emit("inttoreal", $3->place, {"", NULL}, tmp3);
										emit("==real", tmp3, tmp, tmp2);
									}
									else if (i.second->node_data.find("int")!=string::npos || i.second->node_data.find("char")!=string::npos){
										auto tmp3 = getNewTemp(type);
										int x = emit("inttoreal", tmp, {"", NULL}, tmp3);
										emit("==real", $3->place, tmp3, tmp2);
									}
									else {
										emit("==real", $3->place, tmp, tmp2);
									}
								}

								emit("IF_TRUE_GOTO", tmp2, {"", NULL}, {"", NULL}, i.first);
							}

							for(auto i : $6->defaultlist){
								emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL}, i.first);
							}

							$6->caselist.clear();
							$6->defaultlist.clear();
							
							$$->nextlist.insert($$->nextlist.end(), $6->nextlist.begin(), $6->nextlist.end());
							$$->nextlist.insert($$->nextlist.end(), $6->breaklist.begin(), $6->breaklist.end());
							$$->contlist = $6->contlist;
						}
	;

M11
	: /* empty */		{	
							break_level++;
							int x = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
							$$ = x;
						}

M8
	: /* empty */		{
							//////////////// 3AC ////////////////
							node* tmp = $<nodes>-1;
							if(tmp->token == CONSTANT){
								tmp->place = emitConstant(tmp);
								// printf("Ternary condition constant. %s %d\n", tmp->place.first.c_str(), tmp->val.int_const);
							}
							int x = emit("IF_TRUE_GOTO", tmp->place, {"", NULL}, {"", NULL});
							int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
							tmp->truelist.push_back(x);
							tmp->falselist.push_back(y);
							backpatch(tmp->truelist, nextquad);
							/////////////////////////////////////
						}

iteration_statement
	: WHILE '(' M9 expression ')' 			{
												break_level++, continue_level++;

												//////////////// 3AC ////////////////
												if($4->token == CONSTANT){
													$4->place = emitConstant($4);
												}

												int x = emit("IF_TRUE_GOTO", $4->place, {"", NULL}, {"", NULL});
												int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
												$4->truelist.push_back(x);
												$4->falselist.push_back(y);
												backpatch($4->truelist, nextquad);
												/////////////////////////////////////
											} 
	statement 								{
												break_level--, continue_level--;
												$$ = node_(2,$1,-1); $$->v[0] = $4; $$->v[1] = $7;

												//////////////// 3AC ////////////////
												backpatch($7->nextlist, $3);
												backpatch($7->contlist, $3);
												$$->nextlist = $4->falselist;
												$$->nextlist.insert($$->nextlist.end(), $7->breaklist.begin(), $7->breaklist.end());
												emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL}, $3);
												/////////////////////////////////////
											}
	
	| DO 							{break_level++, continue_level++;}
	M9 statement		 			{
										break_level--, continue_level--;
										
										//////////////// 3AC ////////////////
										backpatch($4->nextlist, nextquad);
										backpatch($4->contlist, nextquad);
										/////////////////////////////////////
									}
	WHILE '(' expression ')' ';'	{
										$$ = node_(2,"do-while",-1); $$->v[0] = $4; $$->v[1] = $8;

										//////////////// 3AC ////////////////
										if($4->token == CONSTANT){
											$4->place = emitConstant($4);
										}

										int x = emit("IF_TRUE_GOTO", $8->place, {"", NULL}, {"", NULL});
										// int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
										// $4->falselist.push_back(y);
										$4->truelist.push_back(x);
										backpatch($4->truelist, $3);
										$$->nextlist = $4->breaklist;
										$$->nextlist.insert($$->nextlist.end(), $8->falselist.begin(), $8->falselist.end());
										/////////////////////////////////////
									}

	| FOR '(' expression_statement 	M10
	M9 expression_statement ')' 	{
										break_level++, continue_level++;

										//////////////// 3AC ////////////////
										if($6->token == CONSTANT){
											$6->place = emitConstant($6);
										}

										int x = emit("IF_TRUE_GOTO", $6->place, {"", NULL}, {"", NULL});
										int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
										$6->truelist.push_back(x);
										$6->falselist.push_back(y);
										backpatch($6->truelist, nextquad);
										/////////////////////////////////////
									}
	statement 						{
										break_level--, continue_level--;
										$$ = node_(3,"for-w/o-update",-1); $$->v[0] = $3; $$->v[1] = $6; $$->v[2] = $9;

										//////////////// 3AC ////////////////
										backpatch($9->nextlist, $5);
										backpatch($9->contlist, $5);
										$$->nextlist = $6->falselist;
										$$->nextlist.insert($$->nextlist.end(), $9->breaklist.begin(), $9->breaklist.end());
										emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL}, $5);
										/////////////////////////////////////
									}

	| FOR '(' expression_statement 	M10
	M9 expression_statement 		{
										//////////////// 3AC ////////////////
										if($6->token == CONSTANT){
											$6->place = emitConstant($6);
										}

										int x = emit("IF_TRUE_GOTO", $6->place, {"", NULL}, {"", NULL});
										int y = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
										$6->truelist.push_back(x);
										$6->falselist.push_back(y);
										/////////////////////////////////////
									}
	M9 expression ')' 				{
										break_level++, continue_level++;

										//////////////// 3AC ////////////////
										backpatch($9->truelist, $5);
										backpatch($9->falselist, $5);
										emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL}, $5);
										backpatch($6->truelist, nextquad);
										/////////////////////////////////////
									}
	statement 						{
										break_level--, continue_level--;
										$$ = node_(4,"for",-1); $$->v[0] = $3; $$->v[1] = $6; $$->v[2] = $9; $$->v[3] = $12;

										//////////////// 3AC ////////////////
										backpatch($12->nextlist, $8);
										backpatch($12->contlist, $8);
										$$->nextlist = $6->falselist;
										$$->nextlist.insert($$->nextlist.end(), $12->breaklist.begin(), $12->breaklist.end());
										emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL}, $8);
										/////////////////////////////////////
									}
	;

M9
	: /* empty */			{
								$$ = nextquad;
							}

M10
	: /* empty */			{
								node* tmp = $<nodes>0;
								if(tmp->token == CONSTANT){
									tmp->place = emitConstant(tmp);
								}

								backpatch(tmp->truelist, nextquad);
								backpatch(tmp->falselist, nextquad);
								backpatch(tmp->nextlist, nextquad);
							}

jump_statement
	: GOTO IDENTIFIER ';'										{$$ = node_(1,$1,-1); $$->v[0] = node_(0,$2,IDENTIFIER);}
	| CONTINUE ';'												{
																	$$ = node_(0,$1,-1);
																	if(continue_level<=0){
																		printf("\e[1;31mError [line %d]:\e[0m Continue statement not within loop.\n", line);
																		exit(-1);
																	}

																	//////////////// 3AC ////////////////
																	int x = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
																	$$->contlist.push_back(x);
																	/////////////////////////////////////
																}
	| BREAK ';'													{
																	$$ = node_(0,$1,-1); 
																	if(break_level<=0){
																		printf("\e[1;31mError [line %d]:\e[0m Break statement not within loop or switch.\n", line);
																		exit(-1);
																	}

																	//////////////// 3AC ////////////////
																	int x = emit("GOTO", {"", NULL}, {"", NULL}, {"", NULL});
																	$$->breaklist.push_back(x);
																	/////////////////////////////////////
																}
	| RETURN ';'												{
																	$$ = node_(0,$1,-1);
																	emit("RETURN_VOID", {"", NULL}, {"", NULL}, {"", NULL});
																}
	| RETURN expression ';'										{
																	$$ = node_(1,$1,-1); $$->v[0] = $2;
																	if(func_ret_type == "void"){
																		printf("\e[1;35mWarning [line %d]:\e[0m ‘return’ with a value, in function returning void.\n", line);
																		emit("RETURN_VOID", {"", NULL}, {"", NULL}, {"", NULL});
																	}
																	else{
																		string tmp1 = func_ret_type, tmp2 = $2->node_data;
																		string type;
																		string type1, type2;
																		pair<string, int> p1 = get_equivalent_pointer(tmp1);
																		pair<string, int> p2 = get_equivalent_pointer(tmp2);
																		type1 = p1.first, type2 = p2.first;
																		tt_entry* entry1 = global_type_lookup(type1);
																		tt_entry* entry2 = NULL;
																		if($2->node_data.back()!='*'){
																			entry2 = $2->ttentry;
																		}
																		if(entry1 && entry2){
																			if(entry1 != entry2){
																				printf("\e[1;31mError [line %d]:\e[0m Incompatible types when returning type ‘%s’ but ‘%s’ was expected.\n",line, tmp2.c_str(), tmp1.c_str());
																				exit(-1);
																			}
																		}
																		if(entry1 || entry2){
																			printf("\e[1;31mError [line %d]:\e[0m Incompatible types when returning type ‘%s’ but ‘%s’ was expected.\n",line, tmp2.c_str(), tmp1.c_str());
																			exit(-1);
																		}
																		tt_entry* entry3 = NULL;
																		tt_entry* entry4 = NULL;
																		{
																			string type;
																			type = type1;
																			while(type.back()=='*' || type.back()==' ')
																				type.pop_back();
																			entry3 = global_type_lookup(type);

																			entry4 = $2->ttentry;

																		}
																		if(p1.second && p2.second){
																			if(p1.first != p2.first){
																				printf("\e[1;35mWarning [line %d]:\e[0m Returning ‘%s’ from a function with return type ‘%s’.\n",line, tmp2.c_str(), tmp1.c_str());
																			}
																		}
																		else if(p1.second){
																			if(tmp2 == "float" || tmp2 == "double" || tmp2 == "long double"){
																				printf("\e[1;31mError [line %d]:\e[0m Incompatible types when returning type ‘%s’ but ‘%s’ was expected.\n",line, tmp2.c_str(), tmp1.c_str());
																				exit(-1);
																			}
																			printf("\e[1;35mWarning [line %d]:\e[0m Returning ‘%s’ from a function with return type ‘%s’.\n",line, tmp2.c_str(), tmp1.c_str());
																		}
																		else if(p2.second){
																			if(tmp1 == "float" || tmp1 == "double" || tmp1 == "long double"){
																				printf("\e[1;31mError [line %d]:\e[0m Incompatible types when returning type ‘%s’ but ‘%s’ was expected.\n",line, tmp2.c_str(), tmp1.c_str());
																				exit(-1);
																			}
																			printf("\e[1;35mWarning [line %d]:\e[0m Returning ‘%s’ from a function with return type ‘%s’.\n",line, tmp2.c_str(), tmp1.c_str());	
																		}
																		if(tmp1 == tmp2 && entry3 != entry4){
																			printf("\e[1;35mWarning [line %d]:\e[0m Returning ‘%s’ from a function with return type ‘%s’.\n",line, tmp2.c_str(), tmp1.c_str());
																		}
																		qi tmp;
																		if($2->token == CONSTANT){
																			$2->place = emitConstant($2);
																		}
																		tmp = $2->place;
																		if(tmp1!=tmp2){
																			tmp = getNewTemp(tmp1, entry1);
																			string op = "("+tmp2+"-to-"+tmp1+")";
																			emit(op, $2->place, {"", NULL}, tmp);
																		}
																		emit("RETURN", tmp, {"", NULL}, {"", NULL});
																	}
																}
	;

augment_state
	: translation_unit											{$$ = $1; root = $$;}
	;

translation_unit
	: external_declaration										{if($1){$$ = node_(1,"tr_unit",-1); $$->v[0] = $1;} else $$ = NULL;}
	| translation_unit external_declaration						{if($2 == NULL) $$ = $1;
																else if($1 == NULL){
																	$1 = node_(1,"tr_unit",-1);
																	$1->v[0] = $2;
																	$$ = $1;
																}
																else{
																	add_node($1,$2);
																	$$ = $1;
																}}
	;

external_declaration
	: function_definition							{$$ = $1;}
	| declaration									{$$ = $1;}
	;

function_definition
	: declaration_specifiers M3 declarator {	
												curr_width = 0;
												$1->node_data = get_eqtype($1->node_data);
												if($3->node_data!=""){
													$1->node_data += " "+$3->node_data;
												}
												st_entry* tmp = lookup($3->node_name);
												for(auto p : func_params){
													if(p.first.second == ""){
														printf("\e[1;31mError [line %d]:\e[0m Parameter name omitted\n", line);
														exit(-1);
													}
												} 
												if(tmp!=NULL && tmp->type_name != IS_FUNC){
													printf("\e[1;31mError [line %d]:\e[0m Conflicting declarations of '%s'.\n", line ,($3->node_name).c_str());
													exit(-1);
												}
												if(tmp!=NULL && tmp->type_name == IS_FUNC && tmp->is_init==1){
													printf("\e[1;31mError [line %d]:\e[0m Conflicting definitions of '%s'.\n", line ,($3->node_name).c_str());
													exit(-1);
												}
												if(tmp!=NULL && tmp->type != ($1->node_data)){
													printf("\e[1;31mError [line %d]:\e[0m Conflicting definitions of '%s'.\n", line ,($3->node_name).c_str());
													exit(-1);
												}
												if(tmp!=NULL){
													if(func_params.size()!=tmp->arg_list->size()){
														printf("\e[1;31mError [line %d]:\e[0m Conflicting number of parameters in declaration and definition of '%s'.\n", line ,($3->node_name).c_str());
														exit(-1);
													}
													for(int i = 0; i < func_params.size(); i++){
														if(func_params[i].first.first == ""){
															func_params[i].first.first = (*(tmp->arg_list))[i].first.first; 
														}
														else if(func_params[i].first.first != (*(tmp->arg_list))[i].first.first){
															printf("\e[1;31mError [line %d]:\e[0m Conflicting parameter types in declaration and definition of '%s'.\n", line ,($3->node_name).c_str());	
															exit(-1);
														}
														(*(tmp->arg_list))[i].first.second = func_params[i].first.second;
													}
													tmp->is_init = 1;
													tmp->offset = offset.back();
													next_name = $1->node_name;
												}
												else{
													check_param_list(func_params);
													st_entry* func_entry = add_entry($3->node_name, ($1->node_data), 0, accumulate(offset.begin()+1, offset.end(), 0), IS_FUNC);
													if(!func_params.empty() && func_params[0].first.first == ""){
														printf("\e[1;35mWarning [line %d]:\e[0m Function parameter type defaults to \"int\"\n", line);
													}
													for(int i = 0; i < func_params.size(); i++){
														if(func_params[i].first.first == ""){
															func_params[i].first.first = "int";
														}
													}
													data_type = "";
													func_entry->type_name = IS_FUNC;
													func_entry->is_init = 1;										//added func_params
													vector<pair<pair<string, string>,tt_entry*>> *tmp = new vector<pair<pair<string, string>,tt_entry*>>(func_params);
													func_entry->arg_list = tmp;
													string tmpp = $1->node_data;
													while(tmpp.back()=='*' || tmpp.back()==' ')
														tmpp.pop_back();
													func_entry->ttentry = type_lookup(tmpp);
												}
												next_name = $3->node_name;
												func_ret_type = $1->node_data;
												emit("FUNC_START", {$3->node_name, lookup($3->node_name)}, {"", NULL}, {"", NULL}, var_no);
											} 

	compound_statement					{
											$$ = node_(2,"fun_def",-1); $$->v[0] = $3; $$->v[1] = $5;
											st_entry* tmp = lookup($3->node_name); 
											tmp->sym_table = curr->v.back()->val;
											tmp->size = curr_width;
											func_ret_type = "";

											backpatch($5->nextlist, nextquad);
											emit("FUNC_END", {"", NULL}, {"", NULL}, {"", NULL}, var_no);
										}


	| declarator 						{	
											string datatype = "int";
											if($1->node_data!=""){
												datatype += " "+$1->node_data;
											}
											curr_width = 0;
											for(auto p : func_params){
												if(p.first.second == ""){
													printf("\e[1;31mError [line %d]:\e[0m Parameter name omitted\n", line);
													exit(-1);
												}
											} 
											st_entry* tmp = lookup($1->node_name); 
											if(tmp != NULL && tmp->type_name != IS_FUNC){
												printf("\e[1;31mError [line %d]:\e[0m Conflicting declarations of '%s'.\n", line ,($1->node_name).c_str());
												exit(-1);
											}
											if(tmp != NULL && tmp->type_name == IS_FUNC && tmp->is_init==1){
												printf("\e[1;31mError [line %d]:\e[0m Conflicting definitions of '%s'.\n", line ,($1->node_name).c_str());
												exit(-1);
											}
											if(tmp!=NULL && tmp->type != ($1->node_data)){
												printf("\e[1;31mError [line %d]:\e[0m Conflicting definitions of '%s'.\n", line ,($1->node_name).c_str());
												exit(-1);
											}
											if(tmp!=NULL){
												if(func_params.size()!=tmp->arg_list->size()){
													printf("\e[1;31mError [line %d]:\e[0m Conflicting number of parameters in declaration and definition of '%s'.\n", line ,($1->node_name).c_str());
													exit(-1);
												}
												for(int i = 0; i < func_params.size(); i++){
													if(func_params[i].first.first == ""){
														func_params[i].first.first = (*(tmp->arg_list))[i].first.first; 
													}
													else if(func_params[i].first.first != (*(tmp->arg_list))[i].first.first){
														printf("\e[1;31mError [line %d]:\e[0m Conflicting parameter types in declaration and definition of '%s'.\n", line ,($1->node_name).c_str());	
														exit(-1);
													}
													(*(tmp->arg_list))[i].first.second = func_params[i].first.second;
												}
												tmp->is_init = 1;
												tmp->offset = offset.back();
												next_name = $1->node_name;
											}
											else{
												check_param_list(func_params);
												st_entry* func_entry = add_entry($1->node_name, datatype, 0, accumulate(offset.begin()+1, offset.end(), 0), IS_FUNC);
												printf("\e[1;35mWarning [line %d]:\e[0m Function return type defaults to \"int\"\n", line);
												if(!func_params.empty() && func_params[0].first.first == ""){
													printf("\e[1;35mWarning [line %d]:\e[0m Function parameter type defaults to \"int\"\n", line);
												}
												for(int i = 0; i < func_params.size(); i++){
													if(func_params[i].first.first == ""){
														func_params[i].first.first = "int";
													}
												}
												func_entry->type_name = IS_FUNC;
												func_entry->is_init = 1;										//added func_params
												vector<pair<pair<string, string>,tt_entry*>> *tmp = new vector<pair<pair<string, string>,tt_entry*>>(func_params);
												func_entry->arg_list = tmp;
												next_name = $1->node_name;
											}
											st_entry* temp = lookup($1->node_name);
											func_ret_type = temp->type; 

											emit("FUNC_START", {$1->node_name, lookup($1->node_name)}, {"", NULL}, {"", NULL}, var_no);
										}

	compound_statement					{
											$$ = node_(2,"fun_def",-1); $$->v[0] = $1; $$->v[1] = $3; 
											st_entry* tmp = lookup($1->node_name); 
											tmp->sym_table = curr->v.back()->val;
											tmp->size = curr_width;
											func_ret_type = "";

											backpatch($3->nextlist, nextquad);
											emit("FUNC_END", {"", NULL}, {"", NULL}, {"", NULL}, var_no);
										}
	| declaration_specifiers M3 declarator declaration_list compound_statement		{$$ = node_(1,$3->name,-1); $$->v[0] = $5;/*not used*/}
	| declarator declaration_list compound_statement							{$$ = node_(1,$1->name,-1); $$->v[0] = $3;/*not used*/}
	;

M3
	: /*empty*/										{$<nodes>0->node_data = get_eqtype($<nodes>0->node_data); data_type = $<nodes>0->node_data;}
