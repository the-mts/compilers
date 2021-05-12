//Struct, Array of Struct, Passing Array of Struct as an Argument and Recursion
struct array{
    int x;
    int dummy;
};

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
    struct array arr[10];
	printf("Enter 10 integers in increasing order\n");
	for (i = 0; i < 10; i++){
        scanf("%d", &arr[i].x);
    }
    printf("You can only make 10 queries, use them wisely.\n");
    for(i=0; i<10; i++){
		printf("Enter item to search for: ");
		scanf("%d", &item);
		printf("Index: %d\n", binsearch(arr, item, 0, 9));
	}
}
