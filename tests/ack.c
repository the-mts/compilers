long ack(long m, long n){
	if (m == 0) return n+1;
	if (n == 0) return ack(m-1, 1);
	return ack(m-1, ack(m, n-1));
}

int main(){
	int m, n;
	/*for (m = 0; m <= 6; m++){
	for (n = 0; n <= 4; n++)
	printf("%ld\t", ack(m, n));
	printf("\n");
	}*/
	printf("Enter m and n: ");
	scanf("%d %d", &m, &n);
	printf("A(%d, %d) = %ld\n", m, n, ack(m, n));
}
