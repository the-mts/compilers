struct x{
	int a;
	// long int b;
};
struct x f(struct x a){
	// a.a = 7;
	// a.b = 14; 
	// printf("here\n");
	// printf("%d\n", a.a);
	return a;
}
int main(){
	struct x k;
	// struct x l;
	k.a = 1;
	// k.b = 2;
	f(k);
	// printf(" k = {%d, %ld}\n", k.a, k.b);
	// printf(" l = {%d, %ld}\n", l.a, l.b);
	return 0; 
}