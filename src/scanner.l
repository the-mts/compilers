D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include "y.tab.h"
void count();
void comment();
void comment_();
void yyerror(const char*s);
%}

%%
"/*"			{ comment(); }
"//".*			{ comment_(); }
"auto"			{ yylval.id = strdup(yytext);return(AUTO); }
"break"			{ yylval.id = strdup(yytext);return(BREAK); }
"case"			{ yylval.id = strdup(yytext);return(CASE); }
"char"			{ yylval.id = strdup(yytext);return(CHAR); }
"const"			{ yylval.id = strdup(yytext);return(CONST); }
"continue"		{ yylval.id = strdup(yytext);return(CONTINUE); }
"default"		{ yylval.id = strdup(yytext);return(DEFAULT); }
"do"			{ yylval.id = strdup(yytext);return(DO); }
"double"		{ yylval.id = strdup(yytext);return(DOUBLE); }
"else"			{ yylval.id = strdup(yytext);return(ELSE); }
"enum"			{ yylval.id = strdup(yytext);return(ENUM); }
"extern"		{ yylval.id = strdup(yytext);return(EXTERN); }
"float"			{ yylval.id = strdup(yytext);return(FLOAT); }
"for"			{ yylval.id = strdup(yytext);return(FOR); }
"goto"			{ yylval.id = strdup(yytext);return(GOTO); }
"if"			{ yylval.id = strdup(yytext);return(IF); }
"int"			{ yylval.id = strdup(yytext);return(INT); }
"long"			{ yylval.id = strdup(yytext);return(LONG); }
"register"		{ yylval.id = strdup(yytext);return(REGISTER); }
"return"		{ yylval.id = strdup(yytext);return(RETURN); }
"short"			{ yylval.id = strdup(yytext);return(SHORT); }
"signed"		{ yylval.id = strdup(yytext);return(SIGNED); }
"sizeof"		{ yylval.id = strdup(yytext);return(SIZEOF); }
"static"		{ yylval.id = strdup(yytext);return(STATIC); }
"struct"		{ yylval.id = strdup(yytext);return(STRUCT); }
"switch"		{ yylval.id = strdup(yytext);return(SWITCH); }
"typedef"		{ yylval.id = strdup(yytext);return(TYPEDEF); }
"union"			{ yylval.id = strdup(yytext);return(UNION); }
"unsigned"		{ yylval.id = strdup(yytext);return(UNSIGNED); }
"void"			{ yylval.id = strdup(yytext);return(VOID); }
"volatile"		{ yylval.id = strdup(yytext);return(VOLATILE); }
"while"			{ yylval.id = strdup(yytext);return(WHILE); }

{L}({L}|{D})*		{ yylval.id = strdup(yytext);return(IDENTIFIER); }

0[xX]{H}+{IS}?		{ yylval.id = strdup(yytext);return(CONSTANT); }
0{D}+{IS}?		{ yylval.id = strdup(yytext);return(CONSTANT); }
{D}+{IS}?		{ yylval.id = strdup(yytext);return(CONSTANT); }
L?'(\\.|[^\\'])+'	{ yylval.id = strdup(yytext);return(CONSTANT); }

{D}+{E}{FS}?		{ yylval.id = strdup(yytext);return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ yylval.id = strdup(yytext);return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ yylval.id = strdup(yytext);return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ yylval.id = strdup(yytext);return(STRING_LITERAL); }
"..."			{ yylval.id = strdup(yytext);return(ELLIPSIS); }
">>="			{ yylval.id = strdup(yytext);return(RIGHT_ASSIGN); }
"<<="			{ yylval.id = strdup(yytext);return(LEFT_ASSIGN); }
"+="			{ yylval.id = strdup(yytext);return(ADD_ASSIGN); }
"-="			{ yylval.id = strdup(yytext);return(SUB_ASSIGN); }
"*="			{ yylval.id = strdup(yytext);return(MUL_ASSIGN); }
"/="			{ yylval.id = strdup(yytext);return(DIV_ASSIGN); }
"%="			{ yylval.id = strdup(yytext);return(MOD_ASSIGN); }
"&="			{ yylval.id = strdup(yytext);return(AND_ASSIGN); }
"^="			{ yylval.id = strdup(yytext);return(XOR_ASSIGN); }
"|="			{ yylval.id = strdup(yytext);return(OR_ASSIGN); }
">>"			{ yylval.id = strdup(yytext);return(RIGHT_OP); }
"<<"			{ yylval.id = strdup(yytext);return(LEFT_OP); }
"++"			{ yylval.id = strdup(yytext);return(INC_OP); }
"--"			{ yylval.id = strdup(yytext);return(DEC_OP); }
"->"			{ yylval.id = strdup(yytext);return(PTR_OP); }
"&&"			{ yylval.id = strdup(yytext);return(AND_OP); }
"||"			{ yylval.id = strdup(yytext);return(OR_OP); }
"<="			{ yylval.id = strdup(yytext);return(LE_OP); }
">="			{ yylval.id = strdup(yytext);return(GE_OP); }
"=="			{ yylval.id = strdup(yytext);return(EQ_OP); }
"!="			{ yylval.id = strdup(yytext);return(NE_OP); }
";"			{ yylval.id = strdup(yytext);return(';'); }
("{"|"<%")		{ yylval.id = strdup(yytext);return('{'); }
("}"|"%>")		{ yylval.id = strdup(yytext);return('}'); }
","			{ yylval.id = strdup(yytext);return(','); }
":"			{ yylval.id = strdup(yytext);return(':'); }
"="			{ yylval.id = strdup(yytext);return('='); }
"("			{ yylval.id = strdup(yytext);return('('); }
")"			{ yylval.id = strdup(yytext);return(')'); }
("["|"<:")		{ yylval.id = strdup(yytext);return('['); }
("]"|":>")		{ yylval.id = strdup(yytext);return(']'); }
"."			{ yylval.id = strdup(yytext);return('.'); }
"&"			{ yylval.id = strdup(yytext);return('&'); }
"!"			{ yylval.id = strdup(yytext);return('!'); }
"~"			{ yylval.id = strdup(yytext);return('~'); }
"-"			{ yylval.id = strdup(yytext);return('-'); }
"+"			{ yylval.id = strdup(yytext);return('+'); }
"*"			{ yylval.id = strdup(yytext);return('*'); }
"/"			{ yylval.id = strdup(yytext);return('/'); }
"%"			{ yylval.id = strdup(yytext);return('%'); }
"<"			{ yylval.id = strdup(yytext);return('<'); }
">"			{ yylval.id = strdup(yytext);return('>'); }
"^"			{ yylval.id = strdup(yytext);return('^'); }
"|"			{ yylval.id = strdup(yytext);return('|'); }
"?"			{ yylval.id = strdup(yytext);return('?'); }

[ \t\v\n\f]		{ count(); }
.			{ yyerror(yytext); }

%%

int column = 1;
int line  = 1;

void yyerror(const char* yytext){
	printf("ERROR at line %d: %s\n", line, yytext);
	column++;
}

void comment_(){
	return;
}

int yywrap(){
	return 1;
}

void comment()
{
	char c, c1;

loop:
	while ((c = yyinput()) != '*' && c != 0){
		if(c=='\n'){
			line++;
			column = 1;
		}
		else if (c == '\t')
			column += 4 - ((column-1) % 4);
		else column++;
	}

	if (c=='*') column++;
		
	if ((c1 = yyinput()) != '/' && c != 0)
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
