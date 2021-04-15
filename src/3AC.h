#include <list>

typedef pair<string, st_entry*> qi;

typedef struct {
    string op;
    qi op1, op2;
    qi res;
} quad;

extern vector<quad> code_array;

extern int emit(qi res, string op, qi op1, qi op2);
extern int nextquad;