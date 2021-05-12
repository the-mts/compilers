
int main(){
	char a[100];
	char b[100];
	int i, len, comp;
	char * c;
	for(i = 0; i < 5; i++){
		a[i] = 'a' + i;
		b[i] = 'z' - i;
	}
	a[5] = '\0';
	b[5] = '\0';
	len = strlen((char *)a);
	printf("length of a is %d\n", len);
	comp = strcmp((char *)a, (char *)b);
	printf("value of comp is %d\n",comp);
	strcat((char *)a, (char *)b);
	printf("a and b after strcat: %s %s\n", a, b);
	strcpy((char *)b, (char *)a);
	printf("a and b after strcpy: %s %s\n", a, b);
	c = strchr((char *) a, 'c');
	printf("'c' found index %d\n", (int)(c - (char *)a));
	c = strdup((char *)b);
	c[0] = 'A';
	printf("b and c after strdup: %s %s\n", b, c);



	

}