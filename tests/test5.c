/* #include<stdio.h> */

typedef struct node{
	int x;
	int y;
	struct node* next;
}node;

void fun(int x, int* y, int ** z, struct node** node2)
{
	int i=0;
	struct node** node1 = (struct node**)malloc(x**y*sizeof(struct node*));
	for(i=0;i<=x*y;i++)
	{
		node1 = (struct node*)malloc(2*sizeof(struct node));
		node1->x = 5;
		node1->y = 9;
	}
	node[0]->x = node[1]->x * node[node[2]->x]->y;
}

int main(){
	int x=2, y=5;
	int *ptr = 2;
	struct node * s = (struct node*)malloc(x*y*sizeof(struct node));
	s->next = NULL;
	s->x = y*x;
	fun(x,&y,*ptr,s);
	s->next = (struct node*)malloc(s->x*y*sizeof(struct node));
}
