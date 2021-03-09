int main()
{
	int x = 1, y = 0, z;
	int x1, x2;
	x = x & y;
	x = !x;
	z = 15, x = 2;
	z >>= 2;
	z = z<<3;
	if(x)
		z = 12;
	else
		z = z*(!y);
	x = x?x:y;
	{
		int i=0, j;
		for(i=1;i<=5;i++)
		{
			x*=1;
		}
		while((i=5) && j==6)
		{
			x/=2;
		}
	}
}
