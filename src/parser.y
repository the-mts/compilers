%{
	#include<string.h>
	#include<stdlib.h>
	#include "parse_utils.h"
	#include "symtab.h"
	using namespace std;
	struct node* root;
	void yyerror(char*s);
	extern int yylex();
	extern int yyparse();
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

%type <id> unary_operator assignment_operator storage_class_specifier struct_or_union
%type <nodes> primary_expression postfix_expression argument_expression_list unary_expression type_specifier 
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


%type <nodes> M1

%start augment_state
%%

primary_expression
	: IDENTIFIER 												{$$ = node_(0,$1,IDENTIFIER);}
	| CONSTANT 													{$$ = node_(0,$1,CONSTANT);}
	| STRING_LITERAL											{$$ = node_(0,$1,STRING_LITERAL);}
	| '(' expression ')'										{$$ = $2;}
	;

postfix_expression
	: primary_expression										{$$ = $1;}
	| postfix_expression '[' expression ']'						{$$ = node_(2, "[]", -1); $$->v[0] = $1; $$->v[1] = $3;}
	| postfix_expression '(' ')'								{$$ = node_(1, "()", -1); $$->v[0] = $1;}
	| postfix_expression '(' argument_expression_list ')'		{$$ = node_(2, "(args)", -1); $$->v[0] = $1; $$->v[1] = $3;}
	| postfix_expression '.' IDENTIFIER							{$$ = node_(2,".",-1); $$->v[0] = $1; $$->v[1] = node_(0,$3,IDENTIFIER);}
	| postfix_expression PTR_OP IDENTIFIER 						{$$ = node_(2,"->",-1); $$->v[0] = $1; $$->v[1] = node_(0,$3,IDENTIFIER);}
	| postfix_expression INC_OP									{$$ = node_(1, "exp++", -1); $$->v[0] = $1;}
	| postfix_expression DEC_OP									{$$ = node_(1, "exp--", -1); $$->v[0] = $1;}
	;

argument_expression_list
	: assignment_expression										{$$ = node_(1,"arg_list",-1); $$->v[0] = $1;}
	| argument_expression_list ',' assignment_expression		{$$ = $1; add_node($$,$3);}
	;

unary_expression
	: postfix_expression										{$$ = $1;}
	| INC_OP unary_expression									{$$ = node_(1,"++exp",-1); $$->v[0] = $2;}
	| DEC_OP unary_expression									{$$ = node_(1,"--exp",-1); $$->v[0] = $2;}
	| unary_operator cast_expression							{$$ = node_(1,$1,-1); $$->v[0] = $2;}
	| SIZEOF unary_expression									{$$ = node_(1,"SIZEOF_unary",-1); $$->v[0] = $2;}
	| SIZEOF '(' type_name ')'									{$$ = node_(1,"SIZEOF_type",-1); $$->v[0] = $3;}
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
	| '(' type_name ')' cast_expression							{$$ = node_(2,"()_typecast",-1); $$->v[0] = $2; $$->v[1] = $4;}
	;

multiplicative_expression
	: cast_expression											{$$ = $1;}
	| multiplicative_expression '*' cast_expression				{$$ = node_(2,"*",-1); $$->v[0] = $1; $$->v[1] = $3;}
	| multiplicative_expression '/' cast_expression				{$$ = node_(2,"/",-1); $$->v[0] = $1; $$->v[1] = $3;}
	| multiplicative_expression '%' cast_expression				{$$ = node_(2,"%",-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

additive_expression
	: multiplicative_expression									{$$ = $1;}
	| additive_expression '+' multiplicative_expression			{$$ = node_(2,"+",-1); $$->v[0] = $1; $$->v[1] = $3;}
	| additive_expression '-' multiplicative_expression			{$$ = node_(2,"-",-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

shift_expression
	: additive_expression										{$$ = $1;}
	| shift_expression LEFT_OP additive_expression				{$$ = node_(2,"<<",-1); $$->v[0] = $1; $$->v[1] = $3;}
	| shift_expression RIGHT_OP additive_expression				{$$ = node_(2,">>",-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

relational_expression
	: shift_expression											{$$ = $1;}
	| relational_expression '<' shift_expression				{$$ = node_(2,"<",-1); $$->v[0] = $1; $$->v[1] = $3;}
	| relational_expression '>' shift_expression				{$$ = node_(2,">",-1); $$->v[0] = $1; $$->v[1] = $3;}
	| relational_expression LE_OP shift_expression				{$$ = node_(2,"<=",-1); $$->v[0] = $1; $$->v[1] = $3;}
	| relational_expression GE_OP shift_expression				{$$ = node_(2,">=",-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

equality_expression
	: relational_expression										{$$ = $1;}
	| equality_expression EQ_OP relational_expression			{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	| equality_expression NE_OP relational_expression			{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

and_expression
	: equality_expression										{$$ = $1;}
	| and_expression '&' equality_expression					{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

exclusive_or_expression
	: and_expression											{$$ = $1;}
	| exclusive_or_expression '^' and_expression				{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

inclusive_or_expression
	: exclusive_or_expression									{$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression		{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

logical_and_expression
	: inclusive_or_expression									{$$ = $1;}
	| logical_and_expression AND_OP inclusive_or_expression		{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

logical_or_expression
	: logical_and_expression									{$$ = $1;}
	| logical_or_expression OR_OP logical_and_expression		{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

conditional_expression
	: logical_or_expression												{$$ = $1;}
	| logical_or_expression '?' expression ':' conditional_expression	{$$ = node_(3,"ternary",-1); $$->v[0] = $1; $$->v[1] = $3; $$->v[2] = $5;}
	;

assignment_expression
	: conditional_expression										{$$ = $1;}
	| unary_expression assignment_operator assignment_expression	{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

assignment_operator
	: '='														{$$ = $1;}
	| MUL_ASSIGN												{$$ = $1;}
	| DIV_ASSIGN												{$$ = $1;}
	| MOD_ASSIGN												{$$ = $1;}
	| ADD_ASSIGN												{$$ = $1;}
	| SUB_ASSIGN												{$$ = $1;}
	| LEFT_ASSIGN												{$$ = $1;}
	| RIGHT_ASSIGN												{$$ = $1;}
	| AND_ASSIGN												{$$ = $1;}
	| XOR_ASSIGN												{$$ = $1;}
	| OR_ASSIGN													{$$ = $1;}
	;

expression
	: assignment_expression										{$$ = $1;}
	| expression ',' assignment_expression						{$$ = node_(2,$2,-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

constant_expression
	: conditional_expression									{$$ = $1;}
	;

declaration
	: declaration_specifiers ';'								{$$ = NULL;}
	| declaration_specifiers init_declarator_list ';'			{$$ = $2;}
	;

declaration_specifiers
	: storage_class_specifier									{}
	| storage_class_specifier declaration_specifiers			{}
	| type_specifier											{}
	| type_specifier declaration_specifiers						{}
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
	: declarator												{$$ = NULL; free($1);}
	| declarator '=' initializer								{$$ = node_(2,"=",-1); $$->v[0] = $1; $$->v[1] = $3;}
	;

storage_class_specifier
	: TYPEDEF													{}
	| EXTERN													{}
	| STATIC													{}
	| AUTO														{}
	| REGISTER													{}
	;

type_specifier
	: VOID														{$$ = node_(0,$1,-1);}
	| CHAR														{$$ = node_(0,$1,-1);}
	| SHORT														{$$ = node_(0,$1,-1);}
	| INT														{$$ = node_(0,$1,-1);}
	| LONG														{$$ = node_(0,$1,-1);}
	| FLOAT														{$$ = node_(0,$1,-1);}
	| DOUBLE													{$$ = node_(0,$1,-1);}
	| SIGNED													{$$ = node_(0,$1,-1);}
	| UNSIGNED													{$$ = node_(0,$1,-1);}
	| struct_or_union_specifier									{$$ = $1;}
	| enum_specifier											{$$ = $1;}
	| TYPE_NAME													{}
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'	{char* ch = (char*)malloc(sizeof(char)*(strlen($1)+strlen($2)+2)); 
																	strcpy(ch,$1); strcat(ch," "); strcat(ch,$2);
																	$$ = node_(1,ch,-1); free(ch);
																	$$->v[0] = $4;}	
	| struct_or_union '{' struct_declaration_list '}'				{$$ = node_(1,$1,-1); $$->v[0]=$3;}
	| struct_or_union IDENTIFIER									{char* ch = (char*)malloc(sizeof(char)*(strlen($1)+strlen($2)+2)); 
																	strcpy(ch,$1); strcat(ch," "); strcat(ch,$2);
																	$$ = node_(0,ch,-1); free(ch);}
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
	: specifier_qualifier_list struct_declarator_list ';'		{$$ = node_(2,"struct_decl",-1); $$->v[0] = $1; $$->v[1] = $2;}
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list		{if($2 == NULL)	{$$ = node_(1,"type_list",-1); $$->v[0] = $1;}
													else{
														$$ = $2; push_front($$,$1);
													}}
	| type_specifier								{$$ = node_(1,"type_list",-1); $$->v[0] = $1;}
	| type_qualifier specifier_qualifier_list		{$$ = $2; push_front($$, $1);}
	| type_qualifier								{$$ = node_(1,"type_list",-1); $$->v[0] = $1;}
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

declarator
	: pointer direct_declarator						{/*$$ = $2; char *ch = (char*)malloc(sizeof(char)*(strlen($1)+strlen($2->name)+2));
													strcpy(ch,$1); strcat(ch, $$->name);
													char* tmp = $$->name; $$->name = ch; free(tmp);*/
													$$ = $2;}
	| direct_declarator								{$$ = $1;}
	;

direct_declarator
	: IDENTIFIER										{$$ = node_(0,$1,IDENTIFIER); $$->node_name = $1;}
	| '(' declarator ')'								{$$ = $2;}
	| direct_declarator '[' constant_expression ']'		{$$ = node_(2,"[]",-1); $$->v[0] = $1; $$->v[1] = $3;}
	| direct_declarator '[' ']'							{$$ = $1;}
	| direct_declarator '(' {func_params.clear()}
	parameter_type_list ')'								{
															if($$->node_type == 1){
																printf("Wrong declaration of function %s \n", ($1->node_name).c_str());
																exit(-1);
															}
															$$ = node_(2,"()",-1); 
															$$->v[0] = $1, $$->v[1] = $4;
															$$->node_type = 1;
														}
	| direct_declarator '(' identifier_list ')'			{$$ = node_(2,"()",-1); $$->v[0] = $1, $$->v[1] = $3;}
	| direct_declarator '(' ')'							{$$ = $1;}
	;

pointer
	: '*'											{$$ = node_(1,"pointer",-1); $$->v[0] = node_(0,"*",-1);}
	| '*' type_qualifier_list						{$$ = node_(2,"pointer",-1); $$->v[0] = node_(0,"*",-1); $$->v[1] = $2;}
	| '*' pointer									{node* x = node_(0,"*",-1); $$ = $2; push_front($$, x);}
	| '*' type_qualifier_list pointer				{$$ = $3; node* x = node_(0,"*",-1); push_front($$, $2); push_front($$, x);}
	;

type_qualifier_list
	: type_qualifier								{$$ = node_(1, "type_qual",-1); $$->v[0] = $1;}
	| type_qualifier_list type_qualifier			{$$ = $1; add_node($$, $2);}
	;


parameter_type_list
	: parameter_list								{$$ = $1;}
	| parameter_list ',' ELLIPSIS					{$$ = $1; add_node($$,node_(0,$3,ELLIPSIS));}
	;

parameter_list
	: parameter_declaration							{$$ = node_(1,"param_list",-1); $$->v[0] = $1;}
	| parameter_list ',' parameter_declaration		{$$ = $1; add_node($$,$3);}
	;

parameter_declaration
	: declaration_specifiers declarator				{$$ = $2;}
	| declaration_specifiers abstract_declarator	{$$ = NULL;}
	| declaration_specifiers						{$$ = NULL;}
	;

identifier_list
	: IDENTIFIER									{$$ = node_(1,"id_list",-1); $$->v[0] = node_(0,$1,IDENTIFIER);}
	| identifier_list ',' IDENTIFIER				{add_node($1,node_(0,$3,IDENTIFIER)); $$ = $1;}
	;

type_name
	: specifier_qualifier_list						{$$ = $1;}
	| specifier_qualifier_list abstract_declarator	{$$ = $1; add_node($$,$2);}
	;

abstract_declarator
	: pointer										{$$ = $1;}
	| direct_abstract_declarator					{$$ = $1;}
	| pointer direct_abstract_declarator			{$$ = node_(2,"abs_decl",-1); $$->v[0] = $1; $$->v[1] = $2;}
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'								{$$ = node_(1,"()",-1); $$->v[0]=$2;}
	| '[' ']'													{$$ = node_(0,"[]",-1);}
	| '[' constant_expression ']'								{$$ = node_(1,"[]",-1); $$->v[0]=$2;}
	| direct_abstract_declarator '[' ']'						{$$ = $1; add_node($$, node_(0,"[]",-1));}
	| direct_abstract_declarator '[' constant_expression ']'	{$$ = $1; node* x = node_(1,"[]",-1); x->v[0]=$3; add_node($$,x);}
	| '(' ')'													{$$ = node_(0,"()",-1);}
	| '(' parameter_type_list ')'								{$$ = node_(0,"()",-1); /*$$->v[0] = $2;*/}
	| direct_abstract_declarator '(' ')'						{$$ = $1; add_node($$, node_(0,"()",-1));}
	| direct_abstract_declarator '(' parameter_type_list ')'	{$$ = $1; add_node($$, node_(0,"()",-1));/* $$->v[$$->sz-1]->v[0] = $3;*/}
	;

initializer
	: assignment_expression										{$$ = $1;}
	| '{' initializer_list '}'									{$$ = $2;}
	| '{' initializer_list ',' '}'								{$$ = $2;}
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
																	for(auto p: func_params){
																		st_entry* tmp = add_entry(p.second, p.first, 0, 0, IS_VAR);    //IS_VAR to be changed
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
	: declaration												{if($1){$$ = node_(1,"decl_list",-1); $$->v[0] = $1;} else $$ = NULL;}
	| declaration_list declaration								{if($2 == NULL) $$ = $1;
																else if($1 == NULL){
																	$1 = node_(1,"decl_list",-1);
																	$1->v[0] = $2;
																	$$ = $1;
																}
																else{
																	add_node($1,$2);
																	$$ = $1;
																}}
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
	: declaration_specifiers declarator {	
											st_entry* tmp = lookup($2->node_name); 
											if(tmp!=NULL && tmp->type_name != IS_FUNC){
												printf("Conflicting declarations of %s\n",($2->node_name).c_str());
												exit(-1);
											}
											if(tmp!=NULL && tmp->type_name == IS_FUNC && tmp->is_init==1){
												printf("Conflicting definitions of %s\n",($2->node_name).c_str());
												exit(-1);
											}
											if(tmp!=NULL && tmp->type != $1->node_data){
												printf("Conflicting definitions of %s\n",($2->node_name).c_str());
												exit(-1);
											}
											if(tmp!=NULL){
												if(func_params.size()!=tmp->arg_list->size()){
													printf("Conflicting number of parameters in declaration and definition of %s\n",($2->node_name).c_str());
													exit(-1);
												}
												for(int i = 0; i < func_params.size(); i++){
													if(func_params[i].first != (*(tmp->arg_list))[i].first){
														printf("Conflicting parameter types in declaration and definition of %s\n",($2->node_name).c_str());	
														exit(-1);
													}
													(*(tmp->arg_list))[i].second = func_params[i].second;
												}
											}
											else{
												st_entry* func_entry = add_entry($2->node_name, $1->node_data, 0, 0);
												func_entry->type_name = IS_FUNC;
												func_entry->is_init = 1;										//added func_params
												vector<pair<string, string>> tmp = func_params;
												func_entry->arg_list = &tmp;
											}
										} 

	compound_statement					{
											$$ = node_(2,"fun_def",-1); $$->v[0] = $2; $$->v[1] = $4;
											st_entry* tmp = lookup($2->node_name); 
											tmp->sym_table = curr->v.back()->val;
										}

	| declarator 						{
											st_entry* tmp = lookup($1->node_name); 
											if(tmp != NULL && tmp->type_name != IS_FUNC){
												printf("Conflicting declarations of %s\n",($1->node_name).c_str());
												exit(-1);
											}
											if(tmp != NULL && tmp->type_name == IS_FUNC && tmp->is_init==1){
												printf("Conflicting definitions of %s\n",($1->node_name).c_str());
												exit(-1);
											}
											if(tmp!=NULL){
												if(func_params.size()!=tmp->arg_list->size()){
													printf("Conflicting number of parameters in declaration and definition of %s\n",($1->node_name).c_str());
													exit(-1);
												}
												for(int i = 0; i < func_params.size(); i++){
													if(func_params[i].first != (*(tmp->arg_list))[i].first){
														printf("Conflicting parameter types in declaration and definition of %s\n",($1->node_name).c_str());	
														exit(-1);
													}
													(*(tmp->arg_list))[i].second = func_params[i].second;
												}
											}
											else{
												st_entry* func_entry = add_entry($1->node_name, "int", 0, 0);
												func_entry->type_name = IS_FUNC;
												func_entry->is_init = 1;										//added func_params
												vector<pair<string, string>> tmp = func_params;
												func_entry->arg_list = &tmp;
											}
										}

	compound_statement					{
											$$ = node_(2,"fun_def",-1); $$->v[0] = $1; $$->v[1] = $3; 
											st_entry* tmp = lookup($1->node_name); 
											tmp->sym_table = curr->v.back()->val;
										}
	| declaration_specifiers declarator declaration_list compound_statement		{$$ = node_(1,$2->name,-1); $$->v[0] = $4;/*not used*/}
	| declarator declaration_list compound_statement							{$$ = node_(1,$1->name,-1); $$->v[0] = $3;/*not used*/}
	;

