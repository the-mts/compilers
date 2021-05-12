//Mutual Recursion
int fac2(int x);
int fac1(int x){
	if(!x){
		return 1;
	}
	return fac2(x-1)*x;
}
int fac2(int x){
	if(!x){
		return 1;
	}
	return fac1(x-1)*x;
}
int main(){
	int x;
	printf("Enter the number to compute the factorial for: ");
	scanf("%d", &x);
	printf("The factorial of %d is %d\n", x, fac1(x));

}