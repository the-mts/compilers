struct Vertex {
    int n;
    long w;
    struct Vertex* adj;
};

struct Heapnode {
    int n;
    long d;
};

void Heapify (struct Heapnode** H, int size, int i){
    int minIndex;
    struct Heapnode* temp;
    if (2*i > size) return;
    else{
        if (H[2*i]->d < H[i]->d) 
        	minIndex = 2*i;
        else minIndex = i;
    }
    if ((2*i+1 <= size) && (H[2*i+1]->d < H[minIndex]->d)) minIndex = 2*i+1;
    
    if (minIndex!=i){
        temp = H[i];
        H[i] = H[minIndex];
        H[minIndex] = temp;
        Heapify(H, size, minIndex);
    }
}

struct Heapnode* Extract_Min(struct Heapnode** H, int size){
    struct Heapnode* Min = H[1];
    H[1] = H[size];
    //size--;
    Heapify(H, size-1, 1);
    return Min;
}

void Decrease_Key(struct Heapnode** H, int size, int i, long val){
    struct Heapnode* temp;
    int j = 1;
    for (j; j<=size; j++){
        if (H[j]->n==i) break;
    }
    H[j]->d = val;
    
    while ((j>1)&&(H[j/2]->d>H[j]->d)){
        temp = H[j/2];
        H[j/2] = H[j];
        H[j] = temp;
        j = j/2;
    }
}

int main() {
    int V, E;
    struct Heapnode** H;
    int size = 0;
    long* dist;
    
    int s,t;
    struct Vertex** G;
    int i;
    int x, y;
    long w;
    struct Heapnode* u;
    scanf("%d %d", &V, &E);
    dist = (long*)malloc(V*(sizeof(long int)));
    H = (struct Heapnode**)malloc((V+1)*sizeof(struct Heapnode*));
    G = (struct Vertex**)malloc(V*sizeof(struct Vertex*));
    for (i=0; i<V; i++){
        struct Vertex* v = (struct Vertex*) malloc(sizeof(struct Vertex));
        v->n = i;
        v->adj = 0;
        G[i] = v;
    }
    while (E--){
        struct Vertex* uy = (struct Vertex*) malloc(sizeof(struct Vertex));
        
        scanf("%d %d %ld", &x, &y, &w);
        uy->n = y;
        uy->w = w;
        uy->adj = G[x]->adj;
        G[x]->adj = uy;
    }
    
    scanf("%d %d", &s, &t);
    
    u = (struct Heapnode*) malloc(sizeof(struct Heapnode));
    u->n = s;
    u->d = 0;
    H[++size] = u;
    dist[s]=0;
    
    for (i=0; i<V; i++){
        if (i!=s){
            struct Heapnode* u = (struct Heapnode*) malloc(sizeof(struct Heapnode));
            u->n = i;
            u->d = 1000000000;
            H[++size] = u;
            dist[i]= 1000000000;
        }
    }
    
    
    
    while (size){
        struct Heapnode* u = Extract_Min(H, size);
        struct Vertex* curr = G[u->n];
        size--;
        while (curr->adj){
            if (dist[curr->adj->n] > dist[u->n] + curr->adj->w){
                dist[curr->adj->n] = dist[u->n] + curr->adj->w;
                Decrease_Key(H, size, curr->adj->n, dist[curr->adj->n]);    
            }
            
            curr = curr->adj;
        }
    }
    
    printf("\nMin Distance: %ld\n", dist[t]);
    
    
    return 0;
}

/* 
10 40
7 4 1701
3 9 9070
8 1 754
4 2 1599
5 6 9547
1 6 6797
2 0 5054
1 5 9506
6 1 8728
2 3 8248
6 0 1790
7 8 7936
3 1 9884
5 9 9139
6 5 6736
8 2 2314
6 2 7769
7 6 2351
5 8 4585
8 7 1804
6 7 228
0 5 3486
0 4 2746
7 2 7453
6 9 2740
9 0 7019
2 5 1342
1 0 4110
3 5 6174
9 3 673
0 3 660
5 2 6164
4 3 7059
7 1 8562
4 5 3802
3 0 7228
5 1 6868
2 9 3221
0 2 5836
2 1 4653
0 9
 */

/* Expected output---
Min Distance: 7566
 */