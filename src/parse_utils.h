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
	enum val_type value_type;
	string node_name = "";
	string node_data = "";
}node;

node* node_(int x, char * s, int token);
void add_node(node* par, node* add);
void push_front(node* par, node* add);