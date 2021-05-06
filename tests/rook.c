// Count rook tours on an m x n board that start at (1,1) and end at (1,n)

void recurse(int **arr, int m, int n, int a, int b, int i, long *cnt){
	int d0, d1;
	if (i == m*n -2){
		if (arr[0][n-1] && ((a==0 && b==n-2)||(a==1 && b == n-1)))
		(*cnt)++;
		return;
	}

	d0 = 1, d1 = 0;
	if (a + d0 > -1 && b + d1 > -1 && a + d0 < m && b + d1 < n && arr[a+d0][b+d1]){
		arr[a+d0][b+d1] = 0;
		recurse(arr, m, n, a+d0, b+d1, i+1, cnt);
		arr[a+d0][b+d1] = 1;
	}

	d0 = -1, d1 = 0;
	if (a + d0 > -1 && b + d1 > -1 && a + d0 < m && b + d1 < n && arr[a+d0][b+d1]){
		arr[a+d0][b+d1] = 0;
		recurse(arr, m, n, a+d0, b+d1, i+1, cnt);
		arr[a+d0][b+d1] = 1;
	}
	
	d0 = 0, d1 = 1;
	if (a + d0 > -1 && b + d1 > -1 && a + d0 < m && b + d1 < n && arr[a+d0][b+d1]){
		arr[a+d0][b+d1] = 0;
		recurse(arr, m, n, a+d0, b+d1, i+1, cnt);
		arr[a+d0][b+d1] = 1;
	}

	d0 = 0, d1 = -1;
	if (a + d0 > -1 && b + d1 > -1 && a + d0 < m && b + d1 < n && arr[a+d0][b+d1]){
		arr[a+d0][b+d1] = 0;
		recurse(arr, m, n, a+d0, b+d1, i+1, cnt);
		arr[a+d0][b+d1] = 1;
	}
}

long compute(int m, int n){
	int i, j;
	long cnt;
	int **arr = (int**)malloc(m*sizeof(int*));
	for (i=0; i<m; i++){
	arr[i] = (int*)malloc(n*sizeof(int));
	for (j=0; j<n; j++)
	arr[i][j] = 1;
	}
	cnt = 0;
	arr[0][0] = 0;
	recurse(arr, m, n, 0, 0, 0, &cnt);
	for (i=0; i < m; i++)
	free(arr[i]);
	free(arr);
	return cnt;
}

int main(){
	int m, n, i;
	printf("Enter m and n: ");
	scanf("%d%d", &m, &n);
	for (i = 1; i <= n; i++)
	printf("i: %d\tcnt: %ld\n", i, compute(m, i));
	return 0;
}
