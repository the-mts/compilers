// struct y{
// 	int s;
// };

struct x {
	int x;
	float y;
	// struct y* w;
};

int main(){
	// struct y t;
	// struct y* r = &t;
	struct x p;
	// struct x* q = &p;
	struct x* qq[2];
  // t.s = 7;
	int xx = -1;
	qq[0] = &p;
	// qq[1] = &p;

	// q->x = 12;
	// q->y = 33.333;
	// q->w = r;
	// // r->s = 1234;
	// // (*(qq[0])).x;
	qq[0]->x = 15;
	// xx = (qq[0] == &p);
	printf("%d %d %p %p\n", qq[0]->x, p.x, &p, qq[0]);
	// printf("%d\n", xx);
	return 0;
}