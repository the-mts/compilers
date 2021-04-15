#include<string>
#include<vector>
#include "symtab.h"
using namespace std;

typedef struct node{
	vector<node*> v;
	char * name;
	int token;
	int sz;
	int node_type = 0;
	constant val;
	enum const_type val_dt;
	enum val_type value_type;
	string node_name = "";
	string node_data = "";

	// 3AC Attributes:
	qi place;
	list<int> nextlist;
	list<int> truelist;
	list<int> falselist;
	list<int> contlist;
	list<int> breaklist;
}node;

node* node_(int x, char * s, int token);
void add_node(node* par, node* add);
void push_front(node* par, node* add);
void evaluate_const(node* par, node* child, int op, string type);