// More than 6 variables in case of ints and more than 8 variables in case of floats
int F(int b, int c, int d, long int e, short int f, int g, char h, long int i){
	return b+c+d+e+f+g+h+i;
}
float FF(float bb, float cc, float dd, double ee, double ff, float gg, double hh, float ii, double jj){
	return bb*cc*dd*ee*ff*gg*hh*ii*jj;
}
int main(){
	int b = 1;
	int c = 2;
	int d = 3;
	long int e = 4;
	short int f = 5;
	int g = 6;
	char h = 7;
	long int i = 8;
	float bb = 1;
	float cc = 2;
	float dd = 3;
	double ee = 4;
	double ff = 5;
	float gg = 6;
	double hh = 7;
	float ii = 8;
	double jj = 9;
	int a = F(b, c, d, e, f, g, h, i); // a = 36
	float aa = FF(bb, cc, dd, ee, ff, gg, hh, ii, jj); //aa = 362880.0
	printf("%d %0.1f\n", a, aa);
	return 0;
}