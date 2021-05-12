int main() {

    int a = 11;
    long int b = 20;
    short int c = 4;
    float d = 11.1; 
    float e = 7.4; 
    double f = 100;

    double temp1 = ( a - b % c ) / ( d * e - f / b ) * ( f * 1.1 ) + a + b + c + d;
    float temp2 = ( b * ( a - 20.1 ) ) * ( d / e / f ) + d * e + f * a - b / c;
    
    printf("temp1 = %lf\n", temp1);
    printf("temp2 = %f\n", temp2);
    return 0;
}