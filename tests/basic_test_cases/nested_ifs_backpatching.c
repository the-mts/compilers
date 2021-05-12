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
//Generated 3AC
/*0		<main>:
1		0_tmp = $1
2		x = 0_tmp
3		1_tmp = $2
4		a = 1_tmp
5		2_tmp = $0
6		no_of_ifs_entered = 2_tmp
7		3_tmp = $0
8		no_of_elses_entered = 3_tmp
9		4_tmp = $1
10		5_tmp = x ==int 4_tmp
11		IF 5_tmp IS TRUE GOTO 13
12		GOTO 21
13		6_tmp = x++ no_of_ifs_entered
14		7_tmp = x >int a
15		IF 7_tmp IS TRUE GOTO 17
16		GOTO 19
17		8_tmp = x++ no_of_ifs_entered
18		GOTO 22
19		9_tmp = x++ no_of_elses_entered
20		GOTO 22
21		10_tmp = x++ no_of_elses_entered
22		PARAM .LC0
23		PARAM no_of_ifs_entered
24		PARAM no_of_elses_entered
25		CALL printf 11_tmp
26		12_tmp = $0
27		RETURN 12_tmp
28		<FUNC_END>
*/