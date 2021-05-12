// Struct Union as Parameter and Return Type. Constants of Type Long Int.
struct x{
	int a;
	long int b;
};
union xx{
	int aa;
	long int bb;
};
struct x f(struct x a){
	a.a = 7;
	a.b = 14; 
	return a;
}
union xx ff(union xx aa){
	aa.bb = 1;
	return aa;

}
int main(){
	struct x k;
	struct x l;
	union xx kk;
	union xx ll;
	k.a = 1;
	k.b = 200000000000000l;
	l = f(k);
	printf(" k = {%d, %ld}\n", k.a, k.b);
	printf(" l = {%d, %ld}\n", l.a, l.b);
	kk.bb = 10000000000l;
	ll = ff(kk);
	printf(" kk = %ld\n", kk.bb);
	printf(" ll = %ld\n", ll.bb);
	
	return 0; 
}