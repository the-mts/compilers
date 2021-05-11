int main(){
	int a, b, c ,d;
	a = 1;
	b = 2;
	c = 3; // DNE
	d = 4; // DNE
	if (1){
		a = a + b;
	}
	// Entire Block DNE
	if (0) {  
		c = c + d;
	}
	
	printf("%d\n", a);
return 0;
}
