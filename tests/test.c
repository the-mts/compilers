struct point
{
    int x,y;
};

int te=20;

void foo(struct point l)
{
    struct point z;
    if(l.x>=te){
        return;
    }
    z.x=l.x+1;
    z.y=l.y*2;
    foo(z);
    return;
}

int main()
{
    int x=1;
    double y=(&x);
    struct point a;
    x=(x/y);
    a.x=x;
    a.y=x;
    foo(a);
    return 0;
}