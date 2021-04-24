struct y{
	int y1, y2;
};
struct x{
	int x1, x2;
	struct y y1;
};

int fun(struct x x3, int a1, int * a2, char a3, char * a4, 
	char a5[], float* a6, struct x x2, long double * a7, 
	float f1, float f2, float f3, float f4, float f5, 
	double f6, double f7, double f8, struct x x1, struct y y1, double f9)
{

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