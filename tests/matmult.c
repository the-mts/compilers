void mult(int l, int m, int n, int a[][2], int b[][2], int c[][2]){
	int i, j, k;
	for (i = 0; i < l; i++){
		for (j = 0; j < n; j++){
			c[i][j] = 0;
			for (k = 0; k < l; k++) c[i][j] += a[i][k]* b[k][j];
		}
	}
}

int main(){
	int l = 2, m = 2, n = 2;
	int i, j;
	int a[2][2], b[2][2], c[2][2];
	printf("Enter a %d x %d matrix:\n", l, m);
	for (i = 0; i < l; i++)
	for (j = 0; j < m; j++)
	scanf("%d", &a[i][j]);
	printf("Enter a %d x %d matrix:\n", m, n);
	for (i = 0; i < m; i++)
	for (j = 0; j < n; j++)
	scanf("%d", &b[i][j]);
	mult(l, m, n, a, b, c);
	for (i = 0; i < l; i++){
		for (j = 0; j < n; j++) printf("%d ", c[i][j]);
		printf("\n");
	}
}