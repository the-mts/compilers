%{
	#include<iostream>
	#include<string>
	#include<vector>
	using namespace std;
	struct node{
		vector<node*> v;
		string name;
		int token;
		node(int size, char* s, int token = -1){
			v.resize(size);
			this.token = token;
			this.name = string(s);
		}
	};
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

%token <id> unary_operator assignment_operator storage_class_specifier type_specifier struct_or_union
%token <nodes> primary_expression postfix_expression argument_expression_list unary_expression 
%token <nodes> cast_expression multiplicative_expression additive_expression shift_expression relational_expression 
%token <nodes> equality_expression and_expression exclusive_or_expression inclusive_or_expression logical_and_expression 
%token <nodes> logical_or_expression conditional_expression assignment_expression expression 
%token <nodes> constant_expression declaration declaration_specifiers init_declarator_list init_declarator 
%token <nodes> struct_or_union_specifier struct_declaration_list 
%token <nodes> struct_declaration specifier_qualifier_list struct_declarator_list struct_declarator enum_specifier 
%token <nodes> enumerator_list epostfix_expressionnumerator type_qualifier declarator direct_declarator 
%token <nodes> pointer type_qualifier_list parameter_type_list parameter_list parameter_declaration 
%token <nodes> identifier_list type_name abstract_declarator direct_abstract_declarator initializer 
%token <nodes> initializer_list statement labeled_statement compound_statement declaration_list 
%token <nodes> statement_list expression_statement selection_statement iteration_statement jump_statement 
%token <nodes> translation_unit external_declaration function_definition 

%start translation_unit
%%

primary_expression
	: IDENTIFIER 												{$$ = &node(0,$1,IDENTIFIER);}
	| CONSTANT 													{$$ = &node(0,$1,CONSTANT);}
	| STRING_LITERAL											{$$ = &node(0,$1,STRING_LITERAL);}
	| '(' expression ')'										{$$ = $2;}
	;

postfix_expression
	: primary_expression										{$$ = $1;}
	| postfix_expression '[' expression ']'						{$$ = &node(2, "[]"); $$->v[0] = $1; $$->v[1] = $3;}
	| postfix_expression '(' ')'								{$$ = &node(1, "()"); $$->v[0] = $1;}
	| postfix_expression '(' argument_expression_list ')'		{$$ = &node(2, "(args)"); $$->v[0] = $1; $$->v[1] = $3;}
	| postfix_expression '.' IDENTIFIER							{$$ = &node(2,"."); $$->v[0] = $1; $$->v[1] = &node(0,$3,IDENTIFIER);}
	| postfix_expression PTR_OP IDENTIFIER 						{$$ = &node(2,"->"); $$->v[0] = $1; $$->v[1] = &node(0,$3,IDENTIFIER);}
	| postfix_expression INC_OP									{$$ = &node(1, "exp++"); $$->v[0] = $1;}
	| postfix_expression DEC_OP									{$$ = &node(1, "exp--"); $$->v[0] = $1;}
	;

argument_expression_list
	: assignment_expression										{$$ = &node(1,"arg_list"); $$->v[0] = $1;}
	| argument_expression_list ',' assignment_expression		{$$ = $1; $$->v.push_back($3);}
	;

unary_expression
	: postfix_expression										{$$ = $1;}
	| INC_OP unary_expression									{$$ = &node(1,"++arg"); $$->v[0] = $2;}
	| DEC_OP unary_expression									{$$ = &node(1,"--arg"); $$->v[0] = $2;}
	| unary_operator cast_expression							{$$ = &node(1,$1); $$->v[0] = $2;}
	| SIZEOF unary_expression									{$$ = &node(1,"SIZEOF_unary"); $$->v[0] = $2;}
	| SIZEOF '(' type_name ')'									{$$ = &node(1,"SIZEOF_type"); $$->v[0] = $2;}
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
	| '(' type_name ')' cast_expression							{$$ = &node(2,"()_typecast"); $$->v[0] = $2; $$->v[1] = $4;}
	;

multiplicative_expression
	: cast_expression											{$$ = $1;}
	| multiplicative_expression '*' cast_expression				{$$ = &node(2,"*"); $$->v[0] = $1; $$->v[1] = $3;}
	| multiplicative_expression '/' cast_expression				{$$ = &node(2,"/"); $$->v[0] = $1; $$->v[1] = $3;}
	| multiplicative_expression '%' cast_expression				{$$ = &node(2,"%"); $$->v[0] = $1; $$->v[1] = $3;}
	;

additive_expression
	: multiplicative_expression									{$$ = $1;}
	| additive_expression '+' multiplicative_expression			{$$ = &node(2,"+"); $$->v[0] = $1; $$->v[1] = $3;}
	| additive_expression '-' multiplicative_expression			{$$ = &node(2,"-"); $$->v[0] = $1; $$->v[1] = $3;}
	;

shift_expression
	: additive_expression										{$$ = $1;}
	| shift_expression LEFT_OP additive_expression				{$$ = &node(2,"<<"); $$->v[0] = $1; $$->v[1] = $3;}
	| shift_expression RIGHT_OP additive_expression				{$$ = &node(2,">>"); $$->v[0] = $1; $$->v[1] = $3;}
	;

relational_expression
	: shift_expression											{$$ = $1;}
	| relational_expression '<' shift_expression				{$$ = &node(2,"<"); $$->v[0] = $1; $$->v[1] = $3;}
	| relational_expression '>' shift_expression				{$$ = &node(2,">"); $$->v[0] = $1; $$->v[1] = $3;}
	| relational_expression LE_OP shift_expression				{$$ = &node(2,"<="); $$->v[0] = $1; $$->v[1] = $3;}
	| relational_expression GE_OP shift_expression				{$$ = &node(2,">="); $$->v[0] = $1; $$->v[1] = $3;}
	;

equality_expression
	: relational_expression										{$$ = $1;}
	| equality_expression EQ_OP relational_expression			{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
	| equality_expression NE_OP relational_expression			{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
	;

and_expression
	: equality_expression										{$$ = $1;}
	| and_expression '&' equality_expression					{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
	;

exclusive_or_expression
	: and_expression											{$$ = $1;}
	| exclusive_or_expression '^' and_expression				{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
	;

inclusive_or_expression
	: exclusive_or_expression									{$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression		{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
	;

logical_and_expression
	: inclusive_or_expression									{$$ = $1;}
	| logical_and_expression AND_OP inclusive_or_expression		{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
	;

logical_or_expression
	: logical_and_expression									{$$ = $1;}
	| logical_or_expression OR_OP logical_and_expression		{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
	;

conditional_expression
	: logical_or_expression												{$$ = $1;}
	| logical_or_expression '?' expression ':' conditional_expression	{$$ = &node(3,"ternary"); $$->v[0] = $1; $$->v[1] = $3; $$->v[2] = $5;}
	;

assignment_expression
	: conditional_expression										{$$ = $1;}
	| unary_expression assignment_operator assignment_expression	{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
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
	| expression ',' assignment_expression						{$$ = &node(2,$2); $$->v[0] = $1; $$->v[1] = $3;}
	;

constant_expression
	: conditional_expression									{$$ = $1;}
	;

declaration
	: declaration_specifiers ';'								{}
	| declaration_specifiers init_declarator_list ';'			{}
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
	: init_declarator											{}
	| init_declarator_list ',' init_declarator					{}
	;

init_declarator
	: declarator												{}
	| declarator '=' initializer								{}
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
	: pointer direct_declarator						{}
	| direct_declarator								{}
	;

direct_declarator
	: IDENTIFIER										{}
	| '(' declarator ')'								{}
	| direct_declarator '[' constant_expression ']'		{}
	| direct_declarator '[' ']'							{}
	| direct_declarator '(' parameter_type_list ')'		{}
	| direct_declarator '(' identifier_list ')'			{}
	| direct_declarator '(' ')'							{}
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
	: initializer												{$$ = &node(1,"init_list"); $$->v[0] = $1;}
	| initializer_list ',' initializer							{$$ = $1; $$->v.push_back($3);}
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
	: IDENTIFIER ':' statement									{$$ = &node(1,$1); $$->v[0] = $3;}
	| CASE constant_expression ':' statement					{$$ = &node(2,$1); $$->v[0] = $2; $$->v[1] = $4;}
	| DEFAULT ':' statement										{$$ = &node(1,$1); $$->v[0] = $3;}
	;

compound_statement
	: '{' '}'													{$$ = &node(0,"{}");}
	| '{' statement_list '}'									{$$ = $2;}
	| '{' declaration_list '}'									{$$ = &node(0,"{}");}
	| '{' declaration_list statement_list '}'					{$$ = $3;}
	;

declaration_list
	: declaration												{}
	| declaration_list declaration								{}
	;

statement_list
	: statement													{$$ = &node(1,"stmt_list"); $$->v[0] = $1;}
	| statement_list statement									{$$ = $1; $$->v.push_back($2);}
	;

expression_statement
	: ';'														{$$ = &node(0,$1,';');}
	| expression ';'											{$$ = &node(1,$2,';'); $$->v[0] = $1;}
	;

selection_statement
	: IF '(' expression ')' statement							{$$ = &node(2,$1); $$->v[0] = $3; $$->v[1] = $5;}
	| IF '(' expression ')' statement ELSE statement			{$$ = &node(3, "if-else"); $$->v[0] = $3; $$->v[1] = $5; $$->v[2] = $7;}
	| SWITCH '(' expression ')' statement						{$$ = &node(2, $1); $$->v[0] = $3; $$->v[1] = $5;}
	;

iteration_statement
	: WHILE '(' expression ')' statement											{$$ = &node(2,$1); $$->v[0] = $3; $$->v[1] = $5;}
	| DO statement WHILE '(' expression ')' ';'										{$$ = &node(2,"do-while"); $$->v[0] = $2; $$->v[1] = $5;}
	| FOR '(' expression_statement expression_statement ')' statement				{$$ = &node(3,"for-w/o-update"); $$->v[0] = $3; $$->v[1] = $4; $$->v[2] = $6;}
	| FOR '(' expression_statement expression_statement expression ')' statement	{$$ = &node(4,"for"); $$->v[0] = $3; $$->v[1] = $4; $$->v[2] = $5; $$->v[3] = $7;}
	;

jump_statement
	: GOTO IDENTIFIER ';'										{$$ = &node(1,$1); $$->v[0] = &node(0,$2,IDENTIFIER);}
	| CONTINUE ';'												{$$ = &node(0,$1);}
	| BREAK ';'													{$$ = &node(0,$1);}
	| RETURN ';'												{$$ = &node(0,$1);}
	| RETURN expression ';'										{$$ = &node(1,$1); $$->v[0] = $2;}
	;

translation_unit
	: external_declaration							{$$ = $1;}
	| translation_unit external_declaration			
	;

external_declaration
	: function_definition							{$$ = $1;}
	| declaration									{$$ = &node(0,"declaration");}
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement		{$$ = $4;}
	| declaration_specifiers declarator compound_statement						{$$ = $3;}
	| declarator declaration_list compound_statement							{$$ = $2;}
	| declarator compound_statement
	;
