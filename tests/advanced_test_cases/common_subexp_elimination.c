// compile time constant evaluation and common subexpression elimination

int a, b, c, d, e, f, g;	
float x, y;
int main(){
	a = (3+4)*2;
	b = !((3+3)*9+8);
	c = a + b;
	d = a + b;
	a = d;
	e = a + b;
	f = a + b;
	e = ++a;
	f = ++a;
	g = d/c;
	a = b;
	f = d/c;
}

/*
basic blocks->

0		a = $14
1		b = $0
2		2_tmp = a +int b
3		c = 2_tmp
4		d = 2_tmp
5		a = 2_tmp
6		6_tmp = ++x a
7		e = 6_tmp
8		8_tmp = 2_tmp /int 2_tmp
9		g = 8_tmp
10		a = $0
11		f = 8_tmp

*/