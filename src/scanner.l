D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#define IDENTIFIER 258
#define CONSTANT 259
#define STRING_LITERAL 260
#define SIZEOF 261
#define PTR_OP 262
#define INC_OP 263
#define DEC_OP 264
#define LEFT_OP 265
#define RIGHT_OP 266
#define LE_OP 267
#define GE_OP 268
#define EQ_OP 269
#define NE_OP 270
#define AND_OP 271
#define OR_OP 272
#define MUL_ASSIGN 273
#define DIV_ASSIGN 274
#define MOD_ASSIGN 275
#define ADD_ASSIGN 276
#define SUB_ASSIGN 277
#define LEFT_ASSIGN 278
#define RIGHT_ASSIGN 279
#define AND_ASSIGN 280
#define XOR_ASSIGN 281
#define OR_ASSIGN 282
#define TYPE_NAME 283
#define TYPEDEF 284
#define EXTERN 285
#define STATIC 286
#define AUTO 287
#define REGISTER 288
#define CHAR 289
#define SHORT 290
#define INT 291
#define LONG 292
#define SIGNED 293
#define UNSIGNED 294
#define FLOAT 295
#define DOUBLE 296
#define CONST 297
#define VOLATILE 298
#define VOID 299
#define STRUCT 300
#define UNION 301
#define ENUM 302
#define ELLIPSIS 303
#define CASE 304
#define DEFAULT 305
#define IF 306
#define ELSE 307
#define SWITCH 308
#define WHILE 309
#define DO 310
#define FOR 311
#define GOTO 312
#define CONTINUE 313
#define BREAK 314
#define RETURN 315
void count();
void comment();
void comment_();
void yyerror();
int check_type();
%}

%%
"/*"			{ comment(); }
"//".*			{ comment_(); }
"auto"			{ return(AUTO); }
"break"			{ return(BREAK); }
"case"			{ return(CASE); }
"char"			{ return(CHAR); }
"const"			{ return(CONST); }
"continue"		{ return(CONTINUE); }
"default"		{ return(DEFAULT); }
"do"			{ return(DO); }
"double"		{ return(DOUBLE); }
"else"			{ return(ELSE); }
"enum"			{ return(ENUM); }
"extern"		{ return(EXTERN); }
"float"			{ return(FLOAT); }
"for"			{ return(FOR); }
"goto"			{ return(GOTO); }
"if"			{ return(IF); }
"int"			{ return(INT); }
"long"			{ return(LONG); }
"register"		{ return(REGISTER); }
"return"		{ return(RETURN); }
"short"			{ return(SHORT); }
"signed"		{ return(SIGNED); }
"sizeof"		{ return(SIZEOF); }
"static"		{ return(STATIC); }
"struct"		{ return(STRUCT); }
"switch"		{ return(SWITCH); }
"typedef"		{ return(TYPEDEF); }
"union"			{ return(UNION); }
"unsigned"		{ return(UNSIGNED); }
"void"			{ return(VOID); }
"volatile"		{ return(VOLATILE); }
"while"			{ return(WHILE); }

{L}({L}|{D})*		{ return(check_type()); }

0[xX]{H}+{IS}?		{ return(CONSTANT); }
0{D}+{IS}?		{ return(CONSTANT); }
{D}+{IS}?		{ return(CONSTANT); }
L?'(\\.|[^\\'])+'	{ return(CONSTANT); }

{D}+{E}{FS}?		{ return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ return(STRING_LITERAL); }
"..."			{ return(ELLIPSIS); }
">>="			{ return(RIGHT_ASSIGN); }
"<<="			{ return(LEFT_ASSIGN); }
"+="			{ return(ADD_ASSIGN); }
"-="			{ return(SUB_ASSIGN); }
"*="			{ return(MUL_ASSIGN); }
"/="			{ return(DIV_ASSIGN); }
"%="			{ return(MOD_ASSIGN); }
"&="			{ return(AND_ASSIGN); }
"^="			{ return(XOR_ASSIGN); }
"|="			{ return(OR_ASSIGN); }
">>"			{ return(RIGHT_OP); }
"<<"			{ return(LEFT_OP); }
"++"			{ return(INC_OP); }
"--"			{ return(DEC_OP); }
"->"			{ return(PTR_OP); }
"&&"			{ return(AND_OP); }
"||"			{ return(OR_OP); }
"<="			{ return(LE_OP); }
">="			{ return(GE_OP); }
"=="			{ return(EQ_OP); }
"!="			{ return(NE_OP); }
";"			{ return(';'); }
("{"|"<%")		{ return('{'); }
("}"|"%>")		{ return('}'); }
","			{ return(','); }
":"			{ return(':'); }
"="			{ return('='); }
"("			{ return('('); }
")"			{ return(')'); }
("["|"<:")		{ return('['); }
("]"|":>")		{ return(']'); }
"."			{ return('.'); }
"&"			{ return('&'); }
"!"			{ return('!'); }
"~"			{ return('~'); }
"-"			{ return('-'); }
"+"			{ return('+'); }
"*"			{ return('*'); }
"/"			{ return('/'); }
"%"			{ return('%'); }
"<"			{ return('<'); }
">"			{ return('>'); }
"^"			{ return('^'); }
"|"			{ return('|'); }
"?"			{ return('?'); }

[ \t\v\n\f]		{ count(); }
.			{ yyerror(yytext); }

%%

int column = 1;
int line  = 1;

void yyerror(char* yytext){
	printf("ERROR at line %d column %d: %c is invalid.\n", line, column, yytext[0]);
	column++;
}

void comment_(){
	return;
}

void comment()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0){
		if(c=='\n'){
			line++;
			column = 1;
		}
		else if (c == '\t')
			column += 4 - ((column-1) % 4);
		else column++;
	}

	if (c=='*') column++;
		
	if ((c1 = input()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}
	else{
		column++;
	}
}



void count()
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n'){
			line++;
			column = 1;
		}
		else if (yytext[i] == '\t')
			column += 4 - ((column-1) % 4);
		else
			column++;

	//ECHO;
}


int check_type()
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return(TYPE_NAME);
*
*	return(IDENTIFIER);
*/

/*
*	it actually will only return IDENTIFIER
*/

	return(IDENTIFIER);
}

void print_entry(char const* s, char * yytext, int l, int c){
	printf("%-24.20s%-30.30s%8d  %8d\n", s, yytext, l, c);
}

int main(int argc, char const* argv[]){
	printf("%-24.20s%-30.30s%10s%10s\n", "TOKEN","LEXEME","LINE#","COLUMN#");
	if(argc != 2){
		printf("USAGE: <bin>/scanner <file_name>\n");
		return 1;
	}
	yyin = fopen(argv[1], "r");
	if(yyin == NULL){
		printf("Can't open file\n");
		return 1;
	}
	int x = yylex();
	while(x>0){
		switch(x){
			case IDENTIFIER:
				print_entry("IDENTIFIER", yytext, line, column);
				break;
			case CONSTANT:
				print_entry("CONSTANT", yytext, line, column);
				break;
			case STRING_LITERAL:
				print_entry("STRING_LITERAL", yytext, line, column);
				break;
			case SIZEOF:
				print_entry("SIZEOF", yytext, line, column);
				break;
			case PTR_OP:
				print_entry("PTR_OP", yytext, line, column);
				break;
			case INC_OP:
				print_entry("INC_OP", yytext, line, column);
				break;
			case DEC_OP:
				print_entry("DEC_OP", yytext, line, column);
				break;
			case LEFT_OP:
				print_entry("LEFT_OP", yytext, line, column);
				break;
			case RIGHT_OP:
				print_entry("RIGHT_OP", yytext, line, column);
				break;
			case LE_OP:
				print_entry("LE_OP", yytext, line, column);
				break;
			case GE_OP:
				print_entry("GE_OP", yytext, line, column);
				break;
			case EQ_OP:
				print_entry("EQ_OP", yytext, line, column);
				break;
			case NE_OP:
				print_entry("NE_OP", yytext, line, column);
				break;
			case AND_OP:
				print_entry("AND_OP", yytext, line, column);
				break;
			case OR_OP:
				print_entry("OR_OP", yytext, line, column);
				break;
			case MUL_ASSIGN:
				print_entry("MUL_ASSIGN", yytext, line, column);
				break;
			case DIV_ASSIGN:
				print_entry("DIV_ASSIGN", yytext, line, column);
				break;
			case MOD_ASSIGN:
				print_entry("MOD_ASSIGN", yytext, line, column);
				break;
			case ADD_ASSIGN:
				print_entry("ADD_ASSIGN", yytext, line, column);
				break;
			case SUB_ASSIGN:
				print_entry("SUB_ASSIGN", yytext, line, column);
				break;
			case LEFT_ASSIGN:
				print_entry("LEFT_ASSIGN", yytext, line, column);
				break;
			case RIGHT_ASSIGN:
				print_entry("RIGHT_ASSIGN", yytext, line, column);
				break;
			case AND_ASSIGN:
				print_entry("AND_ASSIGN", yytext, line, column);
				break;
			case XOR_ASSIGN:
				print_entry("XOR_ASSIGN", yytext, line, column);
				break;
			case OR_ASSIGN:
				print_entry("OR_ASSIGN", yytext, line, column);
				break;
			case TYPE_NAME:
				print_entry("TYPE_NAME", yytext, line, column);
				break;
			case TYPEDEF:
				print_entry("TYPEDEF", yytext, line, column);
				break;
			case EXTERN:
				print_entry("EXTERN", yytext, line, column);
				break;
			case STATIC:
				print_entry("STATIC", yytext, line, column);
				break;
			case AUTO:
				print_entry("AUTO", yytext, line, column);
				break;
			case REGISTER:
				print_entry("REGISTER", yytext, line, column);
				break;
			case CHAR:
				print_entry("CHAR", yytext, line, column);
				break;
			case SHORT:
				print_entry("SHORT", yytext, line, column);
				break;
			case INT:
				print_entry("INT", yytext, line, column);
				break;
			case LONG:
				print_entry("LONG", yytext, line, column);
				break;
			case SIGNED:
				print_entry("SIGNED", yytext, line, column);
				break;
			case UNSIGNED:
				print_entry("UNSIGNED", yytext, line, column);
				break;
			case FLOAT:
				print_entry("FLOAT", yytext, line, column);
				break;
			case DOUBLE:
				print_entry("DOUBLE", yytext, line, column);
				break;
			case CONST:
				print_entry("CONST", yytext, line, column);
				break;
			case VOLATILE:
				print_entry("VOLATILE", yytext, line, column);
				break;
			case VOID:
				print_entry("VOID", yytext, line, column);
				break;
			case STRUCT:
				print_entry("STRUCT", yytext, line, column);
				break;
			case UNION:
				print_entry("UNION", yytext, line, column);
				break;
			case ENUM:
				print_entry("ENUM", yytext, line, column);
				break;
			case ELLIPSIS:
				print_entry("ELLIPSIS", yytext, line, column);
				break;
			case CASE:
				print_entry("CASE", yytext, line, column);
				break;
			case DEFAULT:
				print_entry("DEFAULT", yytext, line, column);
				break;
			case IF:
				print_entry("IF", yytext, line, column);
				break;
			case ELSE:
				print_entry("ELSE", yytext, line, column);
				break;
			case SWITCH:
				print_entry("SWITCH", yytext, line, column);
				break;
			case WHILE:
				print_entry("WHILE", yytext, line, column);
				break;
			case DO:
				print_entry("DO", yytext, line, column);
				break;
			case FOR:
				print_entry("FOR", yytext, line, column);
				break;
			case GOTO:
				print_entry("GOTO", yytext, line, column);
				break;
			case CONTINUE:
				print_entry("CONTINUE", yytext, line, column);
				break;
			case BREAK:
				print_entry("BREAK", yytext, line, column);
				break;
			case RETURN:
				print_entry("RETURN", yytext, line, column);
				break;
			case ';':
				print_entry(";", yytext, line, column);
				break;
			case ',':
				print_entry(",", yytext, line, column);
				break;
			case ':':
				print_entry(":", yytext, line, column);
				break;
			case '=':
				print_entry("=", yytext, line, column);
				break;
			case '(':
				print_entry("(", yytext, line, column);
				break;
			case ')':
				print_entry(")", yytext, line, column);
				break;
			case '.':
				print_entry(".", yytext, line, column);
				break;
			case '&':
				print_entry("&", yytext, line, column);
				break;
			case '!':
				print_entry("!", yytext, line, column);
				break;
			case '~':
				print_entry("~", yytext, line, column);
				break;
			case '-':
				print_entry("-", yytext, line, column);
				break;
			case '+':
				print_entry("+", yytext, line, column);
				break;
			case '*':
				print_entry("*", yytext, line, column);
				break;
			case '/':
				print_entry("/", yytext, line, column);
				break;
			case '%':
				print_entry("%", yytext, line, column);
				break;
			case '<':
				print_entry("<", yytext, line, column);
				break;
			case '>':
				print_entry(">", yytext, line, column);
				break;
			case '^':
				print_entry("^", yytext, line, column);
				break;
			case '|':
				print_entry("|", yytext, line, column);
				break;
			case '?':
				print_entry("?", yytext, line, column);
				break;
			case '{':
				print_entry("{", yytext, line, column);
				break;
			case '}':
				print_entry("}", yytext, line, column);
				break;
			case '[':
				print_entry("[", yytext, line, column);
				break;
			case ']':
				print_entry("]", yytext, line, column);
				break;
		}
		count();
		x = yylex();
	}
}