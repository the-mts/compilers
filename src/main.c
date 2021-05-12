#include <stdio.h>
#include <iostream>
#include "y.tab.h"
#include "parse_utils.h"
#include "3AC.h"
#include "codegen.h"
using namespace std;
extern FILE* yyin;
extern char yytext[];
extern node* root;
extern int column;
extern int yydebug;
FILE* dot_file;
int typ_file = 0;
int scope_num = 0;
string curr_fun = "";
string out_file;
int file_ptrs = 1;
int enable_opt = 1;
int enable_tail = 1;
int enable_warn = 1;

// For debugging purposes
// Have to rewrite
void print_quad(quad q){
    cout<<q.op<<' '<<(q.op1.first!=""? q.op1.first: "-")<<' '<<(q.op2.first!=""? q.op2.first: "-")<<' '<<(q.res.first!=""? q.res.first: "-")<<' '<<(q.goto_addr);
}

// For debugging purposes
// Have to rewrite
void print_code(){
	int j=0;
    for(auto i: code_array){
		cout<<j<<"\t\t";
		if(i.op == "ADDR=")	cout<<i.res.first<< ' '<<i.op << ' '<<i.op1.first;
        else if(i.op.find("UNARY")!= string::npos) cout<<i.res.first<< " = " << i.op<< ' '<<i.op1.first;
		else if(i.op == "IF_TRUE_GOTO") cout<<"IF "<< i.op1.first << " IS TRUE GOTO " << i.goto_addr;
		else if(i.op == "GOTO") cout<<"GOTO " << i.goto_addr;
		else if(i.op == "PARAM") cout<<"PARAM " << i.op1.first;
		else if(i.op == "CALL") cout<<"CALL " << i.op1.first<<' '<<i.res.first;
		else if(i.op == "RETURN_VOID") cout<<"RETURN_VOID";
		else if(i.op == "RETURN") cout<<"RETURN " << i.op1.first;
		else if(i.op == "FUNC_START") cout<<"<" << i.op1.first << ">:";
		else if(i.op == "FUNC_END")	cout<<"<FUNC_END>";
		else if(i.op1.first!="" && i.op2.first!="") cout<<i.res.first<< " = " << i.op1.first <<' '<< i.op<<' ' << i.op2.first;
		else if(i.op2.first == "" && i.op[0]=='=') cout<<i.res.first<< ' '<<i.op << ' '<<i.op1.first;
		else if(i.op2.first == "") cout<<i.res.first<< " = "<< i.op << ' '<<i.op1.first;
        cout<<endl;
		j++;
    }
}

void print_blocks(int only_alive = 1){
	int c = 0, j;
	if (blocks.size()==0) return;
	if (only_alive){
		for (int b = 0; b != -1; b = blocks[b].next){
			cout<<"Block "<<b<<":"<<endl;
			cout<<"isglobal: "<<blocks[b].isglobal<<endl;
			j = 0;
			cout<<"pred: ";
			for (auto p: blocks[b].pred)
			cout << p << ", ";
			cout<<endl;
			for(auto i: blocks[b].code){
				cout<<j<<"\t\t";
		        if(i.op == "ADDR=")	cout<<i.res.first<< ' '<<i.op << ' '<<i.op1.first;
        		else if(i.op.find("UNARY")!= string::npos) cout<<i.res.first<< " = " << i.op<< ' '<<i.op1.first;
				else if(i.op == "IF_TRUE_GOTO") cout<<"IF "<< i.op1.first << " IS TRUE GOTO " << i.goto_addr;
				else if(i.op == "GOTO") cout<<"GOTO " << i.goto_addr;
				else if(i.op == "PARAM") cout<<"PARAM " << i.op1.first;
				else if(i.op == "CALL") cout<<"CALL " << i.op1.first<<' '<<i.res.first;
				else if(i.op == "RETURN_VOID") cout<<"RETURN_VOID";
				else if(i.op == "RETURN") cout<<"RETURN " << i.op1.first;
				else if(i.op == "FUNC_START") cout<<"<" << i.op1.first << ">:";
				else if(i.op == "FUNC_END") cout<<"<" << "FUNC_END"<<">";
				else if(i.op1.first!="" && i.op2.first!="") cout<<i.res.first<< " = " << i.op1.first <<' '<< i.op<<' ' << i.op2.first;
				else if(i.op2.first == "" && i.op[0]=='=') cout<<i.res.first<< ' '<<i.op << ' '<<i.op1.first;
				else if(i.op2.first == "") cout<<i.res.first<< " = "<< i.op << ' '<<i.op1.first;
	    	    cout<<endl;
				j++;
    		}
			//cout<<"varstart: "<< blocks[b].varstart<<", varend: "<< blocks[b].varend << endl;
			cout<<"succ: " << blocks[b].succ << ", cond_succ: " << blocks[b].cond_succ << endl;
			cout<<"next: "<< blocks[b].next << endl << endl;
			c++;
		}
	}
	else {
		for (auto b: blocks){
			cout<<"Block "<<c<<":"<<endl;
			if (b.alive) cout<<"alive\n";
			else cout<<"dead\n";
			cout<<"isglobal: "<<b.isglobal<<endl;

			j = 0;
			cout<<"pred: ";
			for (auto p: b.pred)
			cout << p << ", ";
			cout<<endl;
			for(auto i: b.code){
				cout<<j<<"\t\t";
		        if(i.op == "ADDR=")	cout<<i.res.first<< ' '<<i.op << ' '<<i.op1.first;
        		else if(i.op.find("UNARY")!= string::npos) cout<<i.res.first<< " = " << i.op<< ' '<<i.op1.first;
				else if(i.op == "IF_TRUE_GOTO") cout<<"IF "<< i.op1.first << " IS TRUE GOTO " << i.goto_addr;
				else if(i.op == "GOTO") cout<<"GOTO " << i.goto_addr;
				else if(i.op == "PARAM") cout<<"PARAM " << i.op1.first;
				else if(i.op == "CALL") cout<<"CALL " << i.op1.first<<' '<<i.res.first;
				else if(i.op == "RETURN_VOID") cout<<"RETURN_VOID";
				else if(i.op == "RETURN") cout<<"RETURN " << i.op1.first;
				else if(i.op == "FUNC_START") cout<<"<" << i.op1.first << ">:";
				else if(i.op == "FUNC_END") cout<<"<" << "FUNC_END"<<">";
				else if(i.op1.first!="" && i.op2.first!="") cout<<i.res.first<< " = " << i.op1.first <<' '<< i.op<<' ' << i.op2.first;
				else if(i.op2.first == "" && i.op[0]=='=') cout<<i.res.first<< ' '<<i.op << ' '<<i.op1.first;
				else if(i.op2.first == "") cout<<i.res.first<< " = "<< i.op << ' '<<i.op1.first;
	    	    cout<<endl;
				j++;
    		}
			//cout<<"varstart: " << b.varstart << ", varend: " << b.varend << endl;
			cout<<"succ: " << b.succ << ", cond_succ: " << b.cond_succ << endl;
			cout<<"next: "<< b.next << endl << endl;
			c++;
		}
	}
}

void print_table(typtab* table){
	string name = "";
	if(scope_num == 0){
		scope_num++;
		name = curr_fun;
	}
	else{
		name = curr_fun+"("+to_string(scope_num++)+")";
	}

	printf("Scope '%s':\n", name.c_str());

	for(auto i = table->begin(); i != table->end(); i++){
		tt_entry* temp = i->second;
		cout << "\t\t" << i->first << "\t\t\t" << temp->type << "\n";
		if(temp->mem_list!=NULL){
			cout<< "\t\t\t" <<temp->mem_list->size()<< " members: " << endl;
			for(auto i : *(temp->mem_list)){
				cout<< "\t\t\t\t" << i.first.second << " (" << i.first.first << ")\n";
			}
		}
	}
}

void print_table(symtab* table){
	string name = "";
	if(scope_num == 0){
		scope_num++;
		name = curr_fun;
	}
	else{
		name = curr_fun+"("+to_string(scope_num++)+")";
	}

	printf("Scope '%s':\n", name.c_str());

	vector<pair<int, pair<st_entry*, string>>> order;

	for(auto i = table->begin(); i != table->end(); i++){
		st_entry* temp = i->second;
		order.push_back(make_pair(temp->offset,make_pair(temp, i->first)));
	}

	sort(order.begin(), order.end());

	for(auto i : order){
		st_entry* temp = i.second.first;
		cout << "\t\t" << i.second.second << "\t\t" << temp->size << "\t\t" << temp->offset << "\t\t" << temp->type << "\n";
		cout << "\t\tTop of stack = " << temp->top_of_stack << "\n";
		if(temp->type_name == IS_FUNC){
			cout<< "\t\t\t" << temp->arg_list->size() <<" arguments " << (temp->arg_list->size()? "{\n" : "\n");
			for(auto i : *(temp->arg_list)){
				cout << "\t\t\t\t" << i.first.second << " (" << i.first.first << ")\n";
			}
			if(temp->arg_list->size()) cout << "\t\t\t" << "}\n";
		}
	}
	cout<<"\n";
}

void dfs3(table_tree* u){
	print_table(u->types);
	for(auto i : u->v){
		if(u == st_root) curr_fun = i->name, scope_num  = 0;
		dfs3(i);
	}
}

void dfs2(table_tree* u){
	print_table(u->val);
	if(u == st_root) cout<<'\n';
	for(auto i : u->v){
		if(u == st_root) curr_fun = i->name, scope_num  = 0;
		dfs2(i);
		if(u == st_root) cout<<'\n';
	}
}

int dfs(node* u, int num){
	int x = num;
	for(int i = 0; i < u->sz; i++){
		fprintf(dot_file,"\t%d -> %d\n",x,num+1);
		num = dfs(u->v[i],num+1);
	}
	// if (u->name[0]!='\"')
	fprintf(dot_file,"\t%d [label=\"",x);
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
			case IS_CHAR: ch = "'"; ch += u->val.char_const; ch += "'"; u->name = (char*)(ch).c_str(); break;
			default: printf("Main.c not a constant\n");exit(-1);
		}
		// u->name = (char*)(to_string(tmp)).c_str();
	}
	int i = 0;
	while(u->name[i] != '\0'){
		if(u->name[i]=='\"' || u->name[i]=='\\') fprintf(dot_file,"\\");
		fprintf(dot_file,"%c",u->name[i]);
		i++;
	}

	printf("\"]\n");
	return num;
}


//yylval.id = yytext; in all token specs
int main(int argc, char const* argv[]){
	if(argc<2){
		printf("USAGE: <bin>/parser <file_name> [options]\n");
		return 1;
	}
	if(argv[1][0] == '-'){
		printf("USAGE: <bin>/parser <file_name> [options]\n");
		return 1;
	}
	yyin = fopen(argv[1], "r");
	if(yyin == NULL){
		printf("Can't Open File\n");
		return 1;
	}
	dot_file = fopen("bin/AST.dot","w+");
	out_file = "./bin/assembly.s";
	int skipnext = 0;
	for(int i = 2; i < argc; i++){
		if(skipnext){
			skipnext = 0;
			continue;
		}
		if(argv[i][0] != '-'){
			printf("Invalid option format.\n");
			return 1;
			continue;
		}
		int j = 1;
		while(argv[i][j]!='\0'){
			if(argv[i][j] != 'g' && argv[i][j] != 'o' && argv[i][j] != 't' && argv[i][j] != 'f' && argv[i][j]!='O' && argv[i][j]!='W' && argv[i][j]!='T'){
				printf("Invalid Option\n");
				return 1;
			}
			if(argv[i][j] == 'g'){
				yydebug = 1;
			}
			if(argv[i][j] == 'o'){
				if(argc == i+1){
					printf( "Output file not specified\n");
					return 1;
				}
				skipnext = 1;
				out_file = string(argv[i+1]);
			}
			if(argv[i][j] == 't'){
				typ_file = 1;
			}
			if(argv[i][j] == 'f'){
				file_ptrs = 0;
			}
			if(argv[i][j] == 'O'){
				enable_opt = 0;
			}
			if(argv[i][j] == 'W'){
				enable_warn = 0;
			}
			if(argv[i][j] == 'T'){
				enable_tail = 0;
			}
			j++;
		}
	}
	// file_ptrs = 1;
	init_symtab();
	init_equiv_types();
	init_escape_chars();
	int x = yyparse();
	if(x != 0){
		return -1;
	}
	fprintf(dot_file,"digraph G {\n");
	dfs(root,0);
	fprintf(dot_file,"}");

	freopen("bin/symtab.csv", "w", stdout);

	st_root->name = "global";
	curr_fun = "global";
	dfs2(st_root);

	if(typ_file){
		scope_num = 0;
		freopen("bin/typtab.csv", "w", stdout);
		curr_fun = "global";
		dfs3(st_root);
	}
	//fclose(fp);
	freopen("bin/3AC.txt", "w", stdout);
	print_code();

//	cout<<"Check me\n";
	make_blocks();
	if(enable_opt){
		optimize();
	}

 	freopen("bin/basic_blocks.txt", "w", stdout);
 	print_blocks(0);
 	freopen(out_file.c_str(), "w", stdout);
 	codegen();
	return 0;
}
