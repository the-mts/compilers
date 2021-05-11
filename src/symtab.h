#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;
extern int line;
extern int curr_width;
extern vector<long> offset; 

// offset

/*
{
	int x;
	{
		int y;
	}
}
*/
enum sym_type {UNDEFINED, IS_FUNC, IS_STRUCT, IS_VAR, IS_TYPE, IS_TEMP, REQUIRES_TYPECHECK};
enum const_type {IS_INT, IS_LONG, IS_SHORT, IS_U_INT, IS_U_LONG, IS_U_SHORT, IS_FLOAT, IS_DOUBLE, IS_LONG_DOUBLE, IS_CHAR, IS_NON_CONST};
enum val_type {LVALUE, RVALUE};

typedef unordered_map<string, struct st_entry*> symtab;
typedef unordered_map<string, struct tt_entry*> typtab;

union constant{
	int int_const;
	long long_const;
	short short_const;
	unsigned int u_int_const;
	unsigned long u_long_const;
	unsigned short u_short_const;
	float float_const;
	double double_const;
	long double long_double_const;
	char char_const;
};


struct st_entry{
	string type;
	unsigned long size;
	long offset;
	int is_init;
	enum sym_type type_name;
	symtab* sym_table;
	vector<pair<pair<string, string>,tt_entry*>> *arg_list;
	vector<int> * dim = NULL;
	tt_entry* ttentry = NULL;
	bool is_global = false;
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
	vector<pair<pair<string, string>,tt_entry*>> * mem_list = NULL;
	int is_init = 0;
	int is_typedef = 0;
	typtab* typ_table = NULL;
};
struct table_tree{
	symtab * val;
	typtab* types;
	vector<table_tree*> v;
	table_tree * par;
	string name = "";
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

extern string get_eqtype(string type, int flag = 1);

extern st_entry* add_entry(string key, string type, unsigned long size, long offset, enum sym_type type_name = UNDEFINED);
extern st_entry* lookup(string key);

extern tt_entry* add_type_entry(string key, string type);
extern tt_entry* type_lookup(string key);

extern void new_scope();
extern void scope_cleanup();

extern pair<constant, enum const_type> parse_constant(string s);
extern pair<string, int> get_equivalent_pointer(string s);
extern string reduce_pointer_level(string s);
extern string arithmetic_type_upgrade(string type1, string type2, string op, tt_entry* ttentry1 = NULL, tt_entry* ttentry2 = NULL);
extern string increase_array_level(string s);
extern void check_param_list(vector<pair<pair<string, string>,tt_entry*>> v);
extern void check_valid_array(string s);
extern st_entry* current_lookup(string key);
extern void struct_init_check(string type);
extern tt_entry* current_type_lookup(string key);
extern unsigned long get_size(string s, tt_entry* entry = NULL);
extern void check_mem_list(vector<pair<pair<string, string>,tt_entry*>> v, string s);
extern int is_struct_or_union(string s);
extern st_entry* global_lookup(string key);

extern tt_entry* global_type_lookup(string key);

#endif