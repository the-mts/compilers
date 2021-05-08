long fac_tail(int n, long accum){
	if (n == 0) return accum;
	return fac_tail(n-1, accum*n);
}

/*unsigned long factorial(int n){
return fac_tail(n, 1);
}*/

int main(){
int n;
printf("Enter n: ");
scanf("%d", &n);
printf("%d! = %ld\n", n, fac_tail(n, 1));
}
