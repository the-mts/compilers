struct array{
    int x;
};
    struct array arr[10];

int binsearch(struct array arr[], int item, int low, int high){
    int mid;
    if (low > high) return -1;
    mid = (low + high)/2;
    if (arr[mid].x == item) return mid;
    if (arr[mid].x < item) return binsearch(arr, item, mid+1, high);
    return binsearch(arr, item, low, mid-1);
}

int main(){
    int item, i;
    printf("Enter 10 integers in increasing order\n");
    for (i = 0; i < 10; i++){
        // scanf("%d", &arr[i].x);
        arr[i].x = i;
    }
    printf("Enter item to search for: ");
    scanf("%d", &item);
    printf("Index: %d\n", binsearch(arr, item, 0, 9));
}
