int main()
{
	int _abc_123=0, x = _abc_123;
	//Comment Test
	/*MultiLine 
	Comment 
	Test*/
	if(x>=_abc_123)
	{
		if(x<_abc_123)
		{
			printf("Nested If\n");
		}
		else
		{
			printf("Success\n");
		}
	}
	else
	{
		printf("Failure\n");
	}
	x*=_abc_123;
	x+=_abc_123;
	x = x + _abc_123;
	printf("%d\n", _abc_123);
}
