// Backpatching and No Mulitple Gotos
int main(){
	int x = 1;
	int a = 2;
	int no_of_ifs_entered = 0;
	int no_of_elses_entered = 0;
	if(x==1){
		no_of_ifs_entered++;
		if(x>a){
			no_of_ifs_entered++;
		}
		else{
			no_of_elses_entered++;
		}
	}
	else{
		no_of_elses_entered++;
	}
	printf("no of ifs entered = %d, no of elses entered = %d\n", no_of_ifs_entered, no_of_elses_entered);
	return 0;

}