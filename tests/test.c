double func(float a, double b, float c){
	printf("%f %lf %f\n", a, b, c);
	return (a+b*c)/2;
}
int main(){
	float a = 4;
	double b = 3.5;
	float c = 1;
    float f = func(a, b, c);
	printf("%f %lf %f\n", a, b, c);
    printf("%f\n", f);
    return 0;

}