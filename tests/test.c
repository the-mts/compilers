int main(){
	int y = 5;
	int yy = 10;
	int * x = &y;
	int * xx = &yy;
	printf("Akhilesh lodu %p  %d\n", x, *(x-2));
}