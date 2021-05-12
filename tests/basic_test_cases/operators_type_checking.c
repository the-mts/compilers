// Operator Type Checking, Pointer Arithmetic and Type Casting
int main(){
	int a = 10;
	int * b = &a;
	long int c = 5;
	float d = 7.5;

	int * should_work_0 = b + c; // adding long int to pointer
	int should_work_1 = c + d; // adding long int and float (and storing in an int variable)
	int diff = should_work_0 - b;
	printf("Value of b is %p\n", b); 
	printf("%p %d\n", should_work_0, should_work_1);
	printf("Difference between the values of b and should_work_0 is: %d\n", diff); // Taking into account the pointer arithmetic (20/4 = 5)

	// int should_not_work_0 = b + d; // adding float to pointer
	// int should_not_work_1 = b * b; .// multiplying two pointers

}