void swap(int* a, int l, int r){
    int tmp = a[r];
    a[r] = a[l];
    a[l] = tmp;
}

void merge(int* a, int l, int m, int r){
    int * a1, *a2;
    int x = l, i, l1 = 0, l2 = 0;;
    a1 = (int*)malloc((m-l+1)*sizeof(int));
    a2 = (int*)malloc((r-m)*sizeof(int));
    for(i=l;i<=m;i++){
        a1[i-l] = a[i];
    }
    for(i=m+1;i<=r;i++){
        a2[i-m-1] = a[i];
    }
    while(l1+l<=m && l2+m+1<=r){
        if(a1[l1]<=a2[l2]){
            a[x++] = a1[l1++];
        }
        else{
            a[x++] = a2[l2++];
        }
    }
    while(l1+l<=m){
        a[x++] = a1[l1++];
    }
    while(l2+m+1<=r){
        a[x++] = a2[l2++];
    }
}

void sort(int* a, int l, int r){
    int m = (l+r)/2;
    if(r<=l)
        return;
    sort(a,l,m);
    sort(a,m+1,r);
    merge(a,l,m,r);

}

int main(){
    int i, n, *a;
    int x;
    printf("Enter the number of elements: ");
    scanf("%d", &n);
    printf("%d\n", n);
    a = (int*) malloc(n*sizeof(int));
    // printf("%p\n", a);
    printf("Enter the numbers to sort:\n");
    for(i=0;i<n;i++){
      scanf("%d", &a[i]);
    }
    sort(a,0,n-1);
    for(i=0;i<n;i++){
        printf("%d ", a[i]);
    }
    printf("\n");
}