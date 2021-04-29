struct st{
	int x;
}x;
struct s{
	int x;
};
struct s* fun(struct st x){}
int main(){
	struct st *x1, *x2;
	struct s* x3;
	x3 = fun(*x1);
}

/*

conditional operator

*/