int main(){
    int a = 9;
    int *x = &a;
    int z = --(*x);
    printf("%d %d\n", a, z);
    *x /= 2;
    printf("%d %d\n", a, z);
}