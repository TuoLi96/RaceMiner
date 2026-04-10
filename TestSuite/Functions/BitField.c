struct BitStruct {
	int a;
	int b : 3;
	int c : 5;
	int d;
};

int func(struct BitStruct *s) {
	s->c = 2;
	return s->b;
}
