#include "3AC.h"
#include "parse_utils.h"

vector<quad> code_array;
int nextquad = 0;

int emit(string op, qi op1, qi op2, qi res, int goto_addr){
    quad q (op, op1, op2, res, goto_addr);
    code_array.push_back(q);
    nextquad = code_array.size();
    return nextquad - 1;
}

qi getNewTemp(string type){
    static int var_no = 0;
    string var_name = to_string(var_no) + "_tmp";
    var_no++;
    qi q;
    q.first = var_name;
    q.second = add_entry(q.first, type, 0, offset.back(), IS_TEMP);
    return q;
}

void backpatch(vector<int>& list, int addr){
    for(auto i : list) 
        code_array[i].goto_addr = addr;
    list.clear();
}

qi emitConstant(node* tmp){
    qi temp = getNewTemp(tmp->node_data);
    qi temp1 = {"", NULL};

    switch(tmp->val_dt){
        case IS_INT:        temp1.first = to_string(tmp->val.int_const); break;
        case IS_LONG:       temp1.first = to_string(tmp->val.long_const); break;
        case IS_SHORT:      temp1.first = to_string(tmp->val.short_const); break;
        case IS_U_INT:      temp1.first = to_string(tmp->val.u_int_const); break;
        case IS_U_LONG:     temp1.first = to_string(tmp->val.u_long_const); break;
        case IS_U_SHORT:    temp1.first = to_string(tmp->val.u_short_const); break;
        case IS_FLOAT:      temp1.first = to_string(tmp->val.float_const); break;
        case IS_DOUBLE:     temp1.first = to_string(tmp->val.double_const); break;
        case IS_LONG_DOUBLE: temp1.first = to_string(tmp->val.long_double_const); break;
        case IS_CHAR:       temp1.first = "'" + to_string(tmp->val.char_const) + "'"; break;
    }

    emit("=", temp1, {"", NULL}, temp);
    return temp;
}
