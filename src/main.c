#include <stdio.h>
#include "y.tab.h"
#include "parse_utils.h"
extern FILE* yyin;
extern char yytext[];
extern node* root;
extern int column;

int dfs(node* u, int num){
	int x = num;
	for(int i = 0; i < u->sz; i++){
		printf("\t%d -> %d\n",x,num+1);
		num = dfs(u->v[i],num+1);
	}
	printf("\t%d [label=\"%s\"]\n",x,u->name);
	return num;
}


//yylval.id = yytext; in all token specs
int main(int argc, char const* argv[]){
	if(argc!=2){
		printf("USAGE: <bin>/parser <file_name>\n");
		return 1;
	}
	yyin = fopen(argv[1], "r");
	if(yyin == NULL){
		printf("Can't Open File\n");
		return 1;
	}
	int x = yyparse();
	if(x != 0){
		return -1;
	}
	printf("digraph G {\n");
	dfs(root,0);
	printf("}");
	return 0;
}