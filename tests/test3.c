int random = 10;

enum week{Mon, Tue, Wed, Thur, Fri, Sat, Sun}; 

struct Time{
    int hour, minute, second;
} time;

union Node{
    int a[20];
    char ch;
    long long b;
};

int main(){

    extern int a;

    int i = 0;
    union Node node;
    struct Time *t;
    struct Time t1;
    int *ptr1 = &i;
    int **ptr2 = &ptr1;

    while(i<5){
        static volatile int count = 3;
        count++;
        printf("count = %d, i = %d\n",count,i);
        i++;
    }

    tim = (struct Time*) malloc(sizeof(struct Time)); 
    tim->hour = Tue;
    tim->minute = Wed;
    tim->second = Thurs;

    printf("Memory occupied by union : %d\n", sizeof(node));
    printf("Memory occupied by struct : %d\n", sizeof(t1));
    printf("Memory occupied by long long int :%d\n", sizeof(long long int));
    printf("Memory occupied by long long int* :%d\n", sizeof(long long int*));

    return 0;
}