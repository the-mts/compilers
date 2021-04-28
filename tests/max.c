int main(){
	int a = 2, b = 5;
	char x = 'F', y = 'y';
	int n = a > b;
	int l = a==y;
	printf("%d\n", a > b ? a:b);
	printf("%c\n", y > x ? y : x);
	printf("%d\n", y > a ? y : a);
	return n;
}
