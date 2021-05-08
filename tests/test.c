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
	struct x* qq[2];
	qq[0] = &p;
	qq[1] = &p;
	// q->x = 12;
	// q->y = 33.333;
	// q->w = r;
	// r->s = 1234;
	(*(qq[0])).x;
	// qq[1]->x = 2;
	// printf("%d %d\n", qq[0]->x, qq[1]->x);
}