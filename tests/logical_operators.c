int main(){
    int x = 32;
    int y = 2;
    int z = (x<<y);
    int w = (x>>y);
    int a = z | 3;
    int b = w & 15;
    int c = a ^ 4;
    printf("%d %d %d %d %d\n", z, w, a, b, c);
}