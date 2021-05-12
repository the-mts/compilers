int main(){
	double a = sin(3.141592653589793238/2);
	double b = cos(3.141592653589793238);
	double c = a + b;
	double comp = 0;
	printf("a = %lf, b = %lf, c = %lf\n", a, b, c);
	if((comp-c) < 0.00001 || (comp-c) > -0.00001){
		printf("Worked fine.\n");
	}
	else{
		printf("Eh! WA.\n");
	}
	return 0;
}