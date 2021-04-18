#include<vector>
#include<string>
#include "symtab.h"

using namespace std;

typedef pair<string, st_entry*> qi;

typedef struct quad {
    string op;
    qi op1;
    qi op2;
    qi res;
    int goto_addr = -1;
    quad (string op, qi op1, qi op2, qi res, int goto_addr = -1){
        this->op = op;
        this->op1 = op1;
        this->op2 = op2;
        this->res = res;
        this->goto_addr = goto_addr;
    }
} quad;

extern vector<quad> code_array;
extern int nextquad;

extern int emit(string op, qi op1, qi op2, qi res, int goto_addr = -1);
extern void backpatch(vector<int> list, int addr);
extern qi getNewTemp(string type);