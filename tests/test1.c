struct st;

int fun1(int, char*, char, double);
void fun2(int* [], long double *xx[2][3], struct st);
	
struct st{
	int x, y;
	int* z;
};

int fun1(int x, char* y, char z, double w){
	z = x;
	x = '\'';
	*y = 1.2+3<1+'a'? w:x;
	x = *y;
	return 0;
}

void fun2(int* x[], long double* xx[][3], struct st s){
	int y = sizeof(s);
	return;
}

int main(){
	return 0;
}