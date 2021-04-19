// #include<stdio.h>

int printf(){
	;
}

int main(){
	int x = 5;
	int y;
	for(x=1;x<5;x-=1){
	switch(x){
		default:{y = 0;}
		case 2: y = 2;
		break;
		case 3: y = 3;
		continue;
	}
	y++;
	}
	// for(x; x>3;){
	// 	if(y) break;
	// 	else {break;}
	// 	x++;
	// 	break;
	// }
	x++;
}