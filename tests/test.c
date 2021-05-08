struct x {
	int x;
	float y;
	int a[10][10];
};

int main(){
	struct x p;
	int* x;
	p.a[3][2] = 9;
	p.y = 1.11;
	printf("%d %f\n", p.a[3][2], p.y);
}