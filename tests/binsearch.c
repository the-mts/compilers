int binsearch(int arr[], int item, int low, int high){
	int mid;
	if (low > high) return -1;
	mid = (low + high)/2;
	if (arr[mid] == item) return mid;
	if (arr[mid] < item) return binsearch(arr, item, mid+1, high);
	return binsearch(arr, item, low, mid-1);
}

int main(){
	int arr[10], item, i;
	printf("Enter 10 integers in increasing order\n");
	for (i = 0; i < 10; i++) scanf("%d", arr + i);
	printf("Enter item to search for: ");
	scanf("%d", &item);
	printf("Index: %d\n", binsearch(arr, item, 0, 9));
}

/*
int main()
{
    int n,t,k; cin >> n >> t >>k;
    int low = 1; int high = n;
    while(high > low)
    {
        int mid = (high+low)/2;
        cout << "? "<< low <<" "<< mid << endl;
        int res; cin >> res;
        if(mid-low+1-res >= k)
        {
            high = mid;
        }
        else
        {
            k -=(mid-low+1 -res);
            low =mid+1;
        }
    }
    cout << "! "<< low << endl;
    return 0;
} */
