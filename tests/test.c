int main(){
	long int x = 55;
	long int i = 1;
	for(i; i<=10; i++){
		x-=i;
		printf("%ld %ld\n", x, i);
	}
	// printf("%d\n", x);
	return 0;
}
// Block 0:
// isglobal: 0
// pred: 
// 0		<main>:
// varstart: 0, varend: 8
// succ: 1, cond_succ: -1
// next: 1

// Block 1:
// isglobal: 0
// pred: 0, 
// 0		0_tmp = $55
// 1		x = 0_tmp
// 2		1_tmp = $1
// 3		i = 1_tmp
// varstart: 0, varend: 8
// succ: 2, cond_succ: -1
// next: 2

// Block 2:
// isglobal: 0
// pred: 1, 4, 
// 0		2_tmp = $10
// 1		3_tmp = i <=int 2_tmp
// 2		IF 3_tmp IS TRUE GOTO 5
// varstart: 0, varend: 8
// succ: 3, cond_succ: 5
// next: 3

// Block 3:
// isglobal: 0
// pred: 2, 
// 0		GOTO 7
// varstart: 0, varend: 8
// succ: 7, cond_succ: -1
// next: 4

// Block 4:
// isglobal: 0
// pred: 6, 
// 0		4_tmp = x++ i
// 1		GOTO 2
// varstart: 0, varend: 8
// succ: 2, cond_succ: -1
// next: 5

// Block 5:
// isglobal: 0
// pred: 2, 
// 0		5_tmp = x -int i
// 1		x = 5_tmp
// 2		PARAM .LC0
// 3		PARAM x
// 4		PARAM i
// 5		CALL printf 6_tmp
// varstart: 0, varend: 8
// succ: 6, cond_succ: -1
// next: 6

// Block 6:
// isglobal: 0
// pred: 5, 
// 0		GOTO 4
// varstart: 0, varend: 8
// succ: 4, cond_succ: -1
// next: 7

// Block 7:
// isglobal: 0
// pred: 3, 
// 0		7_tmp = $0
// 1		RETURN 7_tmp
// varstart: 0, varend: 8
// succ: -1, cond_succ: -1
// next: 8

// Block 8:
// isglobal: 0
// pred: 
// 0		<FUNC_END>
// varstart: 0, varend: 8
// succ: -1, cond_succ: -1
// next: -1

