/*#include<stdio.h>*/

struct s
{
	int x;
};

void main()
{
	int x = 3;
	int x1 = 5, y = 8;
	int z = x + x1;
    int p[10][5], q[5];
    int a, b = 1, c = 4;
    int i, j;
	z = (int)((float)(x1/2) + ((long double)y)/2)/2;	
    for(i=0; i<10; i++){
        for(j=0; j<5; j++){
            p[i][j] = 4;
        }
    }
    for(i=0; i<5; i++){
        q[i] = 10;
    }
    a = 1;

    q[0] = (--a) + (b++) - sizeof(a+b*x);
    q[1] = sizeof x;
    q[2] = (a<<2) + (q[1]>>2) + (*(q+1));
    

    p[0][x*q[2]] -=  1;
    p[1][x/q[2]] +=  1;
    for(i=0; i<10; i++){
        for(j=0; j<5; j++){
            printf("%d ", p[i][j]);
        }
        printf("\n");
    }
    for(i=0; i<5; i++){
        printf("%d ", q[i]);
    }
    printf("\n");
}