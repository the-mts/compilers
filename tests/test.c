#include<stdio.h>
int fun(int []);
int fun(int *a){
	printf("%d\n", sizeof(a));
	return 0;
}

int main(){
	int a[5];
	printf("%d\n", sizeof(a));
	fun(a);
	return 0;
}
