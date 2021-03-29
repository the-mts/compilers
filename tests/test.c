struct x{
	int zz;
};
int main(){
	struct x{
		float y;
	};
	struct x z;
	char ch;
	z.y = 3.4;
	ch = z.y;
	return 0;
}