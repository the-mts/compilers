#include <string>
#include <unordered_map>
#include <vector>
struct st_entry{
	string type;
	unsigned long size;
	long offset;
	st_entry(string type, unsigned long size, long offset){
		this->type = type;
		this->size = size;
		this->offset = offset;
	}
};
typedef unordered_map<string, st_entry*> symtab;

struct table_tree{
	symtab * val;
	vector<table_tree*> v;
	table_tree * par;
	table_tree(symtab* val, table_tree* par = NULL){
		this->val = val;
		this->par = par;
	}
};
extern vector<symtab*> table_scope;
extern symtab global;
extern table_tree* root;
extern table_tree* curr;
void init_symtab();
void add_entry(string key, string type, unsigned long size, long offset);
st_entry* lookup(string key);
void new_scope();
void scope_cleanup();