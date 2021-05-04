void swap(int* a, int l, int r){
    int tmp = a[r];
    a[r] = a[l];
    a[l] = tmp;
}

void sort(int* a, int l, int r){
    int m = (l+r)/2;
    int i;
    int l1 = l-1;
    if(r<l)
        return;
    if(l==r)
        return;
    for(i=l;i<r;i++){
        if(a[i]<=a[r]){
            l1++;
            swap(a,i,l1);
        }
    }
    l1++;
    swap(a,l1,r);
    sort(a,l,l1-1);
    sort(a,l1+1,r);
}

int main(){
    int i, n, *a;
    int x;
    printf("Enter the number of elements: ");
    scanf("%d", &n);
    printf("%d\n", n);
    a = (int*) malloc(n*sizeof(int));
    printf("%p\n", a);
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