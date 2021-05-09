struct edge{
    char w;
    int i;
    float z;
};

int main(){
    struct edge x;
    struct edge y;
    y.w = 'a';
    y.i = 7;
    y.z = 4.0;
    x = y;
    y.w++;
    y.i++;
    y.z = 0;
    printf("%d %d %d %d %d %d", y.w, y.i, y.z, x.w, x.i, x.z);
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