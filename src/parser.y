%{
	#include<string.h>
	#include<stdlib.h>
	#include "parse_utils.h"
	struct node* root;
	void yyerror(char*s);
	extern int yylex();
	extern int yyparse();
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

%type <id> unary_operator assignment_operator storage_class_specifier type_specifier struct_or_union
%type <nodes> primary_expression postfix_expression argument_expression_list unary_expression 
%type <nodes> cast_expression multiplicative_expression additive_expression shift_expression relational_expression 
%type <nodes> equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression 
%type <nodes> logical_or_expression conditional_expression assignment_expression expression 
%type <nodes> constant_expression declaration declaration_specifiers init_declarator_list init_declarator 
%type <nodes> struct_or_union_specifier struct_declaration_list 
%type <nodes> struct_declaration specifier_qualifier_list struct_declarator_list struct_declarator enum_specifier 
%type <nodes> enumerator_list enumerator type_qualifier declarator direct_declarator 
%type <nodes> pointer type_qualifier_list parameter_type_list parameter_list parameter_declaration 
%type <nodes> identifier_list type_name abstract_declarator direct_abstract_declarator initializer 
%type <nodes> initializer_list statement labeled_statement compound_statement declaration_list 
%type <nodes> statement_list expression_statement selection_statement iteration_statement jump_statement 
%type <nodes> translation_unit external_declaration function_definition augment_state

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
	| INC_OP unary_expression									{$$ = node_(1,"++arg",-1); $$->v[0] = $2;}
	| DEC_OP unary_expression									{$$ = node_(1,"--arg",-1); $$->v[0] = $2;}
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
	: VOID														{}
	| CHAR														{}
	| SHORT														{}
	| INT														{}
	| LONG														{}
	| FLOAT														{}
	| DOUBLE													{}
	| SIGNED													{}
	| UNSIGNED													{}
	| struct_or_union_specifier									{}
	| enum_specifier											{}
	| TYPE_NAME													{}
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'	{}	
	| struct_or_union '{' struct_declaration_list '}'				{}
	| struct_or_union IDENTIFIER									{}
	;

struct_or_union
	: STRUCT													{}
	| UNION														{}
	;

struct_declaration_list
	: struct_declaration										{}
	| struct_declaration_list struct_declaration				{}
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'		{}
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list		{}
	| type_specifier								{}
	| type_qualifier specifier_qualifier_list		{}
	| type_qualifier								{}
	;

struct_declarator_list
	: struct_declarator								{}
	| struct_declarator_list ',' struct_declarator	{}
	;

struct_declarator
	: declarator
	| ':' constant_expression						{}
	| declarator ':' constant_expression
	;

enum_specifier
	: ENUM '{' enumerator_list '}'					{/*Enumerators in AST?*/}
	| ENUM IDENTIFIER '{' enumerator_list '}'		{}
	| ENUM IDENTIFIER								{}
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
	: CONST											{}
	| VOLATILE										{}
	;

declarator
	: pointer direct_declarator						{$$ = $2;}
	| direct_declarator								{$$ = $1;}
	;

direct_declarator
	: IDENTIFIER										{$$ = node_(0,$1,IDENTIFIER);}
	| '(' declarator ')'								{$$ = $2;}
	| direct_declarator '[' constant_expression ']'		{$$ = $1;}
	| direct_declarator '[' ']'							{$$ = $1;}
	| direct_declarator '(' parameter_type_list ')'		{$$ = $1;}
	| direct_declarator '(' identifier_list ')'			{$$ = $1;}
	| direct_declarator '(' ')'							{$$ = $1;}
	;

pointer
	: '*'											{}
	| '*' type_qualifier_list						{}
	| '*' pointer									{}
	| '*' type_qualifier_list pointer				{}
	;

type_qualifier_list
	: type_qualifier								{}
	| type_qualifier_list type_qualifier			{}
	;


parameter_type_list
	: parameter_list								{}
	| parameter_list ',' ELLIPSIS					{}
	;

parameter_list
	: parameter_declaration							{}
	| parameter_list ',' parameter_declaration		{}
	;

parameter_declaration
	: declaration_specifiers declarator				{}
	| declaration_specifiers abstract_declarator	{}
	| declaration_specifiers						{}
	;

identifier_list
	: IDENTIFIER									{}
	| identifier_list ',' IDENTIFIER				{}
	;

type_name
	: specifier_qualifier_list						{}
	| specifier_qualifier_list abstract_declarator	{}
	;

abstract_declarator
	: pointer										{}
	| direct_abstract_declarator					{}
	| pointer direct_abstract_declarator			{}
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'								{}
	| '[' ']'													{}
	| '[' constant_expression ']'								{}
	| direct_abstract_declarator '[' ']'						{}
	| direct_abstract_declarator '[' constant_expression ']'	{}
	| '(' ')'													{}
	| '(' parameter_type_list ')'								{}
	| direct_abstract_declarator '(' ')'						{}
	| direct_abstract_declarator '(' parameter_type_list ')'	{}
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
	: '{' '}'													{$$ = node_(0,"{}",-1);}
	| '{' statement_list '}'									{$$ = $2;}
	| '{' declaration_list '}'									{if($2 == NULL)
																	$$ = node_(0,"{}",-1);
																else{
																	$$ = node_(1,"{}",-1);
																	$$->v[0] = $2;
																}}
	| '{' declaration_list statement_list '}'					{if($2 == NULL)
																	$$ = $3;
																else{
																	$$ = node_(2,"{}",-1);
																	$$->v[0] = $2;
																	$$->v[1] = $3;
																}}
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
	: declaration_specifiers declarator declaration_list compound_statement		{$$ = $4;}
	| declaration_specifiers declarator compound_statement						{$$ = $3;}
	| declarator declaration_list compound_statement							{$$ = $3;}
	| declarator compound_statement												{$$ = $2;}
	;
