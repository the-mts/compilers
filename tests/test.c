struct x;
struct x {
    int a;
    float b;
    char c;
};

struct y {
    int akhilesh, chutiya;
    struct x a;
};


int fun(int* a){
    return a;
}

int main(){
    struct x p;
    +p;
    // fun(&a);
}
