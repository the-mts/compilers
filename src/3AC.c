#include <algorithm>
#include <map>
#include<iostream>
#include <stdio.h>
#include <numeric>
#include "3AC.h"
#include "parse_utils.h"

vector<quad> code_array;
int nextquad = 0;
vector<block> blocks;
unordered_map<string, pair<string, string>> constLabels;
int var_no = 0;

string gen_new_const_label(){
	static int a=0;
	return ".LC"+to_string(a++);
}

int emit(string op, qi op1, qi op2, qi res, int goto_addr){
    quad q (op, op1, op2, res, goto_addr);
    code_array.push_back(q);
    nextquad = code_array.size();
    return nextquad - 1;
}

qi getNewTemp(string type, tt_entry* ttentry){
    //static int var_no = 0;
    string var_name = to_string(var_no) + "_tmp";
    var_no++;
    qi q;
    q.first = var_name;
    q.second = add_entry(q.first, type, get_size(type), accumulate(offset.begin()+1, offset.end(), 0), IS_TEMP);
    q.second->ttentry = ttentry;
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
        case IS_INT:        
			temp1.first = "$" + to_string(tmp->val.int_const);
			break;
        case IS_LONG:       
			temp1.first = "$" + to_string(tmp->val.long_const);
			break;
        case IS_SHORT:
			temp1.first = "$" + to_string(tmp->val.short_const);
			break;
        case IS_U_INT:      
			temp1.first = "$" + to_string(tmp->val.u_int_const);
			break;
        case IS_U_LONG:
			temp1.first = "$" + to_string(tmp->val.u_long_const);
			break;
        case IS_U_SHORT:
			temp1.first = "$" + to_string(tmp->val.u_short_const);
			break;
        case IS_CHAR:
			temp1.first = "$" + to_string( (int) tmp->val.char_const);
			break;
        case IS_FLOAT:
			{
				temp1.first = gen_new_const_label();
				unsigned int* xf = (unsigned int*) &tmp->val.float_const;
				unsigned int yf = *xf;
				constLabels[temp1.first] = {".long", ".long "+to_string(yf)};
			}
			break;
        case IS_DOUBLE:
			{	
				temp1.first = gen_new_const_label();
				unsigned long long* xd = (unsigned long long*) &tmp->val.double_const;
				unsigned long long yd = *xd;
				constLabels[temp1.first] = {".quad", ".quad "+to_string(yd)};
			}
			break;
        case IS_LONG_DOUBLE:
			{
				temp1.first = gen_new_const_label();
				unsigned int* xld = (unsigned int*) &tmp->val.long_double_const;
				unsigned int a1 = *xld;
				unsigned int a2 = *(xld+1);
				unsigned int a3 = *((unsigned int*)((short*)(xld+2)));
				constLabels[temp1.first] = {".t", ".long "+to_string(a1)+"\n.long"+to_string(a2)+"\n.long"+to_string(a3)+"\n.long 0"};
			}
			break;
    }

    emit("=", temp1, {"", NULL}, temp, -2);
    return temp;
}

int tailposs(qi caller, qi callee){
	if (caller.first != callee.first) return 0;
	int fcount=0, icount= 0;
	for (auto arg: *(caller.second->arg_list)){
		if (is_struct_or_union(arg.first.first)){
			return 0;
		}
		if (arg.first.first == "float" || arg.first.first == "double") fcount++;
		else icount++;
	}
	return fcount < 16 && icount < 14;
}

void handle_dead_block(int b){
	int temp, f, s;
	if (blocks[b].code.size() == 0){
		blocks[b].alive = 0;
		blocks[b-1].next = blocks[b].next;
		for (auto j: blocks[b].pred){
			f = 1;
			if (blocks[j].succ == b){
				f = 0;
				if (blocks[j].code.back().op == "GOTO"){
					blocks[j].code.back().goto_addr = blocks[b].succ;
				}
				blocks[j].succ = blocks[b].succ;
				if (blocks[b].succ != -1) {
					blocks[blocks[b].succ].pred.push_back(j);
				}
			}
			if (blocks[j].cond_succ == b){
				f = 0;
				blocks[j].code.back().goto_addr = blocks[b].succ;
				blocks[j].cond_succ = blocks[b].succ;
				if (blocks[b].succ != -1) blocks[blocks[b].succ].pred.push_back(j);
			}
			if (f){
				printf("Something went horribly wrong in handle_dead_block(%d)", b);
				exit(-1);
			}
		}
		if (blocks[b].succ != -1){
			s = blocks[b].succ;
			blocks[s].pred.erase(remove(blocks[s].pred.begin(), blocks[s].pred.end(), b), blocks[s].pred.end());
			blocks[b].succ = -1;
		}
		blocks[b].pred.clear();
		temp =b;
		do {
			temp--;
			blocks[temp].next = blocks[temp+1].next;
		}while(!blocks[temp].alive);
	}
}

void make_blocks(){
//	printf("chick0\n");
	int n = code_array.size();
	if (n == 0) return;
	int leader[n];
	int blocknum[n];
	int curr = -1, s, f=0;
 	int vstart, vend, prev;	
//	printf("chick1\n");
	// Replace some obviously redundant GOTOs with DUMMY statements
	qi func;	
	int tail = 1;
	if (tail) {
		for (int i = 0; i < n-1; i++){
			if (code_array[i].op == "GOTO" || code_array[i].op == "IF_TRUE_GOTO"){
				if (code_array[i].goto_addr == i+1) {
					code_array[i].op = "DUMMY";
				}
			}
			else if(code_array[i].op == "CALL"){
				if (code_array[i+1].op == "RETURN" && code_array[i].res.first == code_array[i+1].op1.first && tailposs(func, code_array[i].op1)){
					code_array[i+1].op = "TAIL";
					code_array[i+1].op1 = code_array[i].op1;
					code_array[i].op = "DUMMY";
				}
			}
			else if (code_array[i].op == "FUNC_START"){
				func = code_array[i].op1;
			}
		}
	}
	else {
		for (int i = 0; i < n-1; i++){
			if ((code_array[i].op == "GOTO" || code_array[i].op == "IF_TRUE_GOTO") && code_array[i].goto_addr == i+1) {
				code_array[i].op = "DUMMY";
				/*code_array[i].op1 = {"", NULL};
				code_array[i].op2 = {"", NULL};
				code_array[i].res = {"", NULL};
				code_array[i].goto_addr = -1;*/
			}
		}
	}

	// Finding leaders
//	printf("chick2\n");
	
	leader[0] = 1;
	for (int i = 1; i < n; i++)	leader[i] = 0;
	for (int i = 0; i < n-1; i++){
		if (code_array[i].op== "IF_TRUE_GOTO" || code_array[i].op== "GOTO"){
			leader[i+1] = 1;
			leader[code_array[i].goto_addr] = 1;
		}
		else if (code_array[i].op == "FUNC_START" || code_array[i].op == "FUNC_END") {
			leader[i] = 1;
			leader[i+1] = 1;
		}
		else if (code_array[i].op == "RETURN" || code_array[i].op == "RETURN_VOID" || code_array[i].op == "CALL" || code_array[i].op == "TAIL") leader[i+1] = 1;	
	}
	if (code_array[n-1].op == "IF_TRUE_GOTO" || code_array[n-1].op == "GOTO") leader[code_array[n-1].goto_addr] = 1;
	else if (code_array[n-1].op == "FUNC_END") leader[n-1] = 1;
	for (int i = 0; i < n; i++){
		if (leader[i]) curr++;
		blocknum[i] = curr;
	}
//	printf("chick3\n");
	
	curr = 0;
	blocks.push_back(block(0));
	if (code_array[0].op == "FUNC_START") {
		f = 1;
		//vstart = code_array[0].goto_addr;
		prev = 1;
	}
	for (int i = 0; i < n; i++){
		if (curr < blocknum[i]){
			blocks[curr].isglobal = !f;
			if (blocks[curr].isglobal) blocks[curr].pred.push_back(-1);
			if (code_array[i-1].op == "GOTO"){
				blocks[curr].succ = blocknum[code_array[i-1].goto_addr];
				blocks[curr].code[blocks[curr].code.size()-1].goto_addr = blocks[curr].succ;
			}
			else if (code_array[i-1].op == "IF_TRUE_GOTO"){
				blocks[curr].cond_succ = blocknum[code_array[i-1].goto_addr];
				blocks[curr].code[blocks[curr].code.size()-1].goto_addr = blocks[curr].cond_succ;			
			}
			else if (code_array[i-1].op == "RETURN" || code_array[i-1].op == "RETURN_VOID")
				blocks[curr].succ = -1;
			else if (code_array[i-1].op == "TAIL") {
				blocks[curr].succ = blocknum[prev];
				blocks[curr].code[blocks[curr].code.size()-1].goto_addr = blocknum[prev];
			}
			else if (code_array[i-1].op == "FUNC_END"){
				blocks[curr].succ = -1;
				f = 0;
				/*for (int j = prev; j <= curr; j++){
					blocks[j].varend = code_array[i-1].goto_addr;
					blocks[j].varstart = vstart;
				}*/
			}
			curr++;
			blocks.push_back(block(curr));
			if (code_array[i].op == "FUNC_START") {
				f = 1;
				//vstart = code_array[i].goto_addr;
				prev = curr+1;
			}
		}
		blocks[curr].code.push_back(code_array[i]);
	}
	/*if (code_array[n-1].op == "GOTO"){
		blocks[curr].succ = blocknum[code_array[n-1].goto_addr];
		blocks[curr].code[blocks[curr].code.size()-1].goto_addr = blocks[curr].succ;		
	}
	else if (code_array[n-1].op == "IF_TRUE_GOTO"){
		blocks[curr].cond_succ = blocknum[code_array[n-1].goto_addr];
		blocks[curr].code[blocks[curr].code.size()-1].goto_addr = blocks[curr].cond_succ;
	}
	else{
		if (code_array[n-1].op == "FUNC_END") {
			for (int j = prev; j <= curr; j++){
				blocks[j].varend = code_array[n-1].goto_addr;
				blocks[j].varstart = vstart;
			}
		}
		blocks[curr].succ = -1;
	}*/
	blocks[curr].succ = -1;
	blocks[curr].next = -1;
	blocks[curr].isglobal = !f;
	if (blocks[curr].isglobal) blocks[curr].pred.push_back(-1);

//	printf("chick4\n");
	//return;
	for (int i = 0; i < blocks.size(); i++){
		if (blocks[i].succ != -1) {
			blocks[blocks[i].succ].pred.push_back(i);
			if (blocks[i].cond_succ != -1) blocks[blocks[i].cond_succ].pred.push_back(i);
		}
		//blocks[i].code.erase(remove_if(blocks[i].code.begin(), blocks[i].code.end(), [](quad q){return q.op == "DUMMY";}), blocks[i].code.end());
	}
//	printf("chick5\n");
	//return;
	for (int i = blocks.size()-1; i >= 0; i--){
		handle_dead_block(i);		
	}
//printf("chick6\n");

}

void opt_ret_dead(){
	int c = 1, s;
	int temp;
	int lim = 10;
	while (lim-- && c) {
		c = 0;
		for (int b = 0; b != -1; b = blocks[b].next){
			//if (blocks[b].pred.size() == 0 && blocks[b].alive && blocks[b].code[0].op != "FUNC_START" && blocks[b].code[0].op != "FUNC_END"){
			if (blocks[b].pred.size() == 0 && blocks[b].alive && blocks[b].code[0].op != "FUNC_START"){

				blocks[b].alive = 0;
				blocks[b].code.clear();
				if (blocks[b].succ != -1){
					s = blocks[b].succ;
					blocks[s].pred.erase(remove(blocks[s].pred.begin(), blocks[s].pred.end(), b), blocks[s].pred.end());
					blocks[b].succ = -1;
					c = 1;
					if (blocks[b].cond_succ != -1){
						s = blocks[b].cond_succ;
						blocks[s].pred.erase(remove(blocks[s].pred.begin(), blocks[s].pred.end(), b), blocks[s].pred.end());
						blocks[b].cond_succ = -1;
					}
				}
				temp = b;
				do {
					temp--;
					blocks[temp].next = blocks[temp+1].next;
				}while(!blocks[temp].alive);
			}
		}
	}
}

void print_map(const map<pair<string, pair<string, string>>, qi>& m)
{
	   for (const auto& [key, value] : m) {
	        cout <<"(" << key.first<< ", "<<key.second.first<<", "<< key.second.second<< ") = " << value.first << "; ";
	   }
	   cout<<"\n";
}

int opt_cse(){
	map<pair<string, pair<st_entry*, st_entry*>>, qi> expr;
	//map<string, vector<pair<string, pair<string, string>>>> used;
	int i;
	string op;
	st_entry *op1, *op2, *res;
	//cout<<"chick1\n";
	//print_map(expr);
	int c = 0;
	for (int b = 0; b != -1; b = blocks[b].next){
		expr.clear();
		i = 0;
		if (blocks[b].isglobal || blocks[b].code[0].op == "FUNC_START" || blocks[b].code[0].op == "FUNC_END") continue;
		for (; i < blocks[b].code.size(); i++){
			op = blocks[b].code[i].op;
			op1 = blocks[b].code[i].op1.second;
			op2 = blocks[b].code[i].op2.second;
			res = blocks[b].code[i].res.second;
			if (op == "IF_TRUE_GOTO" || op == "GOTO" ||
				op == "PARAM" || op == "CALL" || op == "TAIL" ||
				op == "RETURN" || op == "RETURN_VOID") break;
			if (expr.find({op, {op1, op2}}) != expr.end()){
				//cout<<"Found expr\n";
				blocks[b].code[i].op = "=";
				blocks[b].code[i].op1 = expr[{op, {op1, op2}}];
				blocks[b].code[i].op2 = {"", NULL};
				c = 1;
			}
			else if (op == "ADDR="){
				expr.clear();
			}
			else if (op != "UNARY*" && op != "="){
				//cout<<"Adding expr\n";
				expr[{op, {op1, op2}}] = blocks[b].code[i].res;	
			}
			//Remove all entries in expr which have 
			//for (auto v: used[res])
			//	expr.erase(v);

			auto pred = [&](const auto& item) {
        		auto const& [key, value] = item;
		        return (key.second.first == res || key.second.second == res) && key.first != "UNARY&";
			};

			for (auto i = expr.begin(), last = expr.end(); i != last; ) {
			  if (pred(*i)) {
			      i = expr.erase(i);
			  } else {
			      ++i;
			  }
			}
		}
		//print_map(expr);
		//cout<<"chick2\n";
	}
	return c;
}

void print_copy_map(const map<string, qi>& m)
{
	   for (const auto& [key, value] : m) {
	        cout << key << " = " << value.first << "; ";
	   }
	   cout<<"\n";
}

int opt_copy(){
	map<st_entry*, qi> expr;
	//map<string, vector<pair<string, pair<string, string>>>> used;
	int i;
	int c = 0;
	string op;
	st_entry *op1, *op2, *res;
	//cout<<"chick1\n";
	//print_copy_map(expr);
	for (int b = 0; b != -1; b = blocks[b].next){
		expr.clear();
		i = 0;
		if (blocks[b].code[0].op == "FUNC_START" || blocks[b].code[0].op == "FUNC_END") continue;
		for (; i < blocks[b].code.size(); i++){
			op = blocks[b].code[i].op;
			op1 = blocks[b].code[i].op1.second;
			op2 = blocks[b].code[i].op2.second;
			res = blocks[b].code[i].res.second;
			if (op == "GOTO" || op == "CALL" || op == "TAIL" ||
			    op == "RETURN_VOID") break;
			if (expr.find(op1) != expr.end() && (op == "=" || (expr[op1].first)[0] != '$' && (expr[op1].first)[0] != '.')){
				//cout<<"Found expr\n";
				blocks[b].code[i].op1 = expr[op1];
				c = 1;
			}
			if (expr.find(op2) != expr.end() && expr[op2].first[0] != '$' && expr[op2].first[0] != '.'){
				blocks[b].code[i].op2 = expr[op2];
				c = 1;
			}
			if (op == "="){
				//if (blocks[b].code[i].op1.first[0] != '.')
				expr[res] = blocks[b].code[i].op1;
				auto pred = [&](const auto& item) {
    	    		auto const& [key, value] = item;
				    return value.second == res;
				};
	
				for (auto i = expr.begin(), last = expr.end(); i != last; ) {
			  		if (pred(*i)) {
				      i = expr.erase(i);
			  		} else {
			    	  ++i;
			 		}
				}
			}
			else if (op == "ADDR=") expr.clear();
			else {
				auto pred = [&](const auto& item) {
  	    			auto const& [key, value] = item;
			        return value.second == res || key == res;
				};

				for (auto i = expr.begin(), last = expr.end(); i != last; ) {
			  		if (pred(*i)) {
				    	i = expr.erase(i);
			  		} else {
			      		++i;
			  		}
				}
			}		
		}
		//print_copy_map(expr);
		//cout<<"chick2\n";
	}
	return c;
}

int istemp(qi v){
	return v.first[0] >= 48 && v.first[0] <= 57;
}

int opt_dead_expr(){
	int n = blocks.size();
	vector<map<qi, int>> gtemp(n);
	map<qi, int> user;
	map<qi, int> temp;
	int l, e;
	int c = 0;
	qi var, op;
	for (int b = n-1; b >= 0; b--){
		if (blocks[b].alive && blocks[b].code[0].op != "FUNC_END" && blocks[b].code[0].op != "FUNC_START"){
			user.clear();
			temp.clear();
			if (!blocks[b].isglobal) {
				/*if ((blocks[b].succ != -1 && blocks[b].succ <= b) || (blocks[b].cond_succ != -1 && blocks[b].cond_succ <= b)){
					for (int l = blocks[b].varstart; l < blocks[b].varend; l++)
					temp[to_string(l)+"_tmp"] = 1;
				}
				else*/ 
				if (blocks[b].succ != -1 && blocks[b].succ > b){
					temp = gtemp[blocks[b].succ];
					if (blocks[b].cond_succ != -1 && blocks[b].cond_succ > b){
						for (auto s: gtemp[blocks[b].cond_succ])
							if (s.second) temp[s.first] = 1;
					}
				}
			
				l = blocks[b].code.size() - 1;
				/*if (blocks[b].succ == -1 || (blocks[b].cond_succ == -1 && blocks[blocks[b].succ].code[0].op == "FUNC_END")){
					for (int i = 0; i <= l; i++){
						var = blocks[b].code[i].op1;
						if (var.first != "" && !istemp(var)) user[var] = 0;
						var = blocks[b].code[i].op2;
						if (var.first != "" && !istemp(var)) user[var] = 0;
						var = blocks[b].code[i].res;
						if (var.first != "" && !istemp(var)) user[var] = 0;
					}
				}*/
				if (blocks[b].code[l].op == "RETURN_VOID" || blocks[b].code[l].op == "GOTO") l--;
				else if (blocks[b].code[l].op == "RETURN" || blocks[b].code[l].op == "IF_TRUE_GOTO") {
					var = blocks[b].code[l].op1;
					if (istemp(var)) temp[var] = 1;
					else user[var] = 1;
					l--;
				}
				else if (blocks[b].code[l].op == "CALL" || blocks[b].code[l].op == "TAIL"){
					//var = blocks[b].code[l].res.first;
					//temp[var] = 0;
					l--;
					while (l>=0 && blocks[b].code[l].op == "PARAM") {
						var = blocks[b].code[l].op1;
						if (istemp(var)) temp[var] = 1;
						else user[var] = 1;
						l--;
					}
				}
			}
			else l = blocks[b].code.size() - 1;

			for (;l >= 0; l--){
				var = blocks[b].code[l].res;
				op = blocks[b].code[l].op1;
				if (blocks[b].code[l].op == "ADDR="){
					if (istemp(op)) temp[op] = 1;
					else user[op] = 1;
					if (istemp(var)) temp[var] = 1;
					else user[var] = 1;
				}
				else if(blocks[b].code[l].op == "UNARY*"){
					if (istemp(var)){
						if (temp.find(var) == temp.end() || temp[var] == 0){
							blocks[b].code[l].op = "DUMMY";
							c = 1;
						}
						else {
							temp[var] = 0;
							user.clear();
							for (int i = l-1; i >= 0 && blocks[b].code[i].op != "UNARY*"; i--){
								var = blocks[b].code[i].res;
								if (istemp(var)) temp[var] = 1;
							}
						}
					}
					else {
						if (user.find(var) != temp.end() && user[var] == 0){
							blocks[b].code[l].op = "DUMMY";
							c = 1;
						}
						else {
							user.clear();
							for (int i = l-1; i >= 0 && blocks[b].code[i].op != "UNARY*"; i--){
								var = blocks[b].code[i].res;
								if (istemp(var)) temp[var] = 1;
							}
						}
					}
				}
				else{
					if (istemp(var)){
						// Check if var is dead
						if ((temp.find(var) == temp.end() || temp[var] == 0) && 
							((blocks[b].code[l].op != "x++" && blocks[b].code[l].op != "++x" && blocks[b].code[l].op != "x--" && blocks[b].code[l].op != "--x") || 
							(user.find(op) != user.end() && user[op] == 0))) {
							blocks[b].code[l].op = "DUMMY";
							c = 1;
						}
						else {
							temp[var] = 0;
							op = blocks[b].code[l].op1;
							if (istemp(op))	temp[op] = 1;
							else user[op] = 1;
							op = blocks[b].code[l].op2;
							if (op.first != ""){
								if (istemp(op))	temp[op] = 1;
								else user[op] = 1;
							}
						}
					}
					else {
						if ((user.find(var)!=user.end() && user[var] == 0) &&
							((blocks[b].code[l].op != "x++" && blocks[b].code[l].op != "++x" && blocks[b].code[l].op != "x--" && blocks[b].code[l].op != "--x") || 
							(user.find(op) != user.end() && user[op] == 0))) {
							blocks[b].code[l].op = "DUMMY";
							c = 1;
						}
						else {
							user[var] = 0;
							op = blocks[b].code[l].op1;
							if (istemp(op))	temp[op] = 1;
							else user[op] = 1;
							op = blocks[b].code[l].op2;
							if (op.first != ""){
								if (istemp(op))	temp[op] = 1;
								else user[op] = 1;
							}
						}
					}
				}
			}
			gtemp[b] = temp;
			blocks[b].code.erase(remove_if(blocks[b].code.begin(), blocks[b].code.end(), [](quad q){return q.op == "DUMMY";}), blocks[b].code.end());
			handle_dead_block(b);	
		}
	}
	return c;
}

int opt_gotos(){
	int c = 0, s;
	int n = blocks.size();
	qi cond, res;
	long val;
	string op, op1;
	for (int b = n-1; b >= 0; b--){
		if (blocks[b].alive && !blocks[b].isglobal){
			if (blocks[b].code.back().op == "GOTO"){
				if (blocks[b].succ == blocks[b].next){
					blocks[b].code.pop_back();
					handle_dead_block(b);
					c = 1;
				}
			}
			else if (blocks[b].code.back().op == "IF_TRUE_GOTO"){
				if (blocks[b].succ != blocks[b].next){
					printf("Succ for block %d not handled correctly\n.", b);
					exit(-1);
				}
				if (blocks[b].cond_succ == blocks[b].next){
					blocks[b].code.pop_back();
					blocks[b].cond_succ = -1;
					handle_dead_block(b);
					c = 1;
				}
				else {
					cond = blocks[b].code.back().op1;
					for (int l = blocks[b].code.size()-2; l >= 0; l--){
						op = blocks[b].code[l].op;
						res = blocks[b].code[l].res;
						op1 = blocks[b].code[l].op1.first;
						if (res == cond){
							if (op == "=" && op1[0] == '$'){
								op1.erase(op1.begin());
								val = stol(op1);
								if (val){
									blocks[b].code.back().op = "GOTO";
									blocks[b].code.back().op1 = {"", NULL};
									s = blocks[b].succ;
									blocks[b].succ = blocks[b].cond_succ;
									blocks[b].cond_succ = -1;
									blocks[s].pred.erase(remove(blocks[s].pred.begin(), blocks[s].pred.end(), b), blocks[s].pred.end());
								}
								else {
									s = blocks[b].cond_succ;
									blocks[s].pred.erase(remove(blocks[s].pred.begin(), blocks[s].pred.end(), b), blocks[s].pred.end());
									blocks[b].cond_succ = -1;
									blocks[b].code.pop_back();
									handle_dead_block(b);
								}
								c = 1;
							}
							break;
						}
					}
				}
			}
		}
	}
	return c;
}



void optimize(){
	if (blocks.size() == 0) return;
	//opt_ret_dead();
	int limit = 10;
	int c = 1, l = limit;
	while (l-- && c){
		c = 0;
		opt_ret_dead();
		c = c | opt_cse();
		c = c | opt_copy();
		c = c | opt_dead_expr();
		c = c | opt_gotos();
	}
	//cout<<"Opt time: "<< limit-1-l << endl;
}
