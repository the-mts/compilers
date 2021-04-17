typedef pair<string, st_entry*> qi;

typedef struct quad {
    string op;
    qi op1;
    qi op2;
    qi res;
    int goto_addr = -1;
} quad;

extern vector<quad> code_array;
extern int nextquad;

extern int emit(string op, qi op1, qi op2, qi res, int goto_addr = -1);
extern void backpatch(vector<int> list, int addr);
extern qi getNewTemp();