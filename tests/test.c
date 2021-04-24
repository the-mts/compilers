int x = 5;
int a = x;
int fun(){
	int z[2];
	return z[0];
}
int main(){
	int x[5];
	// int * y = x;
	int y[5];
	y[2] = fun();
	return 0;
}

int main(){
	struct x xx;
	xx.y1.y2 = 4;
	{
		int x;
		{
			int * x1;
		}
		{
			struct x xx;
		}
	}
	{
		int y;
	}
}