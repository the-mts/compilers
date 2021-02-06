int main()
{
	for(int i=0; i<=5;i++ )
	{
		int j = 5;
		while( j<10)
		{
			j*=2 ;
			j/= 2;
			j += 1;
			j++ ;
			j-- ;
		}
		int x = 2;
		   switch(x)
		{
			case 1:		x*=2;
						break;
			case 2:		x/=2;
			case 3:		x==0;
						break;
			default:	x=0;
		}
	}
	printf("Success\n");
}