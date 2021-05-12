int x, y, z, w;

int main(){
	x = 8; w = 10;
	z = 5; y = 7;
	if (y) 
		y = x;
	else 
		y = w;
}


/*
Block 2:
alive
isglobal: 0
pred: 1, 
0		x = $8
1		w = $10
2		z = $5
3		y = $8
succ: 6, cond_succ: -1
next: 6

Block 6:
alive
isglobal: 0
pred: 2, 
0		<FUNC_END>
succ: -1, cond_succ: -1
next: -1

*/
