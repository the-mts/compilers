int main(){
	long int x = 55;
	long int i = 11;
	for(i; i>=0; i--){
		x-=i;
		printf("%ld %ld\n", x, i);
	}
	// printf("%d\n", x);
	return 0;
}