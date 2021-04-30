int main(){
	int a = 2;
	int *x = &a;
	(*x)++;
	printf("%d\n", a);
}