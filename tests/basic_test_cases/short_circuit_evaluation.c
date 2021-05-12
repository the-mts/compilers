// Short Circuit Evaluation
int main(){
	int x = 0;
	int flag = 0;
	if(1 || 1/x){
		printf("Short circuiting working fine\n");
	}
	if(0 && 1/x){
		flag = 1;
	}
	printf("Flag is: %d\n", flag);

	if(1 && (1 || 1/x)){
		printf("Short circuiting working perfectly\n");
	}
}
