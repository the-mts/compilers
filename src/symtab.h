#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
using namespace std;
enum sym_type {UNDEFINED, IS_FUNC, IS_STRUCT, IS_VAR, IS_TYPE};
enum const_type {IS_INT, IS_LONG, IS_U_INT, IS_U_LONG, IS_FLOAT, IS_DOUBLE, IS_LONG_DOUBLE};
enum val_type {LVALUE, RVALUE};

typedef unordered_map<string, struct st_entry*> symtab;
typedef unordered_map<string, struct tt_entry*> typtab;

union constant{
	int int_const;
	long long_const;
	unsigned int u_int_const;
	unsigned long u_long_const;
	float float_const;
	double double_const;
	long double long_double_const;
};


struct st_entry{
	string type;
	unsigned long size;
	long offset;
	int is_init;
	enum sym_type type_name;
	symtab* sym_table;
	vector<pair<string, string>> *arg_list;
	vector<int> * dim = NULL;
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
extern void init_equiv_types();
extern void init_symtab();

extern string get_eqtype(string type);

extern st_entry* add_entry(string key, string type, unsigned long size, long offset, enum sym_type type_name = UNDEFINED);
extern st_entry* lookup(string key);

extern tt_entry* add_type_entry(string key, string type);
extern tt_entry* type_lookup(string key);

extern void new_scope();
extern void scope_cleanup();

extern pair<constant, enum const_type> parse_constant(string s);