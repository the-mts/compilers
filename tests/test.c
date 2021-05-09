struct edge{
    char w;
    float z;
    int i;
};

int main(){
    struct edge x;
    struct edge y;
    struct edge* p = &x;
    y.w = 'a';
    // y.i = 10;
    // y.z = 23.111;
    *p = y;
    // y.w++;
    // y.i /= 4;
    // y.z = 13;
    // printf("%c %f %d %c %f %d\n", y.w, y.z, y.i, x.w, x.z, x.i);
    printf("%c.. %c..\n", y.w, x.w);
    // int * R = (int *)malloc((10)*sizeof(int));
    // int * arr = (int *)malloc((10)*sizeof(int));
    // int i;
    // // R[1].weight = 1000000;
    // for(i=0;i<=9;i++){
    //     R[i] = arr[i];
        
    //     // printf("%d\n", R[i].weight)
    // }
    // for(i=0; i<10; i++){
    //     printf("%d\n", R[i]);
    // }
}