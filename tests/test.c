/*#include<stdio.h>*/
int main(){
	unsigned int a = 8;
	unsigned int b = 2;
	/*short int b = 5;*/
	/*a &= b;*/
	/*int c = a & b;*/ 
	/*a+=10;*/
	/*int e = a*++a;*/
	/*int e = a ^ b;*/
	int f = a >> b;
	// printf("%d\n", f);	
	return 0;
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