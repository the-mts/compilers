int fun(){
	int z[2];
	return z;
}
int main(){
	int x[5];
	// int * y = x;
	int y[5];
	y[2] = fun();
	return 0;
}


