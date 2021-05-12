// Multi-Dimensional Arrays
void mult(int l, int m, int n, int a[][3], int b[][3], int c[][3]){
	int i, j, k;
	for (i = 0; i < l; i++){
		for (j = 0; j < n; j++){
			c[i][j] = 0;
			for (k = 0; k < l; k++) c[i][j] += a[i][k]* b[k][j];
		}
	}
}

int main(){
	int l = 3, m = 3, n = 3;
	int i, j;
	int a[3][3], b[3][3], c[3][3];
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
