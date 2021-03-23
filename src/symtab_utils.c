#include "symtab.h"

vector<symtab*> table_scope;
symtab global;
table_tree* root;
table_tree* curr;

void init_symtab(){
	table_scope.push_back(&global);
	root = table_tree(&global);
	curr = root;
}

void add_entry(string key, string type, unsigned long size, long offset){
	st_entry * new_entry = st_entry(type, size, offset);
	assert(table_scope.size() != 0);//check scope stack
	symtab * temp = table_scope.back();
	temp->insert({key, new_entry});
}

st_entry* lookup(string key){
	for(int i = (int)(table_scope.size())-1; i >= 0; i--){
		auto it = table_scope[i]->find(key);
		if(it != table_scope[i]->end())
			return it->second;
	}
	return NULL;
}

void new_scope(){
	symtab * new_table= new symtab;
	table_tree * temp = table_tree(new_table,curr);
	curr->v.push_back(temp);
	curr = temp;
	table_scope.push_back(new_table);
}

void scope_cleanup(){
	curr = curr->par;
	table_scope.pop_back();
}