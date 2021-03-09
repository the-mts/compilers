typedef struct node{
	struct node ** v;
	char * name;
	int token;
	int sz;
}node;

node* node_(int x, char * s, int token);
void add_node(node* par, node* add);
void push_front(node* par, node* add);