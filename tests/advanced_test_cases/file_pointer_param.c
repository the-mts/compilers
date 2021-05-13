FILEP fun(FILEP x){
	fprintf(x, "hi3\n");
	x = fopen("../compilers/file2.txt", "w+");
	fprintf(x, "hi4\n");
	return x;
}

int main(){
	FILEP x, y;
	x = fopen("../compilers/file1.txt", "w+");
	fprintf(x, "hi1\n");
	y = x;
	fprintf(y, "hi2\n");
	x = fun(x);
	fprintf(x, "hi5\n");
}