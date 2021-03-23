#include <string>
#include <unordered_map>
#include <vector>
using namespace std;
enum sym_type {UNDEFINED, IS_FUNC, IS_STRUCT, IS_VAR, TYPE__NAME};

typedef unordered_map<string, struct st_entry*> symtab;

struct st_entry{
	string type;
	unsigned long size;
	long offset;
	int is_init;
	enum sym_type type_name;
	symtab* sym_table;
	vector<pair<string, string>> *arg_list;
	st_entry(string type, unsigned long size, long offset, enum sym_type type_name = UNDEFINED){
		this->type = type;
		this->size = size;
		this->offset = offset;
		this->type_name = type_name;
		this->is_init = 0;
		this->sym_table = NULL;
	}
};

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
extern table_tree* st_root;
extern table_tree* curr;
void init_symtab();
extern st_entry* add_entry(string key, string type, unsigned long size, long offset, enum sym_type type_name = UNDEFINED);
st_entry* lookup(string key);
void new_scope();
void scope_cleanup();