#include <string>
#include <unordered_map>
#include <vector>
using namespace std;
enum sym_type {UNDEFINED, IS_FUNC, IS_STRUCT, IS_VAR, IS_TYPE};

typedef unordered_map<string, struct st_entry*> symtab;
typedef unordered_map<string, struct tt_entry*> typtab;

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
struct tt_entry{
	string type;
	vector<pair<string, string>> * mem_list = NULL;
	int is_init = 0;
	int is_typedef = 0;
	typtab* typ_table = NULL;
};
struct table_tree{
	symtab * val;
	typtab* types;
	vector<table_tree*> v;
	table_tree * par;
	table_tree(symtab* val, typtab* types, table_tree* par = NULL){
		this->val = val;
		this->types = types;
		this->par = par;

	}
};

extern vector<symtab*> table_scope;
extern vector<typtab*> type_scope;
extern symtab global;
extern table_tree* st_root;
extern table_tree* curr;
extern typtab types_table;
void init_symtab();
extern st_entry* add_entry(string key, string type, unsigned long size, long offset, enum sym_type type_name = UNDEFINED);
extern tt_entry* add_type_entry(string key, string type);
extern st_entry* lookup(string key);
extern tt_entry* type_lookup(string key);
void new_scope();
void scope_cleanup();