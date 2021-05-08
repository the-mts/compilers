struct y{
	int s;
};

struct x {
	int x;
	float y;
	struct y* w;
};

int main(){
	struct y t;
	struct y* r = &t;
	struct x p;
	struct x* q = &p;
	q->x = 12;
	q->y = 33.333;
	q->w = r;
	r->s = 1234;
	printf("%d %f %d\n", q->x, q->y, q->w->s);
}