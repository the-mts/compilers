int main(){
	int x = 0;
	int flag = 0;
	if(1 || 1/x){
		printf("Short Circuiting Working Perfectly (1)\n");
	}
	if(0 && 1/x){
		flag = 1;
	}
	printf("%d\n", flag);

	if(1 && (1 || 1/x)){
		printf("Short Circuiting Working Perfectly (2)\n");
	}
}
