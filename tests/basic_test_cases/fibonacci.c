// Function Call and For Loop
long fib(int n){
	long a = 0, b = 0, c=1;
	int i;
	for (i = 1; i <= n; i++){
		a = b;
		b = c;
		c = a + b;
	}
	return b;
}

int main(){
	int n,i;
	printf("Enter n: ");
	scanf("%d", &n);
	for (i = 0; i<= n; i++)
	printf("fib(%d) = %ld\n", i, fib(i));
}
