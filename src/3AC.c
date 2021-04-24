#include <algorithm>
#include "3AC.h"
#include "parse_utils.h"

vector<quad> code_array;
int nextquad = 0;
vector<block> blocks;

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
    q.second = add_entry(q.first, type, get_size(type), offset.back(), IS_TEMP);
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


void make_blocks(){
	int n = code_array.size();
	if (n == 0) return;
	int leader[n];
	int blocknum[n];
	int curr = -1, prev;

	// Replace some obviously redundant GOTOs with DUMMY statements
	for (int i = 0; i < n-1; i++)
		if (code_array[i].goto_addr == i+1) {
			code_array[i].op = "DUMMY";
			code_array[i].op1 = {"", NULL};
			code_array[i].op2 = {"", NULL};
			code_array[i].res = {"", NULL};
			code_array[i].goto_addr = -1;
		}

	// Finding leaders

	leader[0] = 1;
	for (int i = 1; i < n; i++)	leader[i] = 0;
	for (int i = 0; i < n-1; i++){
		if (code_array[i].goto_addr > -1){
			leader[i+1] = 1;
			leader[code_array[i].goto_addr] = 1;
		}
		else if (code_array[i].op == "FUNC_START") leader[i] = 1;
		else if (code_array[i].op == "RETURN" || code_array[i].op == "RETURN_VOID") leader[i+1] = 1;
	}
	if (code_array[n-1].goto_addr > -1) leader[code_array[n-1].goto_addr] = 1;
	if (code_array[n-1].op == "FUNC_START") leader[n-1] = 1;

	for (int i = 0; i < n; i++){
		if (leader[i]) curr++;
		blocknum[i] = curr;
	}

	curr = 0;
	blocks.push_back(block(0));
	for (int i = 0; i < n; i++){
		if (curr < blocknum[i]){
			if (code_array[i-1].op == "GOTO")
				blocks[curr].succ = blocknum[code_array[i-1].goto_addr];
			else if (code_array[i-1].goto_addr != -1)
				blocks[curr].cond_succ = blocknum[code_array[i-1].goto_addr];
			else if (code_array[i-1].op == "RETURN" || code_array[i-1].op == "RETURN_VOID") 
				blocks[curr].succ = -1;
			curr++;
			blocks.push_back(block(curr));
		}
		blocks[curr].code.push_back(code_array[i]);
	}
	if (code_array[n-1].op == "GOTO")
		blocks[curr].succ = blocknum[code_array[n-1].goto_addr];
	else if (code_array[n-1].goto_addr != -1)
		blocks[curr].cond_succ = blocknum[code_array[n-1].goto_addr];
	else
		blocks[curr].succ = -1;
	
	for (int i = 0; i < blocks.size(); i++){
		if (blocks[i].succ != -1) blocks[blocks[i].succ].pred.push_back(i);
		if (blocks[i].cond_succ != -1) blocks[blocks[i].cond_succ].pred.push_back(i);
		blocks[i].code.erase(remove_if(blocks[i].code.begin(), blocks[i].code.end(), [](quad q){return q.op == "DUMMY";}), blocks[i].code.end());
	}
	
	for (int i = 0; i < blocks.size(); i++){
		if (blocks[i].code.size() == 0){
			for (auto j: blocks[i].pred){
				if (blocks[j].succ == i){
					blocks[j].succ = blocks[i].succ;
					blocks[blocks[i].succ].pred.push_back(j);
				}
				else if (blocks[j].cond_succ == i){
					blocks[j].cond_succ = blocks[i].succ;
					blocks[blocks[i].succ].pred.push_back(j);
				}
				else {
					//cout << "Something went horribly wrong and I don't know what" << endl;
					exit(-1);
				}
			}
			blocks[i].pred.clear();
		}
	}
}
