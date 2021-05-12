 // Float, Double, Pow, Sqrt, Initialized Global

double EPSILON  = 0.001;
double func(double x, double c)
{
    return pow(x, 2) - c;
}
 
double derivFunc(double x)
{
    return 2*x;
}
 
void newtonRaphson(double x, double c)
{
    double h = func(x, c) / derivFunc(x);
    while (fabs(h) >= EPSILON)
    {
        h = func(x, c)/derivFunc(x);
  
        x = x - h;
    }
 
    printf("The value of the root is : %lf\n", x);
    printf("The value of the root computed using sqrt is : %lf\n", sqrt(c));
}
 
int main()
{
    double x0;
    double c;
    printf("Enter the number you want to find the square root of:  ");
    scanf("%lf", &c);
    x0 = c/2;
    newtonRaphson(x0, c);
    return 0;
}