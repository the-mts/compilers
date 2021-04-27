// int fact(int x){
// 	if(!x) return 1;
// 	return fact(x-1)*x;
// }

// int fib(int n){
// 	if(!n) return 0;
// 	if(!(n-1)) return 1; 
// 	return fib(n-1)+fib(n-2);
// }

// int main(){
// 	int x = fact(7);
// 	int y = fib(10);
// 	printf("%d %d\n", x, y);
// }

// int fact2(int x);
// int fact1(int x){
//     if(!x){
//         return 1;
//     }
//     return fact2(x-1)*x;
// }
// int fact2(int x){
//     if(!x){
//         return 1;
//     }
//     return fact1(x-1)*x;
// } 

// int main(){
// 	int x = fact1(10);
// 	// int y = fib(10);
// 	printf("%d\n", x);
// }

int main(){
    int a = 407;
    int b = 0;
    int z = a;
    int y;
    while(a){
        y = a%10;
        a = a/10;
        b = b + y*y*y;
    }
    if(!(z-b))
        printf("%d is Armstrong number.\n", z);
    else
        printf("%d is not Armstrong number.\n", z);
    return 0;
}