struct edge
{
        int u;
        int v;
        int weight;
        struct edge *link;
}*front = 0;

void make_tree(struct edge tree[]);
void insert_pque(int i,int j,int wt);
struct edge *del_pque();
int isEmpty_pque( );
void create_graph();

int n;   /*Total number of vertices in the graph */

int main()
{
        int i;
        struct edge tree[100]; /* Will contain the edges of spanning tree */
        int wt_tree = 0; /* Weight of the spanning tree */

        create_graph();

        make_tree(tree);

        printf("\nEdges to be included in minimum spanning tree are :\n");
        for(i=1; i<=n-1; i++)
        {
                printf("\n%d->",tree[i].u);
                printf("%d\n",tree[i].v);
                wt_tree += tree[i].weight;
        }
        printf("\nWeight of this minimum spanning tree is : %d\n", wt_tree);

        return 0;

}/*End of main()*/

void make_tree(struct edge tree[])
{
        struct edge *tmp;
        int v1,v2,root_v1,root_v2;
        int father[100]; /*Holds father of each vertex */
        int i,count = 0;    /* Denotes number of edges included in the tree */

        for(i=0; i<n; i++)
                father[i] = -1;

        /*Loop till queue becomes empty or
        till n-1 edges have been inserted in the tree*/
        while( !isEmpty_pque( ) && count < n-1 )
        {
                tmp = del_pque();
                v1 = tmp->u;
                v2 = tmp->v;

                while( v1 !=-1 )
                {
                        root_v1 = v1;
                        v1 = father[v1];
                }
                while( v2 != -1  )
                {
                        root_v2 = v2;
                        v2 = father[v2];
                }

                if( root_v1 != root_v2 )/*Insert the edge (v1, v2)*/
                {
                    count++;
                        tree[count].u = tmp->u;
                        tree[count].v = tmp->v;
                        tree[count].weight = tmp->weight;
                        father[root_v2]=root_v1;
                }
        }

        if(count < n-1)
        {
                printf("\nGraph is not connected, no spanning tree possible\n");
        }

}/*End of make_tree()*/

/*Inserting edges in the linked priority queue */
void insert_pque(int i,int j,int wt)
{
        struct edge *tmp,*q;

        tmp = (struct edge *)malloc(sizeof(struct edge));
        tmp->u = i;
        tmp->v = j;
        tmp->weight = wt;

        /*Queue is empty or edge to be added has weight less than first edge*/
        if( front == 0 || tmp->weight < front->weight )
        {
                tmp->link = front;
                front = tmp;
        }
        else
        {
                q = front;
                while( q->link != 0 && q->link->weight <= tmp->weight )
                        q = q->link;
                tmp->link = q->link;
                q->link = tmp;
                if(q->link == 0)  /*Edge to be added at the end*/
                        tmp->link = 0;
        }
}/*End of insert_pque()*/

/*Deleting an edge from the linked priority queue*/
struct edge *del_pque()
{
        struct edge *tmp;
        tmp = front;
        front = front->link;
        return tmp;
}/*End of del_pque()*/

int isEmpty_pque( )
{
        if ( front == 0 )
                return 1;
        else
                return 0;
}/*End of isEmpty_pque()*/

void create_graph()
{
        int i,wt,max_edges,origin,destin;

        // printf("\nEnter number of vertices : ");
        scanf("%d",&n);
        max_edges = n*(n-1)/2;

        for(i=1; i<=max_edges; i++)
        {
                // printf("\nEnter edge %d(-1 -1 to quit): ",i);
                scanf("%d %d",&origin,&destin);
                if( (origin == -1) && (destin == -1) )
                        break;
                // printf("\nEnter weight for this edge : ");
                scanf("%d",&wt);
                if( origin >= n || destin >= n || origin<0 || destin<0)
                {
                        printf("\nInvalid edge!\n");
                        i--;
                }
                else
                        insert_pque(origin,destin,wt);
        }
}
