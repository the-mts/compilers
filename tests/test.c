// struct w{
//     int x;
//     char c;
//     float y;
//     int z;
// };

// int f(struct w p){
//     // int x = p.x;
//     printf("%f\n", p.y);
//     printf("F hi: %d %c %d %f\n", p.x, p.c, p.z, p.y);
//     p.x = 4;
//     p.c = 'x';
//     p.z = 33.333;
//     p.y = 0.55;
//     printf("F bye: %d %c %d %f\n", p.x, p.c, p.z, p.y);
//     return (*(&p)).x;
// }

// int main(){
//     struct w q;
//     q.x = 666;
//     q.c = 'a';
//     q.z = 55.234;
//     q.y = 3.21;
//     printf("before F: %d %c %d %f\n", q.x, q.c, q.z, q.y);
//     f(q);
//     printf("after F: %d %c %d %f\n", q.x, q.c, q.z, q.y);
// }

struct w{
    double x;
};

int f(int a1, int a2,int a3,int a4,int a5,int a6,int a7){
    // int x = p.x;
    // printf("%f\n", p.y);
    // int z = 1.1;
    printf("F hi: %d \n", a1);
    // p.x = 4;
    // p.c = 'x';
    // p.z = 33.333;
    // p.y = 0.55;
    // printf("F bye: %lf\n", p.x);
    // return (*(&p)).x;
}

int main(){
    struct w q;
    q.x = 666.111;
    // q.c = 'a';
    // q.z = 55.234;
    // q.y = 3.21;

    // printf("before F: %lf\n", q.x);
    f(1,1,1,1,1,1,1);
    // printf("after F: %lf\n", q.x);
}

