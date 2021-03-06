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
void yyerror(char*s);
%}

%%
"/*"			{ comment(); }
"//".*			{ comment_(); }
"auto"			{ yylval.id = yytext;return(AUTO); }
"break"			{ yylval.id = yytext;return(BREAK); }
"case"			{ yylval.id = yytext;return(CASE); }
"char"			{ yylval.id = yytext;return(CHAR); }
"const"			{ yylval.id = yytext;return(CONST); }
"continue"		{ yylval.id = yytext;return(CONTINUE); }
"default"		{ yylval.id = yytext;return(DEFAULT); }
"do"			{ yylval.id = yytext;return(DO); }
"double"		{ yylval.id = yytext;return(DOUBLE); }
"else"			{ yylval.id = yytext;return(ELSE); }
"enum"			{ yylval.id = yytext;return(ENUM); }
"extern"		{ yylval.id = yytext;return(EXTERN); }
"float"			{ yylval.id = yytext;return(FLOAT); }
"for"			{ yylval.id = yytext;return(FOR); }
"goto"			{ yylval.id = yytext;return(GOTO); }
"if"			{ yylval.id = yytext;return(IF); }
"int"			{ yylval.id = yytext;return(INT); }
"long"			{ yylval.id = yytext;return(LONG); }
"register"		{ yylval.id = yytext;return(REGISTER); }
"return"		{ yylval.id = yytext;return(RETURN); }
"short"			{ yylval.id = yytext;return(SHORT); }
"signed"		{ yylval.id = yytext;return(SIGNED); }
"sizeof"		{ yylval.id = yytext;return(SIZEOF); }
"static"		{ yylval.id = yytext;return(STATIC); }
"struct"		{ yylval.id = yytext;return(STRUCT); }
"switch"		{ yylval.id = yytext;return(SWITCH); }
"typedef"		{ yylval.id = yytext;return(TYPEDEF); }
"union"			{ yylval.id = yytext;return(UNION); }
"unsigned"		{ yylval.id = yytext;return(UNSIGNED); }
"void"			{ yylval.id = yytext;return(VOID); }
"volatile"		{ yylval.id = yytext;return(VOLATILE); }
"while"			{ yylval.id = yytext;return(WHILE); }

{L}({L}|{D})*		{ yylval.id = yytext;return(IDENTIFIER); }

0[xX]{H}+{IS}?		{ yylval.id = yytext;return(CONSTANT); }
0{D}+{IS}?		{ yylval.id = yytext;return(CONSTANT); }
{D}+{IS}?		{ yylval.id = yytext;return(CONSTANT); }
L?'(\\.|[^\\'])+'	{ yylval.id = yytext;return(CONSTANT); }

{D}+{E}{FS}?		{ yylval.id = yytext;return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ yylval.id = yytext;return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ yylval.id = yytext;return(CONSTANT); }

L?\"(\\.|[^\\"])*\"	{ yylval.id = yytext;return(STRING_LITERAL); }
"..."			{ yylval.id = yytext;return(ELLIPSIS); }
">>="			{ yylval.id = yytext;return(RIGHT_ASSIGN); }
"<<="			{ yylval.id = yytext;return(LEFT_ASSIGN); }
"+="			{ yylval.id = yytext;return(ADD_ASSIGN); }
"-="			{ yylval.id = yytext;return(SUB_ASSIGN); }
"*="			{ yylval.id = yytext;return(MUL_ASSIGN); }
"/="			{ yylval.id = yytext;return(DIV_ASSIGN); }
"%="			{ yylval.id = yytext;return(MOD_ASSIGN); }
"&="			{ yylval.id = yytext;return(AND_ASSIGN); }
"^="			{ yylval.id = yytext;return(XOR_ASSIGN); }
"|="			{ yylval.id = yytext;return(OR_ASSIGN); }
">>"			{ yylval.id = yytext;return(RIGHT_OP); }
"<<"			{ yylval.id = yytext;return(LEFT_OP); }
"++"			{ yylval.id = yytext;return(INC_OP); }
"--"			{ yylval.id = yytext;return(DEC_OP); }
"->"			{ yylval.id = yytext;return(PTR_OP); }
"&&"			{ yylval.id = yytext;return(AND_OP); }
"||"			{ yylval.id = yytext;return(OR_OP); }
"<="			{ yylval.id = yytext;return(LE_OP); }
">="			{ yylval.id = yytext;return(GE_OP); }
"=="			{ yylval.id = yytext;return(EQ_OP); }
"!="			{ yylval.id = yytext;return(NE_OP); }
";"			{ yylval.id = yytext;return(';'); }
("{"|"<%")		{ yylval.id = yytext;return('{'); }
("}"|"%>")		{ yylval.id = yytext;return('}'); }
","			{ yylval.id = yytext;return(','); }
":"			{ yylval.id = yytext;return(':'); }
"="			{ yylval.id = yytext;return('='); }
"("			{ yylval.id = yytext;return('('); }
")"			{ yylval.id = yytext;return(')'); }
("["|"<:")		{ yylval.id = yytext;return('['); }
("]"|":>")		{ yylval.id = yytext;return(']'); }
"."			{ yylval.id = yytext;return('.'); }
"&"			{ yylval.id = yytext;return('&'); }
"!"			{ yylval.id = yytext;return('!'); }
"~"			{ yylval.id = yytext;return('~'); }
"-"			{ yylval.id = yytext;return('-'); }
"+"			{ yylval.id = yytext;return('+'); }
"*"			{ yylval.id = yytext;return('*'); }
"/"			{ yylval.id = yytext;return('/'); }
"%"			{ yylval.id = yytext;return('%'); }
"<"			{ yylval.id = yytext;return('<'); }
">"			{ yylval.id = yytext;return('>'); }
"^"			{ yylval.id = yytext;return('^'); }
"|"			{ yylval.id = yytext;return('|'); }
"?"			{ yylval.id = yytext;return('?'); }

[ \t\v\n\f]		{ count(); }
.			{ yyerror(yytext); }

%%

int column = 1;
int line  = 1;

void yyerror(char* yytext){
	printf("ERROR at line %d column %d: %s is invalid.\n", line, column, yytext);
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
