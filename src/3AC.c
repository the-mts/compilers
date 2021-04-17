#include "3AC.h"

vector<quad> code_array;
int nextquad = 0;

int emit(string op, qi op1, qi op2, qi res, int goto_addr){
    quad q (op, op1, op2, res, goto_addr);
    code_array.push_back(q);
    nextquad = code_array.size();
    return nextquad - 1;
}

qi getNewTemp(){
    static int var_no = 0;
    string var_name = to_string(var_no) + "_tmp";
    var_no++;
    qi q;
    q.first = var_name;
    q.second = NULL;
    return q;
}

void backpatch(vector<int> list, int addr){
    for(auto i : list) 
        code_array[i].goto_addr = addr;
}


// For debugging purposes
// Have to rewrite
void print_quad(quad q){
    cout<<q.op<<' '<<q.op1.first<<' '<<q.op2.first<<' '<<q.res.first<<' '<<q.goto_addr;
}

// For debugging purposes
// Have to rewrite
void print_code(){
    for(auto i: code_array){
        print_quad(i);
        cout<<endl;
    }
}