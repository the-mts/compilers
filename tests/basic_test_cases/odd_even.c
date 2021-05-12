//Mutual Recursion
long int fac2(long int x);
long int fac1(long int x){
	if(x<=0){
		return 1;
	}
	return fac2(x-1)*x;
}
long int fac2(long int x){
	if(x<=0){
		return 1;
	}
	return fac1(x-1)*x;
}
int main(){
	long int x;
	printf("Enter the number to compute the factorial for: ");
	scanf("%ld", &x);
	if(x<0){
		printf("Factorial not defined for negative numbers.\n");
		return 0;
	}
	printf("The factorial of %ld is %ld\n", x, fac1(x));
	return 0;
}