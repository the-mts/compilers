#include "symtab.h"
using namespace std;
vector<symtab*> table_scope;
vector<typtab*> type_scope;
symtab global;
typtab types_table;
table_tree* st_root;
table_tree* curr;
unordered_map<string, string> equiv_types;

void init_equiv_types(){
	equiv_types.insert({"int","int"});
    equiv_types.insert({"char", "char"});
    equiv_types.insert({"float", "float"});
    equiv_types.insert({"void", "void"});
    equiv_types.insert({"double", "double"});

    equiv_types.insert({"signed", "int"});
    equiv_types.insert({"unsigned", "unsigned int"});

    equiv_types.insert({"long", "long int"});
    equiv_types.insert({"short", "short int"});

    equiv_types.insert({"long int", "long int"});
    equiv_types.insert({"short int", "short int"});

    equiv_types.insert({"signed long", "long int"});
    equiv_types.insert({"signed short", "short int"});
    equiv_types.insert({"signed long int", "long int"});
    equiv_types.insert({"signed short int", "short int"});

    equiv_types.insert({"signed int", "int"});
    equiv_types.insert({"unsigned int", "unsigned int"});

    equiv_types.insert({"unsigned long", "unsigned long int"});
    equiv_types.insert({"unsigned short", "unsigned short int"});
    equiv_types.insert({"unsigned long int", "unsigned long int"});
    equiv_types.insert({"unsigned short int", "unsigned short int"});

    equiv_types.insert({"long double", "long double"});
    equiv_types.insert({"unsigned char", "char"});
}

string get_eqtype(string type){
    vector<pair<int, string>> a;
    unordered_map<string, int> m;
    m["struct"] = 0;
    m["enum"] = 0;
    m["signed"] = 1;
    m["unsigned"] = 1;
    m["short"] = 2;
    m["long"] = 2;
    m["int"] = 3;
    m["double"] = 4;
    m["char"] = 5;
    m["float"] = 6;
    m["void"] = 7;

    string tmp ="";
    for(auto i : type){
        if(i==' '){
            a.push_back({(m.find(tmp)!=m.end() ? m[tmp] : 10)  , tmp});
            tmp = "";
        }
        else tmp += i;
    }
    a.push_back({(m.find(tmp)!=m.end() ? m[tmp] : 10)  , tmp});

    sort(a.begin(), a.end());
    
    if(a[0].first==0){
        if(a.size()!=2) return "";
        else return type;
    }

    string new_type="";
    for(auto i:a){
        if(new_type!="") new_type += " ";
        new_type += i.second;
    }

    if(equiv_types.find(new_type)==equiv_types.end()) return "";
    return equiv_types[new_type];
}

void init_symtab(){
	table_scope.push_back(&global);
	type_scope.push_back(&types_table);
	st_root = new table_tree(&global, &types_table);
	curr = st_root;

}

st_entry* add_entry(string key, string type, unsigned long size, long offset, enum sym_type type_name){
	st_entry * new_entry = new st_entry(type, size, offset, type_name);
	//assert(table_scope.size() != 0);//check scope stack
	symtab * temp = table_scope.back();
	temp->insert({key, new_entry});
	return new_entry;
}

tt_entry* add_type_entry(string key, string type){
	tt_entry * new_entry = new tt_entry;
	typtab * temp = type_scope.back();
	temp->insert({key, new_entry});
	return new_entry;
}

st_entry* lookup(string key){
	for(int i = (int)(table_scope.size())-1; i >= 0; i--){
		auto it = table_scope[i]->find(key);
		if(it != table_scope[i]->end())
			return it->second;
	}
	return NULL;
}

tt_entry* type_lookup(string key){
	for(int i = (int)(type_scope.size())-1; i >= 0; i--){
		auto it = type_scope[i]->find(key);
		if(it != type_scope[i]->end())
			return it->second;
	}
	return NULL;
}

void new_scope(){
	symtab * new_table= new symtab;
	typtab * new_type_table = new typtab;
	table_tree * temp = new table_tree(new_table, new_type_table, curr);
	curr->v.push_back(temp);
	curr = temp;
	table_scope.push_back(new_table);
	type_scope.push_back(new_type_table);
}

void scope_cleanup(){
	curr = curr->par;
	table_scope.pop_back();
}

pair<constant, enum const_type> parse_constant(string s){
	return pair<constant, enum const_type>();
}
