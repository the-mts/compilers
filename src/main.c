#include <stdio.h>

extern char yytext[];
// extern int column;

// yyerror(s)
// char *s;
// {
// 	fflush(stdout);
// 	printf("\n%*s\n%*s\n", column, "^", column, s);
// }

//yylval.id = yytext in all token specs
int main(){
	int x = yyparse();
	return 0;
}