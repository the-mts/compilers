%{
	#include<string.h>
	#include<stdlib.h>
	#include "parse_utils.h"
	using namespace std;
	struct node* root;
	void yyerror(const char*s);
	extern int yylex();
	extern int yyparse();
	extern int line;
	int unnamed_var = 0;
	string data_type = "";
	string params = "";
	vector<pair<string, string>> func_params;
%}
%union {
	struct node* nodes;
	char * id;
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

%type<id> M6
%type <nodes> M1

%start augment_state
%define parse.error verbose
%%

primary_expression
	: IDENTIFIER 												{
																	$$ = node_(0,$1,IDENTIFIER);
																	st_entry * entry = lookup(string((const char *)$1));
																	if(entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Undeclared identifier %s\n",line, $1);
																		exit(-1);
																	}
																	$$->node_name = $1;
																	$$->node_data = entry->type;
																	$$->value_type = LVALUE;
																}
	| CONSTANT 													{
																	$$ = node_(0,$1,CONSTANT);
																	pair<constant, enum const_type> parsed = parse_constant(string((const char*)$1));
																	switch(parsed.second){
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
																	}
																	$$->val = parsed.first;
																	$$->value_type = RVALUE;
																}
	| STRING_LITERAL											{$$ = node_(0,$1,STRING_LITERAL); $$->node_data = "char ["+to_string(strlen($1)+1)+"]";}
	| '(' expression ')'										{$$ = $2;}
	;

postfix_expression
	: primary_expression										{$$ = $1;}
	| postfix_expression '[' expression ']'						{
																	$$ = node_(2, "[]", -1);
																	$$->v[0] = $1; $$->v[1] = $3;
																	pair<string,int> p = get_equivalent_pointer($1->node_data);
																	if(p.second == 0){
																		printf("\e[1;31mError [line %d]:\e[0m Subscripted value is neither array nor pointer.\n",line);
																		exit(-1);
																	}
																	$$->node_data = reduce_pointer_level($1->node_data);
																}
	| postfix_expression '(' ')'								{
																	$$ = node_(1, "()", -1);
																	$$->v[0] = $1;
																	if($1->token != IDENTIFIER){
																		printf("\e[1;31mError [line %d]:\e[0m Invalid function call\n",line);
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
																	if(entry->is_init != 1){
																		printf("\e[1;31mError [line %d]:\e[0m Function '%s' declared but not defined.\n",line,$1->name);
																		exit(-1);
																	}
																	if((int)entry->arg_list->size() != 0){
																		printf("\e[1;31mError [line %d]:\e[0m Function '%s' needs %d arguments but 0 provided.\n",line,$1->name,(int)entry->arg_list->size());
																		exit(-1);
																	}
																	$$->node_data = entry->type;
																	$$->value_type = RVALUE;
																}
	| postfix_expression '(' argument_expression_list ')'		{
																	$$ = node_(2, "(args)", -1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	if($1->token != IDENTIFIER){
																		printf("\e[1;31mError [line %d]:\e[0m Invalid function call\n",line);
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
																	if(entry->is_init != 1){
																		printf("\e[1;31mError [line %d]:\e[0m Function '%s' declared but not defined.\n",line,$1->name);
																		exit(-1);
																	}
																	if((int)entry->arg_list->size() != $3->sz){
																		printf("\e[1;31mError [line %d]:\e[0m Function '%s' needs %d arguments but %d provided.\n",line,$1->name,(int)entry->arg_list->size(),$3->sz);
																		exit(-1);
																	}
																	auto arg_list = *(entry->arg_list);
																	for(int i = 0; i < $3->sz; i++){
																		string type;
																		if($3->v[i]->node_data.back() == ']')
																			type = increase_array_level(reduce_pointer_level($3->v[i]->node_data));
																		else
																			type = $3->v[i]->node_data;
																		if(type != arg_list[i].first){
																			printf("\e[1;31mError [line %d]:\e[0m For function '%s', argument %d should be of type %s, %s provided.\n",line,$1->name,i+1,arg_list[i].first.c_str(),type.c_str());
																			exit(-1);
																		}
																	}
																	$$->node_data = entry->type;
																	$$->value_type = RVALUE;
																}
	| postfix_expression '.' IDENTIFIER							{
																	$$ = node_(2,".",'.');
																	$$->v[0] = $1;
																	$$->v[1] = node_(0,$3,IDENTIFIER);
																	tt_entry * type_entry = type_lookup($1->node_data);
																	if(type_entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m '.' operator applied on non-struct or non-union type.\n",line);
																		exit(-1);
																	}
																	/*while(type_entry->is_typedef){
																		type_entry = type_lookup(type_entry->type);
																	}*/
																	int flag = 0;
																	string name = string((const char*)$3);
																	string type;
																	for(auto it : *(type_entry->mem_list))
																		if(it.second == name){
																			type = it.first;
																			flag = 1;
																			break;
																		}
																	if(!flag){
																		printf("\e[1;31mError [line %d]:\e[0m (%s) has no member named (%s).\n",line, $1->node_data.c_str(),$3);/*typedef changes*/
																		exit(-1);
																	}
																	$$->node_data = type;
																	if($1->value_type == RVALUE)
																		$$->value_type = RVALUE;
																	else
																		$$->value_type = LVALUE;
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
																	tt_entry * type_entry = type_lookup(type);
																	if(type_entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m '->' operator applied on non-struct or non-union pointer type.\n",line);
																		exit(-1);
																	}
																	/*while(type_entry->is_typedef){
																		type_entry = type_lookup(type_entry->type);
																	}*/
																	int flag = 0;
																	string type1 = type;
																	string name = string((const char*)$3);
																	for(auto it : *(type_entry->mem_list))
																		if(it.second == name){
																			type = it.first;
																			flag = 1;
																			break;
																		}
																	if(!flag){
																		printf("\e[1;31mError [line %d]:\e[0m (%s) has no member named (%s).\n",line, type1.c_str(),$3);/*typedef changes*/
																		exit(-1);
																	}
																	$$->node_data = type;
																	if($1->value_type == RVALUE)
																		$$->value_type = RVALUE;
																	else
																		$$->value_type = LVALUE;
																}
	| postfix_expression INC_OP									{
																	$$ = node_(1, "exp++", -1);
																	$$->v[0] = $1;
																	$$->node_data = $1->node_data;
																	tt_entry* type_entry = type_lookup($1->node_data);
																	if(type_entry != NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Increment operator cannot be apllied on non-integer, non-floating point and non pointer types.\n",line);
																		exit(-1);
																	}/*array*/
																	if($1->value_type == RVALUE){
																		printf("\e[1;31mError [line %d]:\e[0m Increment operator cannot be apllied on rvalue.\n",line);
																		exit(-1);
																	}
																	$$->value_type = RVALUE;
																}
	| postfix_expression DEC_OP									{
																	$$ = node_(1, "exp--", -1); $$->v[0] = $1;
																	$$->node_data = $1->node_data;
																	tt_entry* type_entry = type_lookup($1->node_data);
																	if(type_entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Decrement operator cannot be apllied on non-integer, non-floating point and non pointer types.\n",line);
																		exit(-1);
																	}/*array*/
																	if($1->value_type == RVALUE){
																		printf("\e[1;31mError [line %d]:\e[0m Decrement operator cannot be apllied on rvalue.\n",line);
																		exit(-1);
																	}
																	$$->value_type = RVALUE;
																}
	;

argument_expression_list
	: assignment_expression										{$$ = node_(1,"arg_list",-1); $$->v[0] = $1;}
	| argument_expression_list ',' assignment_expression		{$$ = $1; add_node($$,$3);}
	;

unary_expression
	: postfix_expression										{$$ = $1;}
	| INC_OP unary_expression									{
																	$$ = node_(1,"++exp",-1);
																	$$->v[0] = $2;
																	$$->node_data = $2->node_data;
																	tt_entry* type_entry = type_lookup($2->node_data);
																	if(type_entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Increment operator cannot be apllied on non-integer, non-floating point and non pointer types.\n",line);
																		exit(-1);
																	}/*array*/
																	if($2->value_type == RVALUE){
																		printf("\e[1;31mError [line %d]:\e[0m Increment operator cannot be apllied on rvalue.\n",line);
																		exit(-1);
																	}
																	$$->value_type = RVALUE;
																}
	| DEC_OP unary_expression									{
																	$$ = node_(1,"++exp",-1);
																	$$->v[0] = $2;
																	$$->node_data = $2->node_data;
																	tt_entry* type_entry = type_lookup($2->node_data);
																	if(type_entry == NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Decrement operator cannot be apllied on non-integer, non-floating point and non pointer types.\n",line);
																		exit(-1);
																	}/*array*/
																	if($2->value_type == RVALUE){
																		printf("\e[1;31mError [line %d]:\e[0m Decrement operator cannot be apllied on rvalue.\n",line);
																		exit(-1);
																	}
																	$$->value_type = RVALUE;
																}
	| unary_operator cast_expression							{
																	$$ = node_(1,$1,*($1));
																	$$->v[0] = $2;
																	string temp_data = get_equivalent_pointer($2->node_data).first;
																	switch(*($1)){
																		case '&':{
																			if($2->value_type == LVALUE){
																				$$->value_type = RVALUE;
																				if(temp_data.back() == '*'){
																					$$->node_data = temp_data+'*';
																				}
																				else{
																					$$->node_data = temp_data + " *";
																				}
																			}
																			else{
																				printf("\e[1;31mError [line %d]:\e[0m Address-of operator cannot be applied on rvalue.\n",line);
																				exit(-1);
																			}
																		}
																		break;
																		case '*':{
																			if(temp_data.back() != '*'){
																				printf("\e[1;31mError [line %d]:\e[0m Indirection operator cannot be applied on non-pointer type.\n",line);
																				exit(-1);
																			}
																			else{
																				$$->node_data = reduce_pointer_level($2->node_data);
																				//$$->node_data.pop_back();
																				$$->value_type = LVALUE;
																			}
																		}
																		break;
																		case '!':{
																			if(temp_data.back() == '*'){
																				$$->value_type = RVALUE;
																				$$->node_data = "int";
																				break;
																			}
																		}
																		case '-':
																		case '+':{
																			if(temp_data.substr((int)temp_data.size() - 3,3) != "int"){
																				if(temp_data != "float" && temp_data != "double" && temp_data != "long double"){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible type for %c operator.\n",line,*($1));
																					exit(-1);
																				}
																			}
																			$$->node_data = temp_data;
																			$$->value_type = RVALUE;
																		}
																		break;
																		case '~':{
																			if(temp_data.substr((int)temp_data.size() - 3,3) != "int"){
																				printf("\e[1;31mError [line %d]:\e[0m ~ cannot be applied to non-integer types.\n",line);
																				exit(-1);
																			}
																			$$->node_data = temp_data;
																			$$->value_type = RVALUE;
																		}
																	}

																}
	| SIZEOF unary_expression									{
																	long unsigned sz = get_size($2->node_data);
																	$$ = node_(0,(char*)to_string(sz).c_str(),CONSTANT);
																	$$->node_data = "unsigned long int";
																	$$->val_dt = IS_U_LONG;
																	$$->val.u_long_const = sz;
																}
	| SIZEOF '(' type_name ')'									{
																	long unsigned sz = get_size($3->node_data);
																	$$ = node_(0,(char*)to_string(sz).c_str(),CONSTANT);
																	$$->node_data = "unsigned long int";
																	$$->val_dt = IS_U_LONG;
																	$$->val.u_long_const = sz;
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
																}
	;

multiplicative_expression
	: cast_expression											{$$ = $1;}
	| multiplicative_expression '*' cast_expression				{
																	$$ = node_(2,"*",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	| multiplicative_expression '/' cast_expression				{
																	$$ = node_(2,"/",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	| multiplicative_expression '%' cast_expression				{
																	$$ = node_(2,"%",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") != string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

additive_expression
	: multiplicative_expression									{$$ = $1;}
	| additive_expression '+' multiplicative_expression			{
																	$$ = node_(2,"+",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	| additive_expression '-' multiplicative_expression			{
																	$$ = node_(2,"-",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

shift_expression
	: additive_expression										{$$ = $1;}
	| shift_expression LEFT_OP additive_expression				{
																	$$ = node_(2,"<<",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") != string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	$$->node_data = $1->node_data;
																	$$->value_type = RVALUE;
																}
	| shift_expression RIGHT_OP additive_expression				{
																	$$ = node_(2,">>",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") != string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	$$->node_data = $1->node_data;
																	$$->value_type = RVALUE;
																}
	;

relational_expression
	: shift_expression											{$$ = $1;}
	| relational_expression '<' shift_expression				{
																	$$ = node_(2,"<",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(!(p1.second || p2.second) || (p1.second && p2.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	| relational_expression '>' shift_expression				{
																	$$ = node_(2,">",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(!(p1.second || p2.second) || (p1.second && p2.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	| relational_expression LE_OP shift_expression				{
																	$$ = node_(2,"<=",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(!(p1.second || p2.second) || (p1.second && p2.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	| relational_expression GE_OP shift_expression				{
																	$$ = node_(2,">=",-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(!(p1.second || p2.second) || (p1.second && p2.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	;

equality_expression
	: relational_expression										{$$ = $1;}
	| equality_expression EQ_OP relational_expression			{
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(!(p1.second || p2.second) || (p1.second && p2.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	| equality_expression NE_OP relational_expression			{
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(!(p1.second || p2.second) || (p1.second && p2.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	;

and_expression
	: equality_expression										{$$ = $1;}
	| and_expression '&' equality_expression					{
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") != string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

exclusive_or_expression
	: and_expression											{$$ = $1;}
	| exclusive_or_expression '^' and_expression				{
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") != string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

inclusive_or_expression
	: exclusive_or_expression									{$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression		{
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	string type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2));
																	if(type.back() == '*'){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	if(type.find("int") != string::npos){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	$$->node_data = type;
																	$$->value_type = RVALUE;
																}
	;

logical_and_expression
	: inclusive_or_expression									{$$ = $1;}
	| logical_and_expression AND_OP inclusive_or_expression		{
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(!(p1.second || p2.second) || (p1.second && p2.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	arithmetic_type_upgrade(p1.first,p2.first, string((const char*)$2));
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	;

logical_or_expression
	: logical_and_expression									{$$ = $1;}
	| logical_or_expression OR_OP logical_and_expression		{
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																	pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																	if(!(p1.second || p2.second) || (p1.second && p2.second)){
																		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																		exit(-1);
																	}
																	arithmetic_type_upgrade(p1.first,p2.first,string((const char*)$2));
																	$$->node_data = "int";
																	$$->value_type = RVALUE;
																}
	;

conditional_expression
	: logical_or_expression												{$$ = $1;}
	| logical_or_expression '?' expression ':' conditional_expression	{
																			$$ = node_(3,"ternary",-1);
																			$$->v[0] = $1;
																			$$->v[1] = $3;
																			$$->v[2] = $5;
																			pair<string,int> p1 = get_equivalent_pointer($1->node_data);
																			pair<string,int> p2 = get_equivalent_pointer($3->node_data);
																			pair<string,int> p3 = get_equivalent_pointer($5->node_data);
																			if(p1.second != 0){
																				arithmetic_type_upgrade(p1.first,"int", "ternary");
																			}
																			if(!(p3.second || p2.second) || (p3.second && p2.second)){
																				printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2);
																				exit(-1);
																			}
																			if(p3.second && p2.second){
																				$$->node_data = "void *";
																			}
																			else if(p2.first == p3.first){
																				$$->node_data = p3.first;
																			}else{
																				$$->node_data = arithmetic_type_upgrade(p1.first,p2.first, "ternary");
																			}
																			$$->value_type = RVALUE;
																		}
	;

assignment_expression
	: conditional_expression										{$$ = $1;}
	| unary_expression assignment_operator assignment_expression	{
																		$$ = $2;
																		add_node($$, $1);
																		add_node($$, $3);
																		if($1->value_type == RVALUE){
																			printf("\e[1;31mError [line %d]:\e[0m lvalue required as left operand of %s.\n",line, $2->name);
																			exit(-1);
																		}
																		string type;
																		switch($2->token){
																			case '=':
																			break;
																			case MUL_ASSIGN:
																			case DIV_ASSIGN:
																				type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2->name));
																				if(type.back() == '*'){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2->name);
																					exit(-1);
																				}
																			break;
																			case ADD_ASSIGN:
																			case SUB_ASSIGN:
																				type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2->name));
																			break;
																			case MOD_ASSIGN:
																			case LEFT_ASSIGN:
																			case RIGHT_ASSIGN:
																			case AND_ASSIGN:
																			case XOR_ASSIGN:
																			case OR_ASSIGN:{
																				type = arithmetic_type_upgrade(get_equivalent_pointer($1->node_data).first,get_equivalent_pointer($3->node_data).first, string((const char*)$2->name));
																				if(type.back() == '*'){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2->name);
																					exit(-1);
																				}
																				if(type.find("int") != string::npos){
																					printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, $2->name);
																					exit(-1);
																				}
																			}
																		}
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
	: assignment_expression										{$$ = $1;}
	| expression ',' assignment_expression						{
																	$$ = node_(2,$2,-1);
																	$$->v[0] = $1;
																	$$->v[1] = $3;
																	$$->node_data = $3->node_data;
																	$$->value_type = $3->value_type;
																	$$->val = $3->val;
																	$$->val_dt = $3->val_dt;
																	$$->token = $3->token;
																}
	;

constant_expression
	: conditional_expression									{$$ = $1;/*check if constant*/}
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
																	if(tmp2=="struct"){
																		tt_entry * entry = current_type_lookup(tmp);
																		if(entry==NULL){
																			add_type_entry(tmp,tmp2);
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
																	$$->node_data+=$1->node_data;
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
	: init_declarator											{if($1){$$ = node_(1,"init_list",-1); $$->v[0] = $1;}}
	| init_declarator_list ',' init_declarator					{if($3 == NULL) $$ = $1;
																else if($1 == NULL){
																	$1 = node_(1,"init_list",-1);
																	$1->v[0] = $3;
																	$$ = $1;
																}
																else{
																	add_node($1,$3);
																	$$ = $1;
																}}
	;

init_declarator
	: declarator												{
																	string data_type_ = data_type;
																	// string cmp;
																	// if($1->sz)
																	// 	cmp = $1->v[$1->sz-1]->name;
																	// else
																	// 	cmp = "";
																	// if( cmp == "pointer"){
																	// 	data_type_+=" ";
																	// 	for(int i = 0; i < $1->v[$1->sz-1]->node_type; i++)
																	// 		data_type_+="*";
																	// }
																	if($1->node_data!=""){
																		data_type_ += " ";
																		data_type_ += $1->node_data;
																	}
																	if(current_lookup($1->node_name)!=NULL){
																		printf("\e[1;31mError [line %d]:\e[0m Redeclaration of %s\n", line, $1->node_name.c_str());
																		exit(-1);
																	}
																	check_valid_array(data_type_);
																	st_entry* tmp = add_entry($1->node_name, data_type_,0,0);
																	if($1->node_type == 1){
																		tmp->type_name = IS_FUNC;
																		check_param_list(func_params);
																		vector<pair<string, string>> *temp = new vector<pair<string, string>>(func_params);
																		tmp->arg_list = temp;
																	}
																	else
																		tmp->type_name = IS_VAR;
																	$$ = NULL; free($1);
																}
	| declarator '=' initializer								{
																	$$ = node_(2,"=",-1); $$->v[0] = $1; $$->v[1] = $3;
																	if($1->node_type == 1){
																		printf("Funtion pointers not supported\n");
																		exit(-1);
																	}
																	else{
																		if(current_lookup($1->node_name)!=NULL){
																			printf("\e[1;31mError [line %d]:\e[0m Redeclaration of %s\n", line, $1->node_name.c_str());
																			exit(-1);
																		}
																		st_entry* tmp = add_entry($1->name, data_type, 0, 0, IS_VAR);/*change IS_VAR*/
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
	| TYPE_NAME													{}
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER M4 '{' struct_declaration_list M5 '}'
																	{	
																		char* ch = (char*)malloc(sizeof(char)*(strlen($1)+strlen($2)+2)); 
																		strcpy(ch,$1); strcat(ch," "); strcat(ch,$2);
																		$$ = node_(1,ch,-1); free(ch);
																		$$->v[0] = $5;
																		$$->node_data = string((const char*)$1) + " " + string((const char*)$2);
																	}	

	| struct_or_union M6 M4 '{' struct_declaration_list M5 '}'		{
																		$$ = node_(1,$1,-1); $$->v[0]=$5;
																		$$->node_data = string((const char*)$1) + " " + string((const char*)$2);
																	}

	| struct_or_union IDENTIFIER 									{
																		char* ch = (char*)malloc(sizeof(char)*(strlen($1)+strlen($2)+2)); 
																		strcpy(ch,$1); strcat(ch," "); strcat(ch,$2);
																		$$ = node_(0,ch,-1); free(ch);
																		$$->node_data = string((const char*)$1)+" "+string((const char*)$2);
																	}
	;

M4
	:	/* empty */													{
																		string temp1((const char*)$<id>0);
																		string temp2((const char*)$<id>-1);
																		tt_entry* tmp2 = current_type_lookup(temp2+" "+temp1); 
																		if(tmp2==NULL)
																			tt_entry* tmp = add_type_entry(temp2+" "+temp1, temp2);
																		else if(tmp2->is_init == 1){
																			printf("\e[1;31mError [line %d]:\e[0m Redefinition of %s\n", line, (temp2+" "+temp1).c_str());
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
																					vector<pair<string, string>> *tmp =  new vector<pair<string, string>>;
																					struct_entry->mem_list = tmp;
																				}
																				struct_init_check(type);
																				struct_entry->mem_list->push_back({type, name});
																			}
																		}
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
	: specifier_qualifier_list struct_declarator_list ';'		{
																	$1->node_data = get_eqtype($1->node_data);
																	$$ = node_(2,"struct_decl",-1); $$->v[0] = $1; $$->v[1] = $2;
																}
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list		{
														$$ = $2; push_front($$,$1);
														if($$->node_data != "")
															$$->node_data = " " + $$->node_data;
														$$->node_data = $1->node_data + $$->node_data;
													}

	| type_specifier								{
														$$ = node_(1,"type_list",-1); $$->v[0] = $1;
														$$->node_data = $1->node_data;
													}

	| type_qualifier specifier_qualifier_list		{$$ = $2; push_front($$, $1); /* Not propagating in node_data */}
	| type_qualifier								{$$ = node_(1,"type_list",-1); $$->v[0] = $1; /* Not propagating in node_data */}
	;

struct_declarator_list
	: struct_declarator								{$$ = node_(1,"struct_decl_list",-1); $$->v[0]=$1;}
	| struct_declarator_list ',' struct_declarator	{add_node($1,$3); $$ = $1;}
	;

struct_declarator
	: declarator 									{$$ = $1;}
	| ':' constant_expression						{$$ = node_(2,"bit_field",-1); $$->v[0]=node_(0,$1,-1); $$->v[1]=$2;}
	| declarator ':' constant_expression			{$$ = node_(3,"bit_field",-1); $$->v[0]=$1; $$->v[1]=node_(0,$2,-1); $$->v[2]=$3;}
	;

enum_specifier
	: ENUM '{' enumerator_list '}'					{$$ = node_(0,"enum",-1);}
	| ENUM IDENTIFIER '{' enumerator_list '}'		{char * ch = (char*)malloc((strlen($1)+strlen($2)+2)*sizeof(char));
													strcpy(ch,$1), strcat(ch," "), strcat(ch,$2);
													$$ = node_(0,ch,-1);}
	| ENUM IDENTIFIER								{char * ch = (char*)malloc((strlen($1)+strlen($2)+2)*sizeof(char));
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
	: CONST											{$$ = node_(0,$1,CONST);}
	| VOLATILE										{$$ = node_(0,$1,VOLATILE);}
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
	| '(' declarator ')'								{$$ = $2;/**/printf("\e[1;31mError [line %d]:\e[0m Direct_declarator -> ( declarator ) reduce.\n", line ); exit(-1);}
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
																printf("\e[1;31mError [line %d]:\e[0m Wrong declaration of function %s \n", line , ($1->node_name).c_str());
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
																printf("\e[1;31mError [line %d]:\e[0m Wrong declaration of function %s \n", line , ($1->node_name).c_str());
																exit(-1);
															}
															$$ = node_(2,"()",-1); 
															$$->v[0] = $1, $$->v[1] = $4; 
															$$->node_type = 1;
															$$->node_name = $1->node_name;
														}
	| direct_declarator '(' {func_params.clear();} ')'	{
															if($1->node_type == 1){
																printf("\e[1;31mError [line %d]:\e[0m Wrong declaration of function %s %s\n", line , ($1->node_name).c_str(), ($1->name));
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
														func_params.push_back({data_type_,$2->node_name});
													}
	| declaration_specifiers abstract_declarator	{$$ = NULL; /*not handled*/}
	| declaration_specifiers						{
														$$ = NULL;
														$1->node_data = get_eqtype($1->node_data);
														string data_type_ = get_eqtype($1->node_data);
														func_params.push_back({data_type_,""});
													}
	;

identifier_list
	: IDENTIFIER									{
														$$ = node_(1,"id_list",-1); 
														$$->v[0] = node_(0,$1,IDENTIFIER);
														func_params.push_back({"", $1});
													}
	| identifier_list ',' IDENTIFIER				{
														add_node($1,node_(0,$3,IDENTIFIER)); 
														$$ = $1;
														func_params.push_back({"", $3});
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
	: pointer										{$$ = $1;}
	| direct_abstract_declarator					{$$ = $1;}
	| pointer direct_abstract_declarator			{$$ = node_(2,"abs_decl",-1); $$->v[0] = $1; $$->v[1] = $2;$$->node_data = $1->node_data+" "+$2->node_data;}
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
																	$$->node_data += to_string(tmp);
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
/*	| '{' initializer_list '}'									{$$ = $2;}
	| '{' initializer_list ',' '}'								{$$ = $2;}*//*Not Supported*/
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
	| CASE constant_expression ':' statement					{$$ = node_(2,$1,-1); $$->v[0] = $2; $$->v[1] = $4;}
	| DEFAULT ':' statement										{$$ = node_(1,$1,-1); $$->v[0] = $3;}
	;

compound_statement
	: M1 '{' '}' M2												{$$ = node_(0,"{}",-1);}
	| M1 '{' statement_list '}' M2								{$$ = $3;}
	| M1 '{' declaration_list '}' M2							{if($3 == NULL)
																	$$ = node_(0,"{}",-1);
																else{
																	//$$ = node_(1,"{}",-1);
																	$$ = $3;
																}}
	| M1 '{' declaration_list statement_list '}' M2				{if($3 == NULL)
																	$$ = $4;
																else{
																	$$ = node_(2,"{}",-1);
																	$$->v[0] = $3;
																	$$->v[1] = $4;
																}}
	;

M1
	: 	/* empty */	 											{	
																	new_scope();
																	check_param_list(func_params);
																	for(auto p: func_params){
																		add_entry(p.second, p.first, 0, 0, IS_VAR);    //IS_VAR to be changed
																	}
																	func_params.clear();
																}
	;

M2
	: 	/* empty */												{
																	scope_cleanup();
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
	: statement													{$$ = node_(1,"stmt_list",-1); $$->v[0] = $1;}
	| statement_list statement									{$$ = $1; add_node($$,$2);}
	;

expression_statement
	: ';'														{$$ = node_(0,$1,';');}
	| expression ';'											{$$ = $1;}
	;

selection_statement
	: IF '(' expression ')' statement							{$$ = node_(2,$1,-1); $$->v[0] = $3; $$->v[1] = $5;}
	| IF '(' expression ')' statement ELSE statement			{$$ = node_(3, "if-else", -1); $$->v[0] = $3; $$->v[1] = $5; $$->v[2] = $7;}
	| SWITCH '(' expression ')' statement						{$$ = node_(2, $1, -1); $$->v[0] = $3; $$->v[1] = $5;}
	;

iteration_statement
	: WHILE '(' expression ')' statement											{$$ = node_(2,$1,-1); $$->v[0] = $3; $$->v[1] = $5;}
	| DO statement WHILE '(' expression ')' ';'										{$$ = node_(2,"do-while",-1); $$->v[0] = $2; $$->v[1] = $5;}
	| FOR '(' expression_statement expression_statement ')' statement				{$$ = node_(3,"for-w/o-update",-1); $$->v[0] = $3; $$->v[1] = $4; $$->v[2] = $6;}
	| FOR '(' expression_statement expression_statement expression ')' statement	{$$ = node_(4,"for",-1); $$->v[0] = $3; $$->v[1] = $4; $$->v[2] = $5; $$->v[3] = $7;}
	;

jump_statement
	: GOTO IDENTIFIER ';'										{$$ = node_(1,$1,-1); $$->v[0] = node_(0,$2,IDENTIFIER);}
	| CONTINUE ';'												{$$ = node_(0,$1,-1);}
	| BREAK ';'													{$$ = node_(0,$1,-1);}
	| RETURN ';'												{$$ = node_(0,$1,-1);}
	| RETURN expression ';'										{$$ = node_(1,$1,-1); $$->v[0] = $2;}
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
												$1->node_data = get_eqtype($1->node_data);
												st_entry* tmp = lookup($3->node_name);
												for(auto p : func_params){
													if(p.second == ""){
														printf("\e[1;31mError [line %d]:\e[0m Parameter name omitted\n", line);
														exit(-1);
													}
												} 
												if(tmp!=NULL && tmp->type_name != IS_FUNC){
													printf("\e[1;31mError [line %d]:\e[0m Conflicting declarations of %s\n", line ,($3->node_name).c_str());
													exit(-1);
												}
												if(tmp!=NULL && tmp->type_name == IS_FUNC && tmp->is_init==1){
													printf("\e[1;31mError [line %d]:\e[0m Conflicting definitions of %s\n", line ,($3->node_name).c_str());
													exit(-1);
												}
												if(tmp!=NULL && tmp->type != get_eqtype($1->node_data)){
													printf("\e[1;31mError [line %d]:\e[0m Conflicting definitions of %s\n", line ,($3->node_name).c_str());
													exit(-1);
												}
												if(tmp!=NULL){
													if(func_params.size()!=tmp->arg_list->size()){
														printf("\e[1;31mError [line %d]:\e[0m Conflicting number of parameters in declaration and definition of %s\n", line ,($3->node_name).c_str());
														exit(-1);
													}
													for(int i = 0; i < func_params.size(); i++){
														if(func_params[i].first == ""){
															func_params[i].first = (*(tmp->arg_list))[i].first; 
														}
														else if(func_params[i].first != (*(tmp->arg_list))[i].first){
															printf("\e[1;31mError [line %d]:\e[0m Conflicting parameter types in declaration and definition of %s\n", line ,($3->node_name).c_str());	
															exit(-1);
														}
														(*(tmp->arg_list))[i].second = func_params[i].second;
													}
													tmp->is_init = 1;
												}
												else{
													check_param_list(func_params);
													st_entry* func_entry = add_entry($3->node_name, get_eqtype($1->node_data), 0, 0);
													if(!func_params.empty() && func_params[0].first == ""){
														printf("\e[1;35mWarning [line %d]:\e[0m Function parameter type defaults to \"int\"\n", line);
													}
													for(int i = 0; i < func_params.size(); i++){
														if(func_params[i].first == ""){
															func_params[i].first = "int";
														}
													}
													data_type = "";
													func_entry->type_name = IS_FUNC;
													func_entry->is_init = 1;										//added func_params
													vector<pair<string, string>> *tmp = new vector<pair<string, string>>(func_params);
													func_entry->arg_list = tmp;
												}
											} 

	compound_statement					{
											$$ = node_(2,"fun_def",-1); $$->v[0] = $3; $$->v[1] = $5;
											st_entry* tmp = lookup($3->node_name); 
											tmp->sym_table = curr->v.back()->val;
										}

	| declarator 						{

											for(auto p : func_params){
												if(p.second == ""){
													printf("\e[1;31mError [line %d]:\e[0m Parameter name omitted\n", line);
													exit(-1);
												}
											} 
											st_entry* tmp = lookup($1->node_name); 
											if(tmp != NULL && tmp->type_name != IS_FUNC){
												printf("\e[1;31mError [line %d]:\e[0m Conflicting declarations of %s\n", line ,($1->node_name).c_str());
												exit(-1);
											}
											if(tmp != NULL && tmp->type_name == IS_FUNC && tmp->is_init==1){
												printf("\e[1;31mError [line %d]:\e[0m Conflicting definitions of %s\n", line ,($1->node_name).c_str());
												exit(-1);
											}
											if(tmp!=NULL){
												if(func_params.size()!=tmp->arg_list->size()){
													printf("\e[1;31mError [line %d]:\e[0m Conflicting number of parameters in declaration and definition of %s\n", line ,($1->node_name).c_str());
													exit(-1);
												}
												for(int i = 0; i < func_params.size(); i++){
													if(func_params[i].first == ""){
														func_params[i].first = (*(tmp->arg_list))[i].first; 
													}
													else if(func_params[i].first != (*(tmp->arg_list))[i].first){
														printf("\e[1;31mError [line %d]:\e[0m Conflicting parameter types in declaration and definition of %s\n", line ,($1->node_name).c_str());	
														exit(-1);
													}
													(*(tmp->arg_list))[i].second = func_params[i].second;
												}
												tmp->is_init = 1;
											}
											else{
												check_param_list(func_params);
												st_entry* func_entry = add_entry($1->node_name, "int", 0, 0);
												printf("\e[1;35mWarning [line %d]:\e[0m Function return type defaults to \"int\"\n", line);
												if(!func_params.empty() && func_params[0].first == ""){
													printf("\e[1;35mWarning [line %d]:\e[0m Function parameter type defaults to \"int\"\n", line);
												}
												for(int i = 0; i < func_params.size(); i++){
													if(func_params[i].first == ""){
														func_params[i].first = "int";
													}
												}
												func_entry->type_name = IS_FUNC;
												func_entry->is_init = 1;										//added func_params
												vector<pair<string, string>> *tmp = new vector<pair<string, string>>(func_params);
												func_entry->arg_list = tmp;
											}
										}

	compound_statement					{
											$$ = node_(2,"fun_def",-1); $$->v[0] = $1; $$->v[1] = $3; 
											st_entry* tmp = lookup($1->node_name); 
											tmp->sym_table = curr->v.back()->val;
										}
	| declaration_specifiers M3 declarator declaration_list compound_statement		{$$ = node_(1,$3->name,-1); $$->v[0] = $5;/*not used*/}
	| declarator declaration_list compound_statement							{$$ = node_(1,$1->name,-1); $$->v[0] = $3;/*not used*/}
	;

M3
	: /*empty*/										{$<nodes>0->node_data = get_eqtype($<nodes>0->node_data); data_type = $<nodes>0->node_data;}