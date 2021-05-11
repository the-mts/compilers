struct node{
    int rank;
    int parent;
    int key;
};
 struct vertex{
     int value;
     struct vertex* next;
     int prob;
 };
struct edge{
    int u;
    int v;
    int weight;
};

    
    
    

void edge_includer(int v1, int v2,struct vertex**adj,int w){
     struct vertex* temp = (struct vertex*)malloc(sizeof(struct vertex));
     temp->value = v2;
     temp->prob = w;
     temp->next = adj[v1];
     adj[v1] = temp;
 }
void edge_remover(int v1,int v2, struct vertex**adj){
    struct vertex * previous;
    struct vertex * temp;
    temp = adj[v1];
    if(temp!=0 && temp->value==v2){
        adj[v1]=temp->next;
        return;
    }
    while(temp!= 0 && temp->value!=v2){
        previous = temp;
        temp = temp->next;
    }
    if(temp==0) return;
    previous->next = temp->next;
    return;
}
void BFS(struct vertex **adj,int * parent, int* weight, int* status,int n,int v1,int v2){
    int queue[10001];
    int head = 0;
    int tail = 0;
    queue[tail++]=v1;
    status[v1]=1;
    while(head!=tail){
        int current = queue[head++];
        struct vertex* temp = adj[current];
        // printf("hi %d %d\n", head, tail);
        while(temp!=0){
            // printf("hi %p\n", temp);
            if(status[temp->value]==0){
                queue[tail++]=temp->value;
                parent[temp->value]=current;
                status[temp->value] = 1;
                weight[temp->value] = temp->prob;
                
            }
            if(temp->value==v2){
                return;
            }
            else{
                temp=temp->next;
            }
        }
    }
    
}
void Merge(struct edge *arr,int p,int q,int r){
    // struct edge L[q-p+2],R[r-q+1];
    struct edge * R = (struct edge *)malloc((r-q+1)*sizeof(struct edge));
    struct edge * L = (struct edge *)malloc((q-p+2)*sizeof(struct edge));
    int i;
    int x, y;
    R[r-q].weight = 1000000;
    L[q-p+1].weight = 1000000;
    for(i=0;i<=q-p;i++){
        L[i] = arr[p+i] ;
    }
    for(i=0;i<=r-q-1;i++){
        R[i] = arr[q+1+i] ;
    }
    x = 0;
    y = 0;
    for(i=p;i<=r;i++){
       
       
        if (L[x].weight <= R[y].weight){
            arr[i] = L[x];
            x+=1;
        }
            
        else {
            arr[i] = R[y];
            y+=1;
        
        }
        
        
    }
             
    return;
        
        
}
int find(struct node x, struct node* set){
    struct node y = x;
    int result;
    while(y.key!=y.parent){
        y = set[y.parent];
    }
    result = y.parent;
    y = set[x.parent];
    
    while(y.key!=y.parent){
        x.parent = result;
        x = y;
        y = set[y.parent];
    }
    return result;
}
void Union(struct edge* array,struct node* set,int i){
    int r_x = find(set[array[i].u],set);
    
    int r_y = find(set[array[i].v],set);
    // printf("Union\n");
    // printf("%d %d %d\n", set[r_x].parent, set[r_x].rank, set[r_x].key);
    // printf("%d %d %d\n", set[r_y].parent, set[r_y].rank, set[r_y].key);
    
    if(set[r_x].parent==set[r_y].parent) return;
    if(set[r_x].rank > set[r_y].rank) set[r_y].parent = set[r_x].key;
    else if (set[r_x].rank<set[r_y].rank) set[r_x].parent = set[r_y].key;
    else {
        set[r_y].parent = set[r_x].key;
        set[r_x].rank = set[r_x].rank +1;
    }
    
}


void MergeSort(struct edge* arr,int p,int r){
    if(p<r){
        int q = (p+r)/2;
        MergeSort(arr,p,q);
        
        MergeSort(arr,q+1,r);
        
        Merge(arr,p,q,r);
        
        
    }
    else 
        return ;
}


int main() {
         int n;
         int e;
         int q;
         int * parent, * status, * wgt;
         struct edge * array;
         struct vertex ** adjacency;
         struct node* set;
         int i;
         int MST;
         scanf("%d",&n);
         scanf("%d",&e);
         scanf("%d",&q);
         parent = (int*)malloc(n*sizeof(int));
         status = (int*)malloc(n*sizeof(int));
         wgt = (int*)malloc(n*sizeof(int));
         array = (struct edge*)malloc((e+q)*sizeof(struct edge));
         adjacency = (struct vertex**)malloc(n*sizeof(struct vertex*));
         
     
             for(i = 0;i<e;i++){
                 int v1, v2;
                 int w;
                 scanf("%d %d %d",&v1,&v2,&w); 
                 array[i].u = v1;
                 array[i].v = v2;
                 array[i].weight = w;
                 
             }
        MergeSort(array,0,e-1);
             
    

     set = (struct node*)malloc(n*sizeof(struct node));
    
    printf("\n");
    for(i =0;i<n;i++){
        set[i].rank = 0;
        set[i].parent = i;
        set[i].key = i;
        // printf("%d %d\n", set[i].parent, set[i].key);
    }
    // for(i=0;i<n;i++)
    //     printf("%d %d %d\n", set[i].rank, set[i].parent, set[i].key);
    
    MST = 0;
            // int ii;
    for(i=0;i<e;i++){
        
        if((set[find(set[array[i].u],set)].key)!=(set[find(set[array[i].v],set)].key)){
            
            MST = MST + array[i].weight;
            Union(array,set,i);
            // printf("i = %d\n", i);
            edge_includer(array[i].u,array[i].v,adjacency,array[i].weight);
            edge_includer(array[i].v,array[i].u,adjacency,array[i].weight);
        }
    }
    printf("%d\n",MST);
    
    for(i=0;i<q;i++){
        
        int v1;
        int v2;
        int w;
        int max_v;
        int max_u;
        int maximum;
        int current;
        int j;
        for(j=0;j<n;j++){
            parent[j]=-1;
            wgt[j]=0;
            status[j]=0;
        }
        scanf("%d",&v1);
        scanf("%d",&v2);
        scanf("%d",&w);
        array[e+i].u = v1;
        array[e+i].v = v2;
        array[e+i].weight = w;
        BFS(adjacency,parent,wgt,status,n,v1,v2);
        maximum=0;
        current = v2;
        while(current!=v1){
            if(maximum<wgt[current]){
                max_u=current;
                max_v=parent[current];
                maximum = wgt[current];
            }
            current = parent[current];
        }
        if(maximum<=w){
            printf("%d\n",MST);
        }
        if(maximum>w){
            edge_remover(max_u,max_v,adjacency);
            edge_remover(max_v,max_u,adjacency);
            edge_includer(v1,v2,adjacency,w);
            edge_includer(v2,v1,adjacency,w);
            MST = MST+w-maximum;
            printf("%d\n",MST);
        }
    }

    

    /* Enter your code here. Read input from STDIN. Print output to STDOUT */    
    return 0;
}
