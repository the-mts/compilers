// eliminate dead code
int i;
int main(){
	int a, b, c ,d;
	a = 1;
	b = 2;
	c = 3; // DNE
	d = 4; // DNE
	i = 1234;
	if (1){
		a = a + b;
	}
	// Entire Block DNE
	if ((6-2*3)) {  
		c = c + d;
	}
	
	printf("%d\n", a);
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
2		i = $1234
3		6_tmp = a +int b
succ: 5, cond_succ: -1
next: 5

Block 5:
alive
isglobal: 0
pred: 2, 
0		PARAM .LC0
1		PARAM 6_tmp
2		CALL printf 9_tmp
succ: 9, cond_succ: -1
next: 9

Block 9:
alive
isglobal: 0
pred: 5, 
0		10_tmp = $0
1		RETURN 10_tmp
succ: -1, cond_succ: -1
next: -1
*/
