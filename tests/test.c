/*#include<stdio.h>*/
void fun(int x[][10]){
	x[1]++;
}

int main(){
	int *x, y=2;
	/*short int b = 5;*/
	/*a &= b;*/
	/*int c = a & b;*/ 
	/*a+=10;*/
	/*int e = a*++a;*/
	/*int e = a ^ b;*/
	int xx[2][10];




	x = &y;
	fun(xx);/*
	printf("%p %p", x, ++x);
	printf("%p %p", x, ++x);*/
	/*printf("%d\n", f);	
	*/return 0;
}
/*[]
CALL
(-to-)
PARAM
++ (Pre/Post)
-- ''
UNARY&
UNARY*
!
UNARY+
UNARY-
~
inttoreal
*int
*real
/int
/real
%
+int
+real
-int
-real
<<
>>
<int
<real
>int
>real
<=int,real
>=int,real
==int,real
!=int,real
&
^
|
IF_TRUE_GOTO
GOTO
=
+=int,real
-=...
*=...
/=...
%,<<,>>,&,^,|=
RETURN_VOID
RETURN
FUNC_START*/