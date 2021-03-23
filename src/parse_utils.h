#include<string>
using namespace std;
typedef struct node{
	struct node ** v;
	char * name;
	int token;
	int sz;
	int node_type = 0;
	string node_name = "";
	string node_data = "";
}node;

node* node_(int x, char * s, int token);
void add_node(node* par, node* add);
void push_front(node* par, node* add);