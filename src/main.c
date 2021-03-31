#include <stdio.h>
#include <iostream>
#include "y.tab.h"
#include "parse_utils.h"
using namespace std;
extern FILE* yyin;
extern char yytext[];
extern node* root;
extern int column;
extern int yydebug;
FILE* out_file;

/*  
	string type;
	unsigned long size;
	long offset;
	int is_init;
	enum sym_type type_name;
	symtab* sym_table;
	vector<pair<string, string>> *arg_list;
*/

void print_table(typtab* table){
	for(auto i = table->begin(); i != table->end(); i++){
		tt_entry* temp = i->second;
		cout << i->first << "\t\t\t" << temp->type << "\n";
		if(temp->mem_list!=NULL){
			// cout<< "\t\tFunction Symbol Table Pointer: " << temp->sym_table<<endl;
			cout<< "\t\tMembers: " << temp->mem_list << " " << temp->mem_list->size() << endl;
			for(auto i : *(temp->mem_list)){
				cout<< "\t\t\t" << i.second << " (" << i.first << ")\n";/*arg_list not implemented yet*/
			}
		}
	}
}

void print_table(symtab* table){
	for(auto i = table->begin(); i != table->end(); i++){
		st_entry* temp = i->second;
		cout << i->first << "\t\t\t" << temp->type << "\t\t\t" << temp->size << "\t\n";
		if(temp->type_name == IS_FUNC){
			cout<< "\t\tFunction Symbol Table Pointer: " << temp->sym_table<<endl;
			cout<< "\t\tArguments: " << temp->arg_list << " " << temp->arg_list->size() << endl;
			for(auto i : *(temp->arg_list)){
				cout<< "\t\t\t" << i.second << " (" << i.first << ")\n";/*arg_list not implemented yet*/
			}
		}
	}
}

void dfs3(table_tree* u){
	cout<< "Scope Type Pointer: "<< u->types << " " << u->val <<endl;
	print_table(u->types);
	cout<<endl;
	for(auto i : u->v){
		dfs3(i);
	}
}

void dfs2(table_tree* u){
	cout<< "Symbol Table Pointer: "<< u->val<<endl;
	print_table(u->val);
	cout<<endl;
	for(auto i : u->v){
		dfs2(i);
	}
}

int dfs(node* u, int num){
	int x = num;
	for(int i = 0; i < u->sz; i++){
		fprintf(out_file,"\t%d -> %d\n",x,num+1);
		num = dfs(u->v[i],num+1);
	}
	// if (u->name[0]!='\"')
	fprintf(out_file,"\t%d [label=\"",x);
	if(u->token == CONSTANT){
		string ch = *new string;
		switch(u->val_dt){
			case IS_INT: u->name = (char*)(to_string(u->val.int_const)).c_str(); break;
			case IS_LONG: u->name = (char*)(to_string(u->val.long_const)).c_str(); break;
			case IS_SHORT: u->name = (char*)(to_string(u->val.short_const)).c_str(); break;
			case IS_U_INT: u->name = (char*)(to_string(u->val.u_int_const)).c_str(); break;
			case IS_U_LONG: u->name = (char*)(to_string(u->val.u_long_const)).c_str(); break;
			case IS_U_SHORT: u->name = (char*)(to_string(u->val.u_short_const)).c_str(); break;
			case IS_FLOAT: u->name = (char*)(to_string(u->val.float_const)).c_str(); break;
			case IS_DOUBLE: u->name = (char*)(to_string(u->val.double_const)).c_str(); break;
			case IS_LONG_DOUBLE: u->name = (char*)(to_string(u->val.long_double_const)).c_str(); break;
			case IS_CHAR: ch = u->val.char_const; ch += '\0'; u->name = (char*)(ch).c_str(); break;
			default: printf("Main.c not a constant\n");exit(-1);
		}
		// u->name = (char*)(to_string(tmp)).c_str();
	}
	int i = 0;
	while(u->name[i] != '\0'){
		if(u->name[i]=='\"' || u->name[i]=='\\') fprintf(out_file,"\\");
		fprintf(out_file, "%c",u->name[i]);
		i++;
	}

	fprintf(out_file,"\"]\n");
	return num;
}


//yylval.id = yytext; in all token specs
int main(int argc, char const* argv[]){
	if(argc<2){
		fprintf(stderr,"USAGE: <bin>/parser <file_name> [options]\n");
		return 1;
	}
	if(argv[1][0] == '-'){
		fprintf(stderr,"USAGE: <bin>/parser <file_name> [options]\n");
		return 1;
	}
	yyin = fopen(argv[1], "r");
	if(yyin == NULL){
		fprintf(stderr,"Can't Open File\n");
		return 1;
	}
	out_file = stdout;
	for(int i = 2; i < argc; i++){
		if(argv[i][0] != '-')
			continue;
		int j = 1;
		while(argv[i][j]!='\0'){
			if(argv[i][j] != 'g' && argv[i][j] != 'o'){
				fprintf(stderr,"Invalid Option\n");
				return 1;
			}
			if(argv[i][j] == 'g'){
				yydebug = 1;
			}
			if(argv[i][j] == 'o'){
				if(argc == i+1){
					fprintf(stderr, "Output file not specified\n");
					return 1;
				}
				out_file = fopen(argv[i+1],"w+");
				if(out_file == NULL){
					fprintf(stderr,"Can't Open Output File\n");
					return 1;
				}
			}
			j++;
		}
	}
	init_symtab();
	init_equiv_types();
	int x = yyparse();
	if(x != 0){
		return -1;
	}
	fprintf(out_file,"digraph G {\n");
	dfs(root,0);
	fprintf(out_file,"}");

	freopen("symtab.out", "w", stdout);
	dfs2(st_root);

	freopen("typtab.out", "w", stdout);
	dfs3(st_root);
	
	return 0;
}