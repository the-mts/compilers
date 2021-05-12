// eliminate dead code
int i;
int main(){
	int a, b, c ,d;
	a = 1;
	b = 2;
	c = 3; // DNE
	d = 4; // DNE
	if (1){
		a = a + b;
	}
	// Entire Block DNE
	if ((6-2*3)) {  
		c = c + d;
	}
	
	printf("%d\n", a);
	i = 1234;
	if(i)	
		return 0;
	i = i+i-i||i;
	if (i == 2){
		i=3+5;
	}
	i = i+i-i||i;
	if (i == 2){
		3+5;
	}
	i = i+i-i||i;
	if (i == 2){
		3+5;
	}
	i = i+i-i||i;
	if (i == 2){
		3+5;
	} 
}

/*
Block 2:
alive
isglobal: 0
pred: 1, 
0		a = $1
1		b = $2
2		5_tmp = a +int b
succ: 8, cond_succ: -1
next: 8

Block 8:
alive
isglobal: 0
pred: 2, 
0		PARAM .LC0
1		PARAM 5_tmp
2		CALL printf 8_tmp
succ: 9, cond_succ: -1
next: 9

Block 9:
alive
isglobal: 0
pred: 8, 
0		i = $1234
1		10_tmp = $0
2		RETURN 10_tmp
succ: -1, cond_succ: -1
next: -1
*/
