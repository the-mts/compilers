// #include <stdio.h>
// #include <stdlib.h>
void main()
{
	long long int a_12 = 1;
	nene:
	a_12<<=a_12;
	a_12 = a_12>>2 ;
	if(a_12>=4)
	{
		printf("Hello\n");
		exit(0);
	}
	goto nene;
	return;
}