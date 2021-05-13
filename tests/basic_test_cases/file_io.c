int main(){
	FILEP inf = fopen("../input.txt", "r");
	FILEP outf = fopen("../output.txt", "w+");
	int a[10];
	int i;
	char c[100];
	fscanf(inf, "%s", c);
	for(i=0; i<10; i++){
		fscanf(inf, "%d", &a[i]);
	}
	fclose(inf);
	for(i=0; i<50; i++){
		fprintf(outf, " ");
	}
	fprintf(outf, "\n\n");
	
	for(i=0; i<10; i++){
		fprintf(outf, "Array[%d] = %d\n", i, a[i]);
	}

	fseek(outf, 0, 0);
	fprintf(outf, "String inserted at top: %s", c);
	
	fclose(outf);
}