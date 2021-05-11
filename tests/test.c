int main(){
	char c[10];
	char d[10];
	char* e;
	c[0] = 's';
	c[1] = 'e';
	c[2] = 'a';
	c[3] = '\0';
	c[4] = '\0';

	d[0] = 's';
	d[1] = 'a';
	d[2] = 'a';
	d[3] = 'q';
	d[4] = '\0';
	e = strdup((char*)c);
	e[1] = 'F';
	printf("%d %s %d\n", strcmp((char*) c, (char*)d), c, strlen((char*)c));
}