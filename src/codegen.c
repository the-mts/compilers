#include "codegen.h"

vector<string> code;
// unordered_map<int, string> gotoLabels;
// unordered_map<string, > //
map<pair<int, int>, string> intregs;
map<int, string> retregs;
map<pair<int, int>, string> genregs;
vector<qi> params_list;
extern int file_ptrs;

string set_offset(qi quad){
	if(quad.second->is_global == 0){
		string s = "(%rbp)";
		s = to_string(-quad.second->offset)+s;
		return s;
	}
	else{
		string s = quad.first + "(%rip)";
		return s;
	}
}

void dump_const_labels(){
	for (auto i : constLabels){
		cout << i.first << ":" << endl;
		cout << i.second.second << endl;
	}
}

void setregmap(){
	intregs[{1,1}] = "%dil";
	intregs[{2,1}] = "%sil";
	intregs[{3,1}] = "%dl";
	intregs[{4,1}] = "%cl";
	intregs[{5,1}] = "%r8b";
	intregs[{6,1}] = "%r9b";

	intregs[{1,2}] = "%di";
	intregs[{2,2}] = "%si";
	intregs[{3,2}] = "%dx";
	intregs[{4,2}] = "%cx";
	intregs[{5,2}] = "%r8w";
	intregs[{6,2}] = "%r9w";

	intregs[{1,4}] = "%edi";
	intregs[{2,4}] = "%esi";
	intregs[{3,4}] = "%edx";
	intregs[{4,4}] = "%ecx";
	intregs[{5,4}] = "%r8d";
	intregs[{6,4}] = "%r9d";

	intregs[{1,8}] = "%rdi";
	intregs[{2,8}] = "%rsi";
	intregs[{3,8}] = "%rdx";
	intregs[{4,8}] = "%rcx";
	intregs[{5,8}] = "%r8";
	intregs[{6,8}] = "%r9";


	retregs[1] = "%al";
	retregs[2] = "%ax";
	retregs[4] = "%eax";
	retregs[8] = "%rax";

	// reguster_no, size
	genregs[{0, 1}] = "%al";
	genregs[{0, 2}] = "%ax";
	genregs[{0, 4}] = "%eax";
	genregs[{0, 8}] = "%rax";

	genregs[{1, 1}] = "%bl";
	genregs[{1, 2}] = "%bx";
	genregs[{1, 4}] = "%ebx";
	genregs[{1, 8}] = "%rbx";

	genregs[{2, 1}] = "%cl";
	genregs[{2, 2}] = "%cx";
	genregs[{2, 4}] = "%ecx";
	genregs[{2, 8}] = "%rcx";

	genregs[{3, 1}] = "%dl";
	genregs[{3, 2}] = "%dx";
	genregs[{3, 4}] = "%edx";
	genregs[{3, 8}] = "%rdx";

	genregs[{4, 1}] = "%sil";
	genregs[{4, 2}] = "%si";
	genregs[{4, 4}] = "%esi";
	genregs[{4, 8}] = "%rsi";

	genregs[{5, 1}] = "%dil";
	genregs[{5, 2}] = "%di";
	genregs[{5, 4}] = "%edi";
	genregs[{5, 8}] = "%rdi";

	for (int i = 8; i<16; i++){
		genregs[{i-2, 8}] = "%r" + to_string(i);
		genregs[{i-2, 1}] = genregs[{i-2, 8}] + "b";
		genregs[{i-2, 2}] = genregs[{i-2, 8}] + "w";
		genregs[{i-2, 4}] = genregs[{i-2, 8}] + "d";
	}
};

char sizechar(int size){
	switch (size)
	{
		case 1: return 'b';
		case 2: return 'w';
		case 4: return 'l';
		case 8: return 'q';
	}

	return 0;
}

void moveintarg(int arg, int offset, int size){
	switch(size){
		case 1:
			cout << "movb " << intregs[{arg, size}] << ", " << offset << "(%rbp)" << endl;
			break;
		case 2:
			cout << "movw " << intregs[{arg, size}] << ", " << offset << "(%rbp)" << endl;
			break;
		case 4:
			cout << "movl " << intregs[{arg, size}] << ", " << offset << "(%rbp)" << endl;
			break;
		case 8:
			cout << "movq " << intregs[{arg, size}] << ", " << offset << "(%rbp)" << endl;
			break;
	}
}

void codegen(){
	setregmap();
	for(int bno=0; bno!=-1; bno = blocks[bno].next){
		if(blocks[bno].isglobal){
			//Process global variables
			for(int i=0; i<blocks[bno].code.size(); i++){
				quad instr = blocks[bno].code[i];
				if(instr.op == "=global"){
					cout<<".globl"<<" "<<instr.res.first<<endl;
					cout<<".data"<<endl;
					cout<<".align"<<" "<<get_size(instr.res.second->type)<<endl;
					cout << ".type " << instr.res.first << ", @object" << endl;
					cout << ".size " << instr.res.first << ", "<< get_size(instr.res.second->type) << endl;
					cout<<instr.res.first<<":"<<endl;
					cout<<instr.op1.first<<endl;
				}
				else if(instr.op == "UNINIT_GLOBAL"){
					string type = instr.res.second->type;
					int sz1 = get_size(type, instr.res.second->ttentry);
					int align = 0;
					if(type.back()==']' || is_struct_or_union(type)){
						align = 32;
					}
					else{
						align = sz1;
					}
					cout<<".comm "<<instr.res.first<<","<<sz1<<","<<align<<endl;
				}
			}
			continue;
		}

		//cout << "bno: " << bno << ", pred's goto_addr:, " << (blocks[blocks[bno].pred[0]].code.back().goto_addr == bno) << endl;
		if(blocks[bno].pred.size()!=0 && (blocks[bno].pred.size()!=1 || blocks[blocks[bno].pred[0]].code.back().goto_addr == bno)){
			cout << ".L" << bno << ":" << endl;
		}

		for(int i=0; i<blocks[bno].code.size(); i++){
			quad instr = blocks[bno].code[i];
			if (instr.op == "FUNC_START"){
				string fname = instr.op1.first;
				cout << ".text" << endl;
				cout << ".globl " << fname << endl;
				cout << ".type " << fname << ", @function" << endl;
				cout << fname << ":" << endl;
				cout << "endbr64" << endl;
				cout << "pushq %rbp" << endl;
				cout << "movq %rsp, %rbp" << endl;
				long long sz = instr.op1.second->size;
				sz += (16-sz%16)%16;
				sz += 16;
				cout << "subq $"<<sz<<", %rsp" << endl;


				int int_char = 0, double_float = 0;
				auto args = *(instr.op1.second->arg_list);
				for(auto p: args){
					int flag = 0;
					if(!is_struct_or_union(p.first.first) && (p.first.first.find("int") != string::npos || p.first.first.find("char") != string::npos || p.first.first.back() == ']' || p.first.first.back() == '*' || (file_ptrs && p.first.first == "FILEP"))){
						flag = 1;
					}
					else if(!is_struct_or_union(p.first.first) && (p.first.first.find("double") != string::npos || p.first.first.find("float") != string::npos)){
						flag = 2;
					}
					else{
						flag = 3;
					}
					if(flag == 1 && int_char<6){
						int size = get_size(p.first.first);
						moveintarg(int_char+1, - (*(instr.op1.second->sym_table))[p.first.second]->offset, size);
						int_char++;
					}
					else if(flag == 2 && double_float<8){
						//// HANDLE FLOAT/DOUBLE ARGS COPY
						if(p.first.first == "float"){
							cout << "movss " << "%xmm" << double_float << ", " << - (*(instr.op1.second->sym_table))[p.first.second]->offset << "(%rbp)" << endl;
						}	
						else{
							cout << "movsd " << "%xmm" << double_float << ", " << - (*(instr.op1.second->sym_table))[p.first.second]->offset << "(%rbp)" << endl;
						}				

						double_float++;

					}
				}
			}

			else if(instr.op == "RETURN_VOID" || instr.op == "FUNC_END"){
				cout << "leave" << endl;
				cout << "ret" << endl;
			}
			
			else if(instr.op == "RETURN"){
				qi t1 = instr.op1;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
				}
				else if(type1.back()=='*' || (file_ptrs && type1 == "FILEP")){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
				}
				else if(is_struct_or_union(type1)){
					type1.pop_back();
					type1.pop_back();
					int size = get_size(type1, t1.second->ttentry);
					int ptr = instr.res.second->top_of_stack;
					cout << "movq " << -ptr << "(%rbp)" << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					vector<int> tmp;
					tmp.push_back(1);
					tmp.push_back(2);
					tmp.push_back(4);
					tmp.push_back(8);
					while(!tmp.empty()){
						int x = tmp.back();
						tmp.pop_back();
						while(size>=x){
							cout << "mov" << sizechar(x) << " " << "(%rax), " << genregs[{1,x}] << endl;
							cout << "mov" << sizechar(x) << " " << genregs[{1,x}] << ", " << "(%rcx)" << endl;
							cout<< "addq " << "$" << x << ", " << "%rcx" << endl;
							cout<< "addq " << "$" << x << ", " << "%rax" << endl;
							size-=x;
						}
					}
				}
				cout << "leave" << endl;
				cout << "ret" << endl;
			}

			if (instr.op == "GOTO"){
				cout << "jmp .L" << instr.goto_addr << endl;
			}

			else if(instr.op == "IF_TRUE_GOTO"){
				string p = instr.op1.second->type;
				if(p.find("int") != string::npos || p.find("char") != string::npos || p.back() == ']' || p.back() == '*' || (file_ptrs && p == "FILEP")){
					int size = get_size(p);
					if(p.back() == ']') size = 8;
					cout << "cmp" << sizechar(size) << " $0, " << set_offset(instr.op1) << endl;
					cout << "jne .L" << instr.goto_addr << endl;
				}
				else if(p == "float"){
					cout<< "pxor " << "%xmm0" << ", " << "%xmm0" << endl;
					cout<< "ucomiss " << set_offset(instr.op1) << ", " << "%xmm0" << endl;
					cout << "jp .L" << instr.goto_addr << endl;
					cout<< "pxor " << "%xmm0" << ", " << "%xmm0" << endl;
					cout<< "ucomiss " << set_offset(instr.op1) << ", " << "%xmm0" << endl;
					cout << "jne .L" << instr.goto_addr << endl;
				}
				else if(p == "double"){
					cout<< "pxor " << "%xmm0" << ", " << "%xmm0" << endl;
					cout<< "ucomisd " << set_offset(instr.op1) << ", " << "%xmm0" << endl;
					cout << "jp .L" << instr.goto_addr << endl;
					cout<< "pxor " << "%xmm0" << ", " << "%xmm0" << endl;
					cout<< "ucomisd " << set_offset(instr.op1) << ", " << "%xmm0" << endl;
					cout << "jne .L" << instr.goto_addr << endl;
				}
			}

			else if(instr.op == "PARAM"){
				params_list.push_back(instr.op1);
			}

			else if(instr.op == "CALL"){
				int int_char = 0, double_float = 0;
				int param_stk_size = 0;
				vector<qi> stk_params;
				// cerr << params_list.size() << endl;
				for(auto x: params_list){
					if(x.first[0]=='.' && constLabels.find(x.first)!=constLabels.end()){
						auto tmp = constLabels[x.first];
						if (tmp.first == ".string"){
							if(int_char<6){
								cout << "leaq " << x.first << "(%rip)" << ", " << intregs[{int_char+1, 8}] << endl;
								int_char++;
								continue;
							}
							else{
								// stk_params.push_back(x); // this???
							}
						}
					}
					string p = x.second->type;
					int flag = 0;
					if(!is_struct_or_union(p) && (p.find("int") != string::npos || p.find("char") != string::npos || p.back() == ']' || p.back() == '*' || (file_ptrs && p == "FILEP"))){
						flag = 1;
					}
					else if(!is_struct_or_union(p) && (p.find("double") != string::npos || p.find("float") != string::npos)){
						flag = 2;
					}
					else{
						flag = 3;
					}
					// cout << flag << " " << p << endl;

					if(flag == 1 && int_char<6){
						int size = get_size(p);
						if(p.back() == ']') size = 8;
						// cout << intregs[{int_char+1, size}] << endl;
						cout << "mov" << sizechar(size) << " " << set_offset(x) << ", " << intregs[{int_char+1, size}] << endl;
						int_char++;
					}
					else if(flag == 2 && double_float<8){
						if(p.find("float") != string::npos){
							if(instr.op1.second->type_name == REQUIRES_TYPECHECK){
								cout << "cvtss2sd " << set_offset(x) << ", " << "%xmm" << double_float << endl;
							}
							else{
								cout << "movss " << set_offset(x) << ", " << "%xmm" << double_float << endl;
							}
						}
						else if(p.find("double") != string::npos && p.find("long") == string::npos){
							cout << "movsd " << set_offset(x) << ", " << "%xmm" << double_float << endl;
						}
						double_float++;
					}
					else{
						stk_params.push_back(x);
					}
				}
				for(int x = stk_params.size()-1; x>=0; x--){
					string p = stk_params[x].second->type;
					if (!is_struct_or_union(p)){
						param_stk_size += 8;
					}
					else{
						string actual_type = p;
						actual_type.pop_back(); actual_type.pop_back();
						int size = get_size(actual_type, stk_params[x].second->ttentry);
						size += (8-size%8)%8;
						param_stk_size += size;
					}
				}
				if(param_stk_size%16){
					cout<<"pushq $0"<<endl;
					param_stk_size = 8;
				}
				else{
					param_stk_size = 0;
				}
				if(is_struct_or_union(instr.op1.second->type)){
					param_stk_size+=16;
					cout << "pushq " << set_offset(instr.res) << endl;
					cout << "pushq " << set_offset(instr.res) << endl;
				}
				for(int x = stk_params.size()-1; x>=0; x--){
					string p = stk_params[x].second->type;
					int size = get_size(p, stk_params[x].second->ttentry);
					if(p.back() == ']') size = 8;
					if (!is_struct_or_union(p)){
						cout << "pushq " << set_offset(stk_params[x]) << endl;
						param_stk_size += 8;
					}
					else{
						// HANDLE struct type
						string actual_type = p;
						actual_type.pop_back(); actual_type.pop_back();
						size = get_size(actual_type, stk_params[x].second->ttentry);
						size += (8-size%8)%8;
						param_stk_size += size;
						cout<<"movq "<< set_offset(stk_params[x]) << ", "<<"%rax"<< endl;
						cout<<"addq "<< "$"<< size-8<<", "<<"%rax"<<endl;
						cout << "pushq " << "0(%rax)" << endl;
						size -=8;
						while(size>0){
							cout<<"subq "<<"$8"<<", "<<"%rax"<<endl;
							cout << "pushq " << "0(%rax)" << endl;
							size -= 8;
						}

					}
				}

				cout << "movl " << "$" << double_float << ", " << "%eax" << endl;
				cout << "call " << instr.op1.first << endl;
				if(param_stk_size) cout << "addq $" << param_stk_size << ", %rsp" << endl;

				qi t1 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout << "movl " << "%eax, " << set_offset(t1) << endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout << "movw " << "%ax, " << set_offset(t1) << endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout << "movq " << "%rax, " << set_offset(t1) << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << "%al, " << set_offset(t1) << endl;
				}
				else if(type1.back()=='*' || (file_ptrs && type1 == "FILEP")){
					cout << "movq " << "%rax, " << set_offset(t1) << endl;
				}

				else if(type1=="float"){
					cout << "movss " << "%xmm0, " << set_offset(t1) << endl;
				}
				else if(type1=="double"){
					cout << "movsd " << "%xmm0, " << set_offset(t1) << endl;
				}

				stk_params.clear();
				params_list.clear();
			}
			
			else if(instr.op == "TAIL"){
				qi f = instr.op1;
				vector<pair<pair<string, string>, tt_entry*>> args = *(f.second->arg_list);
				int freei = 0, freef = 0, flag;
				string var, type;
				map <string, string> param2reg;
				int size;
				{
					/*if (args.size() != params_list.size()) {
						cout << "Args: " << args.size() << ", Params: " << params_list.size() << endl;
						cout << "Dead!" << endl;
						return;
					}*/
					/*for (auto param: params_list){
						var = param.first;
						type = param.second->type;
						if (param2reg.find(var) == param2reg.end()){
							for (auto arg: args){
								if (var == arg.first.second){
									if (type == "float" || type == "double"){
										param2reg[var] = "%xmm" + to_string(freef);
										if (type == "float"){
											cout << "movss ";
										} else {
											cout << "movsd ";
										}
										cout << -param.second->offset << "(%rbp), " << "%xmm" << freef << endl; 
										freef++;
									}
									else {
										size = get_size(type);
										param2reg[var] = genregs[{freei, size}];
										cout << "mov" << sizechar(size) << " " << -param.second->offset << "(%rbp), " << param2reg[var] << endl;
										freei++;
									}
									break;
								}
							}
						}
					}*/
				}
				//return;
				qi param;
				string arg;
				for (int i = 0; i < args.size(); i++){
					param = params_list[i];
					var = param.first;
					type = param.second->type;
					if (args[i].first.second != var && param2reg.find(var) == param2reg.end()){
						if (type == "float" || type == "double"){
							param2reg[var] = "%xmm" + to_string(freef);
							if (type == "float"){
								cout << "movss ";
							} else {
								cout << "movsd ";
							}
							cout << set_offset(param) << ", " << "%xmm" << freef << endl;
							freef++;
						}
						else {
							size = get_size(type);
							param2reg[var] = genregs[{freei, size}];
							cout << "mov" << sizechar(size) << " " << set_offset(param) << ", " << param2reg[var] << endl;
							freei++;
						}
					}
				}
				//return;
				for (int i = 0; i < args.size(); i++){
					//- (*(instr.op1.second->sym_table))[p.first.second]->offset,
					param = params_list[i];
					arg = args[i].first.second;
					//return;
					var = param.first;
					type = param.second->type;
					size = get_size(type);
					//return;
					if (param2reg.find(var) != param2reg.end()){
						//return;
						if (type == "double"){
							cout << "movsd " << param2reg[var] << ", " << -(*(f.second->sym_table))[arg]->offset << "(%rbp)" << endl;
						}
						else if (type == "float"){
							cout << "movss " << param2reg[var] << ", " << -(*(f.second->sym_table))[arg]->offset << "(%rbp)" << endl;
						}
						/*else if (size == 8){
							cout << "movq " << param2reg[var] << ", " << -(*(f.second->sym_table))[arg]->offset << "(%rbp)" << endl;
						}*/
						else {
							cout << "mov" << sizechar(size) << " " << param2reg[var] << ", " << -(*(f.second->sym_table))[arg]->offset << "(%rbp)" << endl;
						}
					}
					//return;
				}
				cout << "jmp .L" << instr.goto_addr << endl;
				//return;
				params_list.clear();
			}

			else if(instr.op == "+struct"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type;
				cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
				cout << "addq " << instr.op2.first << ", " << "%rax" << endl;
				cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
			}

			else if(instr.op == "+int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type;
				string type2 = instr.op2.second->type;
				if((type1 == "int" && type2 == "int")){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "addl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "long int")){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "short int" && type2 == "short int")){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "addw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "char" && type2 == "char")){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "addb " << set_offset(t2) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "short int" && type2 == "char")){
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "addw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "short int" && type1 == "char")){
					swap(t1,t2);
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "addw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "int" && type2 == "char")){
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "addl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "int" && type1 == "char")){
					swap(t1,t2);
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "addl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "int" && type2 == "short int")){
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "addl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "int" && type1 == "short int")){
					swap(t1,t2);
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "addl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "char")){
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "char")){
					swap(t1,t2);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "short int")){
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "short int")){
					swap(t1,t2);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}

				else if((type1 == "long int" && type2 == "int")){
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "int")){
					swap(t1,t2);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == '*' && (type2 == "char")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2.back() == '*' && (type1 == "char")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == '*' && (type2 == "short int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2.back() == '*' && (type1 == "short int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}

				else if(type1.back() == '*' && (type2 == "int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2.back() == '*' && (type1 == "int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}

				else if(type1.back() == '*' && (type2 == "long int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2.back() == '*' && (type1 == "long int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}

				else if(type1.back() == ']' && (type2 == "char")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2.back() == ']' && (type1 == "char")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == ']' && (type2 == "short int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2.back() == ']' && (type1 == "short int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}

				else if(type1.back() == ']' && (type2 == "int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2.back() == ']' && (type1 == "int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}

				else if(type1.back() == ']' && (type2 == "long int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2.back() == ']' && (type1 == "long int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq $" << size << ", " << "%rax" << endl;
					cout << "addq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "-int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if((type1 == "int" && type2 == "int")){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "subl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "long int")){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "short int" && type2 == "short int")){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "subw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "char" && type2 == "char")){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "subb " << set_offset(t2) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "short int" && type2 == "char")){
					cout << "movsbw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "subw " << "%cx, " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "short int" && type1 == "char")){
					cout << "movsbw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "subw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "int" && type2 == "char")){
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "subl " << "%ecx, " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "int" && type1 == "char")){
					cout << "movsbl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "subl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "int" && type2 == "short int")){
					cout << "movswl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "subl " << "%ecx, " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "int" && type1 == "short int")){
					cout << "movswl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "subl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "char")){
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "char")){
					cout << "movsbq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "short int")){
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "short int")){
					cout << "movswq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}

				else if((type1 == "long int" && type2 == "int")){
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "int")){
					cout << "movslq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == '*' && (type2 == "char")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "imulq $" << size << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == '*' && (type2 == "short int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "imulq $" << size << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == '*' && (type2 == "int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "imulq $" << size << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == '*' && (type2 == "long int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "imulq $" << size << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == '*' && type2.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "movq $" << size << ", %rcx" << endl;
					cout << "idivq " << "%rcx" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}

				else if(type1.back() == ']' && (type2 == "char")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "imulq $" << size << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == ']' && (type2 == "short int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "imulq $" << size << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == ']' && (type2 == "int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "imulq $" << size << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1.back() == ']' && (type2 == "long int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "imulq $" << size << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "subq " << "%rcx, " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "*int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if((type1 == "int" && type2 == "int")){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "imull " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "long int")){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "imulq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "short int" && type2 == "short int")){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "imulw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "char" && type2 == "char")){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "imulb " << set_offset(t2) << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "short int" && type2 == "char")){
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "imulw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "short int" && type1 == "char")){
					swap(t1,t2);
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "imulw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}

				else if((type1 == "int" && type2 == "char")){
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "imull " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "int" && type1 == "char")){
					swap(t1,t2);
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "imull " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "int" && type2 == "short int")){
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "imull " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "int" && type1 == "short int")){
					swap(t1,t2);
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "imull " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "char")){
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "char")){
					swap(t1,t2);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "short int")){
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "short int")){
					swap(t1,t2);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type1 == "long int" && type2 == "int")){
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if((type2 == "long int" && type1 == "int")){
					swap(t1,t2);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "imulq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "/int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << set_offset(t2) << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << set_offset(t2) << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "cwtd" << endl;
					cout << "idivw " << set_offset(t2) << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout << "movsbl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << "%ecx" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movsbw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "cwtd" << endl;
					cout << "idivw " << "%cx" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					cout << "movsbw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "cwtd" << endl;
					cout << "idivw " << set_offset(t2) << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << "%ecx" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "char"){
					cout << "movsbl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << set_offset(t2) << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movswl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << "%ecx" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					cout << "movswl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << set_offset(t2) << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << "%rcx" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					cout << "movsbq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << set_offset(t2) << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << "%rcx" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					cout << "movswq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << set_offset(t2) << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << "%rcx" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					cout << "movslq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << set_offset(t2) << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				// else if(type1 == "unsigned long int"){
				// 	if(type2 == "unsigned long int" || type2 == "long int"){
				// 		cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
				// 		cout << "movl $0, %edx" << endl;
				// 		cout << "divq " << set_offset(t2) << endl;
				// 		cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				// 	}
				// 	else if(type2 == "unsigned int" || type2 == "int"){
				// 		cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
				// 		cout << "movl " << set_offset(t2) << ", " << "%ecx" << endl;
				// 		cout << "movl $0, %edx" << endl;
				// 		cout << "divq " << "%rcx" << endl;
				// 		cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				// 	}
				// }
				// else if(type2 == "unsigned long int"){
				// 	if(type1 == "unsigned long int" || type1 == "long int"){
				// 		cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
				// 		cout << "movl $0, %edx" << endl;
				// 		cout << "divq " << set_offset(t2) << endl;
				// 		cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				// 	}
				// 	else if(type1 == "unsigned int" || type1 == "int"){
				// 		cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
				// 		cout << "movl $0, %edx" << endl;
				// 		cout << "divq " << set_offset(t2) << endl;
				// 		cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				// 	}
				// }
				/* CHECK THE NEXT TWO RULES */

				// else if(type1 == "long int" && type2 == "unsigned int"){
				// 	cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
				// 	cout << "movl " << set_offset(t2) << ", " << "%ecx" << endl;
				// 	cout << "movl $0, %edx" << endl;
				// 	cout << "divq " << "%rcx" << endl;
				// 	cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				// }
				// else if(type1 == "unsigned int" && type2 == "long int"){
				// 	cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
				// 	cout << "movl $0, %edx" << endl;
				// 	cout << "divq " << set_offset(t2) << endl;
				// 	cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				// }

				// else if(type1 == "unsigned int"){
				// 	if(type2 == "unsigned int"){
				// 		cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
				// 		cout << "mov $0, %edx" << endl;
				// 		cout << "divl " << set_offset(t2) << endl;
				// 		cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				// 	}
				// 	else if(type2 == "int"){
				// 		cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
				// 		cout << "mov $0, %edx" << endl;
				// 		cout << "divl " << set_offset(t2) << endl;
				// 		cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				// 	}
				// }
				// else if(type2 == "unsigned int"){
				// 	if(type1 == "unsigned int"){
				// 		cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
				// 		cout << "mov $0, %edx" << endl;
				// 		cout << "divl " << set_offset(t2) << endl;
				// 		cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				// 	}
				// 	else if(type1 == "int"){
				// 		cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
				// 		cout << "mov $0, %edx" << endl;
				// 		cout << "divl " << set_offset(t2) << endl;
				// 		cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				// 	}
				// }
			}

			else if(instr.op == "+real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "addss " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "addsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "addsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "addsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "-real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "subss " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "subsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "subsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "subsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "*real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "mulss " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "mulsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "mulsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "mulsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "/real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "divss " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "divsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "divsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "divsd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "%"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << set_offset(t2) << endl;
					cout << "movl " << "%edx, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << set_offset(t2) << endl;
					cout << "movq " << "%rdx, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "cwtd" << endl;
					cout << "idivw " << set_offset(t2) << endl;
					cout << "movw " << "%dx, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout << "movsbl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << "%ecx" << endl;
					cout << "movb " << "%dl, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movsbw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "cwtd" << endl;
					cout << "idivw " << "%cx" << endl;
					cout << "movw " << "%dx, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					cout << "movsbw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "cwtd" << endl;
					cout << "idivw " << set_offset(t2) << endl;
					cout << "movw " << "%dx, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << "%ecx" << endl;
					cout << "movl " << "%edx, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "char"){
					cout << "movsbl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << set_offset(t2) << endl;
					cout << "movl " << "%edx, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movswl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << "%ecx" << endl;
					cout << "movl " << "%edx, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					cout << "movswl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "cltd" << endl;
					cout << "idivl " << set_offset(t2) << endl;
					cout << "movl " << "%edx, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << "%rcx" << endl;
					cout << "movq " << "%rdx, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					cout << "movsbq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << set_offset(t2) << endl;
					cout << "movq " << "%rdx, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << "%rcx" << endl;
					cout << "movq " << "%rdx, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					cout << "movswq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << set_offset(t2) << endl;
					cout << "movq " << "%rdx, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << "%rcx" << endl;
					cout << "movq " << "%rdx, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					cout << "movslq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "cqto" << endl;
					cout << "idivq " << set_offset(t2) << endl;
					cout << "movq " << "%rdx, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "&"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "andl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "andq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "andw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "andb " << set_offset(t2) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "andw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1, t2);
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "andw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "andl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "andl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "andl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "andl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "andq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "andq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "andq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "andq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "andq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "andq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "|"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "orl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "orq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "orw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "orb " << set_offset(t2) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "orw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1, t2);
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "orw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "orl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "orl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "orl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "orl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "orq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "orq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "orq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "orq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "orq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "orq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "^"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "xorl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "xorq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "xorw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "xorb " << set_offset(t2) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "xorw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1, t2);
					cout << "movsbw " << set_offset(t2) << ", " << "%ax" << endl;
					cout << "xorw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "xorl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "xorl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "xorl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswl " << set_offset(t2) << ", " << "%eax" << endl;
					cout << "xorl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "xorq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "xorq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "xorq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "xorq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "xorq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout << "movslq " << set_offset(t2) << ", " << "%rax" << endl;
					cout << "xorq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "++x"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout << "addl $1, " << set_offset(t1) << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int"){
					cout << "addq $1, " << set_offset(t1) << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int"){
					cout << "addw $1, " << set_offset(t1) << endl;
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char"){
					cout << "addb $1, " << set_offset(t1) << endl;
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "float"){
					cout<< "movl "<<"$1, "<<"%eax"<<endl;
					cout<<"cvtsi2ss "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movss "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"addss %xmm0, %xmm1"<<endl;
					cout<<"movss "<<"%xmm1, "<<set_offset(t1)<<endl;
					cout<<"movss "<<"%xmm1, "<<set_offset(instr.res)<<endl;
				}
				else if(type1 == "double"){
					cout<< "movl "<<"$1, "<<"%eax"<<endl;
					cout<<"cvtsi2sd "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movsd "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"addsd %xmm0, %xmm1"<<endl;
					cout<<"movsd "<<"%xmm1, "<<set_offset(t1)<<endl;
					cout<<"movsd "<<"%xmm1, "<<set_offset(instr.res)<<endl;
				}
				else if(type1.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "addq $" << size << ", " << set_offset(t1) << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "x++"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
					cout << "addl $1, " << set_offset(t1) << endl;
				}
				else if(type1 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
					cout << "addw $1, " << set_offset(t1) << endl;
				}
				else if(type1 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
					cout << "addq $1, " << set_offset(t1) << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
					cout << "addb $1, " << set_offset(t1) << endl;
				}
				else if(type1 == "float"){
					cout<< "movl "<<"$1, "<<"%eax"<<endl;
					cout<<"cvtsi2ss "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movss "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"movss "<<"%xmm1, "<<set_offset(instr.res)<<endl;
					cout<<"addss %xmm0, %xmm1"<<endl;
					cout<<"movss "<<"%xmm1, "<<set_offset(t1)<<endl;
				}
				else if(type1 == "double"){
					cout<< "movl "<<"$1, "<<"%eax"<<endl;
					cout<<"cvtsi2sd "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movsd "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"movsd "<<"%xmm1, "<<set_offset(instr.res)<<endl;
					cout<<"addsd %xmm0, %xmm1"<<endl;
					cout<<"movsd "<<"%xmm1, "<<set_offset(t1)<<endl;
				}
				else if(type1.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
					cout << "addq $" << size << ", " << set_offset(t1) << endl;
				}
			}

			else if(instr.op == "--x"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout << "subl $1, " << set_offset(t1) << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int"){
					cout << "subq $1, " << set_offset(t1) << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int"){
					cout << "subw $1, " << set_offset(t1) << endl;
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char"){
					cout << "subb $1, " << set_offset(t1) << endl;
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "float"){
					cout<< "movl "<<"$1, "<<"%eax"<<endl;
					cout<<"cvtsi2ss "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movss "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"subss %xmm0, %xmm1"<<endl;
					cout<<"movss "<<"%xmm1, "<<set_offset(t1)<<endl;
					cout<<"movss "<<"%xmm1, "<<set_offset(instr.res)<<endl;
				}
				else if(type1 == "double"){
					cout<< "movl "<<"$1, "<<"%eax"<<endl;
					cout<<"cvtsi2sd "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movsd "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"subsd %xmm0, %xmm1"<<endl;
					cout<<"movsd "<<"%xmm1, "<<set_offset(t1)<<endl;
					cout<<"movsd "<<"%xmm1, "<<set_offset(instr.res)<<endl;
				}
				else if(type1.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "subq $" << size << ", " << set_offset(t1) << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "x--"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
					cout << "subl $1, " << set_offset(t1) << endl;
				}
				else if(type1 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
					cout << "subq $1, " << set_offset(t1) << endl;
				}
				else if(type1 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
					cout << "subw $1, " << set_offset(t1) << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
					cout << "subb $1, " << set_offset(t1) << endl;
				}
				else if(type1 == "float"){
					cout<< "movl "<<"$1, "<<"%eax"<<endl;
					cout<<"cvtsi2ss "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movss "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"movss "<<"%xmm1, "<<set_offset(instr.res)<<endl;
					cout<<"subss %xmm0, %xmm1"<<endl;
					cout<<"movss "<<"%xmm1, "<<set_offset(t1)<<endl;
				}
				else if(type1 == "double"){
					cout<< "movl "<<"$1, "<<"%eax"<<endl;
					cout<<"cvtsi2sd "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movsd "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"movsd "<<"%xmm1, "<<set_offset(instr.res)<<endl;
					cout<<"subsd %xmm0, %xmm1"<<endl;
					cout<<"movsd "<<"%xmm1, "<<set_offset(t1)<<endl;
				}
				else if(type1.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1, t1.second->ttentry);
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
					cout << "subq $" << size << ", " << set_offset(t1) << endl;
				}
			}

			else if(instr.op == "<<"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "sall %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "salq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "salw %cl, %ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "movb " << set_offset(t2) << ", " << "%cl" << endl;
					cout << "salb %cl, %al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout << "movsbw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "salw %cl, %ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "salw %cl, %ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "sall %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "sall %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout << "movswl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "sall %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "sall %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "salq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "salq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "salq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "salq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "salq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "salq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == ">>"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "sarl %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "sarq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "sarw %cl, %ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "movb " << set_offset(t2) << ", " << "%cl" << endl;
					cout << "sarb %cl, %al" << endl;
					cout << "movb " << "%al, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout << "movsbw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "sarw %cl, %ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbw " << set_offset(t2) << ", " << "%cx" << endl;
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "sarw %cl, %ax" << endl;
					cout << "movw " << "%ax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "sarl %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "sarl %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout << "movswl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "sarl %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswl " << set_offset(t2) << ", " << "%ecx" << endl;
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "sarl %cl, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "sarq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout << "movsbq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "sarq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "sarq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout << "movswq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "sarq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "sarq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout << "movslq " << set_offset(t2) << ", " << "%rcx" << endl;
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "sarq %cl, %rax" << endl;
					cout << "movq " << "%rax, " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "<int" || instr.op == ">int" || instr.op == "<=int" || 
					instr.op == ">=int" || instr.op == "==int" || instr.op == "!=int"){
				string op = instr.op;
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				qi r  = instr.res;
				string type1 = t1.second->type;
				if (type1.back() == '*' || (file_ptrs && type1 == "FILEP")) type1 = "long int";
				string type2 = t2.second->type;
				if (type2.back() == '*' || (file_ptrs && type1 == "FILEP")) type2 = "long int";
				map<string, int> rank = {{"char", 1}, {"short int", 2}, {"int", 4}, {"long int", 8}};
				
				if (rank[type1] > rank[type2]){
					swap(t1, t2);
					swap(type1, type2);
					if (op == "<int") op = ">int";
					else if (op == ">int") op = "<int";
					else if (op == "<=int") op = ">=int";
					else if (op == ">=int") op = "<=int";
				}

				if (type1 == "char") 			cout << "movsbq " << set_offset(t1) << ", " << "%rax" << endl;
				else if (type1 == "short int")  cout << "movswq " << set_offset(t1) << ", " << "%rax" << endl;
				else if (type1 == "int") 		cout << "movslq " << set_offset(t1) << ", " << "%rax" << endl;
				else if (type1 == "long int") 	cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;

				if (type2 == "char") 			cout << "cmpb " << set_offset(t2) << ", %al" << endl;	
				else if (type2 == "short int")	cout << "cmpw " << set_offset(t2) << ", %ax" << endl;	
				else if (type2 == "int") 		cout << "cmpl " << set_offset(t2) << ", %eax" << endl;	
				else if (type2 == "long int") 	cout << "cmpq " << set_offset(t2) << ", %rax" << endl;

				cout << "set";
				if (op == "<int") 		cout << "l";
				else if (op == ">int") 	cout << "g";
				else if (op == "<=int")	cout << "le";
				else if (op == ">=int") cout << "ge";
				else if (op == "==int") cout << "e";
				else if (op == "!=int") cout << "ne";
				cout << " %al" << endl;

				cout << "movzbl %al, %eax" << endl;
				cout << "movl %eax, " << set_offset(r) << endl;

			}

			else if(instr.op == "<real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comiss " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "seta " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "seta " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "seta " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "seta " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == ">real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comiss " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setb " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setb " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setb " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setb " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "<=real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comiss " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setnb " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setnb " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setnb " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setnb " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == ">=real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comiss " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setna " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setna " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setna " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "comisd " << "%xmm0" << ", " << "%xmm1" << endl;
					cout << "setna " << "%al" << endl;
					cout << "movzbl " << "%al" << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "==real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomiss " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "setnp %al" << endl;
					cout << "movl $0, %edx" << endl;
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomiss " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "cmovne %edx, %eax" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "setnp %al" << endl;
					cout << "movl $0, %edx" << endl;
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "cmovne %edx, %eax" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "setnp %al" << endl;
					cout << "movl $0, %edx" << endl;
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "cmovne %edx, %eax" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "setnp %al" << endl;
					cout << "movl $0, %edx" << endl;
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "cmovne %edx, %eax" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "!=real"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1=="float" && type2=="float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomiss " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "setp %al" << endl;
					cout << "movl $1, %edx" << endl;
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomiss " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "cmovne %edx, %eax" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "setp %al" << endl;
					cout << "movl $1, %edx" << endl;
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "cmovne %edx, %eax" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "setp %al" << endl;
					cout << "movl $1, %edx" << endl;
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "cmovne %edx, %eax" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "setp %al" << endl;
					cout << "movl $1, %edx" << endl;
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "cvtss2sd " << set_offset(t2) << ", " << "%xmm1" << endl;
					cout << "ucomisd " << "%xmm1" << ", " << "%xmm0" << endl;
					cout << "cmovne %edx, %eax" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(instr.res) << endl;
				}
			}

			else if(instr.op == "UNARY+"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "movb " << "%al" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0" << ", " << set_offset(t2) << endl;
				}
				else if(type1.back()=='*'){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax" << ", " << set_offset(t2) << endl;
				}
			}

			else if(instr.op == "UNARY-"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "negl " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "negw " << "%ax" << endl;
					cout << "movw " << "%ax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "negq " << "%rax" << endl;
					cout << "movq " << "%rax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "negb " << "%al" << endl;
					cout << "movb " << "%al" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "float"){
					cout<< "movl "<<"$0, "<<"%eax"<<endl;
					cout<<"cvtsi2ss "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movss "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"subss %xmm1, %xmm0"<<endl;
					cout<<"movss "<<"%xmm0, "<<set_offset(instr.res)<<endl;
				}
				else if(type1 == "double"){
					cout<< "movl "<<"$0, "<<"%eax"<<endl;
					cout<<"cvtsi2sd "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movsd "<<set_offset(t1)<<", "<<"%xmm1"<<endl;
					cout<<"subsd %xmm1, %xmm0"<<endl;
					cout<<"movsd "<<"%xmm0, "<<set_offset(instr.res)<<endl;
				}
			}

			else if(instr.op == "UNARY!"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout << "cmpl " << "$0, " << set_offset(t1) << endl;
					cout << "sete %al" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(t2) << endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout << "cmpw " << "$0, " << set_offset(t1) << endl;
					cout << "sete %al" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(t2) << endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout << "cmpq " << "$0, " << set_offset(t1) << endl;
					cout << "sete %al" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(t2) << endl;
				}
				else if(type1 == "char"){
					cout << "cmpb " << "$0, " << set_offset(t1) << endl;
					cout << "sete %al" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(t2) << endl;
				}
				else if(type1.back()=='*' || type1.back() == ']' || (file_ptrs && type1 == "FILEP")){
					cout << "cmpq " << "$0, " << set_offset(t1) << endl;
					cout << "sete %al" << endl;
					cout << "movzbl %al, %eax" << endl;
					cout << "movl " << "%eax, " << set_offset(t2) << endl;
				}
				else if(type1 == "float"){
					cout<< "pxor " << "%xmm0" << ", " << "%xmm0" << endl;
					cout<< "ucomiss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout<< "setnp " << "%al" << endl;
					cout<< "movl " << "$0" << ", " << "%edx" << endl;
					cout<< "pxor " << "%xmm0" << ", " << "%xmm0" << endl;
					cout<< "ucomiss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout<< "cmovne " << "%edx" << ", " << "%eax" << endl;
					cout<< "movzbl " << "%al" << ", " << "%eax" << endl;
					cout<< "movl " << "%eax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "double"){
					cout<< "pxor " << "%xmm0" << ", " << "%xmm0" << endl;
					cout<< "ucomisd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout<< "setnp " << "%al" << endl;
					cout<< "movl " << "$0" << ", " << "%edx" << endl;
					cout<< "pxor " << "%xmm0" << ", " << "%xmm0" << endl;
					cout<< "ucomisd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout<< "cmovne " << "%edx" << ", " << "%eax" << endl;
					cout<< "movzbl " << "%al" << ", " << "%eax" << endl;
					cout<< "movl " << "%eax" << ", " << set_offset(t2) << endl;
				}
			}

			else if(instr.op == "UNARY~"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "notl " << "%eax" << endl;
					cout << "movl " << "%eax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "notw " << "%ax" << endl;
					cout << "movw " << "%ax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "notq " << "%rax" << endl;
					cout << "movq " << "%rax" << ", " << set_offset(t2) << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "notb " << "%al" << endl;
					cout << "movb " << "%al" << ", " << set_offset(t2) << endl;
				}
			}

			else if(instr.op == "UNARY&"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				cout << "leaq " << set_offset(t1) << ", " << "%rax" << endl;
				cout << "movq " << "%rax, " << set_offset(t2) << endl;
			}

			else if(instr.op == "UNARY*"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t2.second->type;
				cout << "movq " << set_offset(t1) << ", " << "%rcx" << endl;
				if(type1 == "int" || type1 == "unsigned int"){
					cout << "movl " << "(%rcx)" << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(t2) << endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout << "movw " << "(%rcx)" << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(t2) << endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int" || type1.back()=='*'){
					cout << "movq " << "(%rcx)" << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(t2) << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << "(%rcx)" << ", " << "%al" << endl;
					cout << "movb " << "%al, " << set_offset(t2) << endl;
				}
				else if(type1 == "float"){
					cout << "movss " << "(%rcx)" << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0, " << set_offset(t2) << endl;
				}
				else if(type1 == "double"){
					cout << "movsd " << "(%rcx)" << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0, " << set_offset(t2) << endl;
				}
			}

			else if(instr.op == "="){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t2.second->type;
				if(t1.first[0]=='$'){ //Immediate
					if(type1 == "int" || type1 == "unsigned int"){
						cout << "movl " << t1.first << ", " << set_offset(t2) << endl;
					}
					else if(type1 == "short int" || type1 == "unsigned short int"){
						cout << "movw " << t1.first << ", " << set_offset(t2) << endl;
					}
					else if(type1 == "long int" || type1 == "unsigned long int"){
						cout << "movabsq " << t1.first << ", " << "%rax" << endl;
						cout << "movq " << "%rax" << ", " << set_offset(t2) << endl;
					}
					else if(type1 == "char"){
						cout << "movb " << t1.first << ", " << set_offset(t2) << endl;
					}
					else if(type1.back()=='*' || (file_ptrs && type1 == "FILEP")){
						cout << "movq " << t1.first << ", " << set_offset(t2) << endl;
					}
				}
				else if(t1.first[0]=='.'){ //RIP address for floats
					if(constLabels.find(t1.first)!=constLabels.end() && constLabels[t1.first].first == ".string"){
						cout<<"leaq "<<t1.first<<"(%rip)"<<", "<<"%rax"<<endl;
						cout<< "movq "<<"%rax"<<", "<<set_offset(t2)<<endl;
					}
					if(type1 == "float"){
						cout << "movss " << t1.first << "(%rip)" << ", " << "%xmm0" << endl;
						cout << "movss " << "%xmm0, " << set_offset(t2) << endl;
					}
					else if(type1 == "double"){
						cout << "movsd " << t1.first << "(%rip)" << ", " << "%xmm0" << endl;
						cout << "movsd " << "%xmm0, " << set_offset(t2) << endl;
					}
					else if(type1 == "long double"){
						cout << "fldt " << t1.first << "(%rip)" << endl;
						cout << "fstpt " << set_offset(t2) << endl;
					}
				}
				else{
					if(type1 == "int"){
						cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
						cout << "movl " << "%eax, " << set_offset(t2) << endl;
					}
					else if(type1 == "short int"){
						cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
						cout << "movw " << "%ax, " << set_offset(t2) << endl;
					}
					else if(type1 == "long int"){
						cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
						cout << "movq " << "%rax, " << set_offset(t2) << endl;
					}
					else if(type1 == "char"){
						cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
						cout << "movb " << "%al, " << set_offset(t2) << endl;
					}
					else if(type1.back()=='*' || type1.back() == ']' || (file_ptrs && type1 == "FILEP")){
						cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
						cout << "movq " << "%rax, " << set_offset(t2) << endl;
					}

					else if(type1 == "float"){
						cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
						cout << "movss " << "%xmm0, " << set_offset(t2) << endl;
					}
					else if(type1 == "double"){
						cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
						cout << "movsd " << "%xmm0, " << set_offset(t2) << endl;
					}

					else{ //// ASSUMING SIZE 8 FOR ANY OTHER CALLS (SAY STRUCT #)
						cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
						cout << "movq " << "%rax, " << set_offset(t2) << endl;
					}
				}
			}
			else if(instr.op == "=struct"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				type1.pop_back();
				type1.pop_back();
				int size = get_size(type1, t1.second->ttentry);
				cout << "movq " << set_offset(t2) << ", " << "%rcx" << endl;
				cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
				vector<int> tmp;
				tmp.push_back(1);
				tmp.push_back(2);
				tmp.push_back(4);
				tmp.push_back(8);
				while(!tmp.empty()){
					int x = tmp.back();
					tmp.pop_back();
					while(size>=x){
						cout << "mov" << sizechar(x) << " " << "(%rax), " << genregs[{1,x}] << endl;
						cout << "mov" << sizechar(x) << " " << genregs[{1,x}] << ", " << "(%rcx)" << endl;
						cout<< "addq " << "$" << x << ", " << "%rcx" << endl;
						cout<< "addq " << "$" << x << ", " << "%rax" << endl;
						size-=x;
					}
				}
				// while(sz<size){
				// 	cout << "movq " << "(%rax), " << "%rbx" << endl;
				// 	cout << "movq " << "%rbx, " << "(%rcx)" << endl;
				// 	sz+=8;
				// }
			}
			else if(instr.op == "ADDR="){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				cout << "movq " << set_offset(t2) << ", " << "%rcx" << endl;
				if(type1 == "int"){
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << "(%rcx)" << endl;
				}
				else if(type1 == "short int"){
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << "(%rcx)" << endl;
				}
				else if(type1 == "long int"){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << "(%rcx)" << endl;
				}
				else if(type1 == "char"){
					cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
					cout << "movb " << "%al, " << "(%rcx)" << endl;
				}
				else if(type1.back()=='*' || type1.back() == '#'){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << "(%rcx)" << endl;
				}
				else if(type1 == "float"){
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0, " << "(%rcx)" << endl;
				}
				else if(type1 == "double"){
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0, " << "(%rcx)" << endl;
				}
			}

			// (int-to-long int)
			else if(instr.op[0] == '(' || instr.op=="inttoreal"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1, type2;
				if(instr.op[0] == '('){
					int x = instr.op.find("-");
					type1 = instr.op.substr(1, x-1);
					type2 = instr.op.substr(x+4, instr.op.length()-x-5);
				}
				else{
					type1 = instr.op1.second->type;
					type2 = instr.res.second->type;
				}
				if(type1 == type2 && (type1.back()!='*' && type1 != "FILEP")){
					if(type1 == "char"){
						cout << "movb " << set_offset(t1) << ", " << "%al" << endl;
						cout << "movb " << "%al, " << set_offset(t2) << endl;
					}
					else if(type1 == "short int"){
						cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
						cout << "movw " << "%ax, " << set_offset(t2) << endl;
					}
					else if(type1 == "int"){
						cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
						cout << "movl " << "%eax, " << set_offset(t2) << endl;
					}
					else if(type1 == "long int"){
						cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
						cout << "movq " << "%rax, " << set_offset(t2) << endl;
					}
					else if(type1 == "float"){
						cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
						cout << "movss " << "%xmm0, " << set_offset(t2) << endl;
					}
					else if(type1 == "double"){
						cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
						cout << "movsd " << "%xmm0, " << set_offset(t2) << endl;
					}
				}
				// Between ints and pointers
				else if((type1.back()=='*' || (file_ptrs && type1 == "FILEP")) && (type2.back()=='*' || (file_ptrs && type2 == "FILEP"))){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(t2) << endl;
				}
				else if(type1.back()==']' && (type2.back()=='*' || (file_ptrs && type2 == "FILEP"))){
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(t2) << endl;
				}
				else if((type1=="char" || type1=="short int" || type1=="int") && (type2=="long int" || (type2.back()=='*' || (file_ptrs && type2 == "FILEP")))){
					int size = get_size(type1);
					cout << "movs" << sizechar(size) << "q " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "movq " << "%rax, " << set_offset(t2) << endl;
				}
				else if((type1=="char" || type1=="short int") && type2=="int"){
					int size = get_size(type1);
					cout << "movs" << sizechar(size) << "l " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "movl " << "%eax, " << set_offset(t2) << endl;
				}
				else if((type1=="char") && type2=="short int"){
					int size = get_size(type1);
					cout << "movs" << sizechar(size) << "w " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "movw " << "%ax, " << set_offset(t2) << endl;
				}
				else if((type2=="char" || type2=="short int" || type2=="int") && (type1=="long int" || (type1.back()=='*' || (file_ptrs && type1 == "FILEP")))){
					int size = get_size(type2);
					cout << "movq " << set_offset(t1) << ", " << "%rax" << endl;
					cout << "mov" << sizechar(size) << " " << retregs[size] << ", " << set_offset(t2) << endl;
				}
				else if((type2=="char" || type2=="short int") && type1=="int"){
					int size = get_size(type2);
					cout << "movl " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "mov" << sizechar(size) << " " << retregs[size] << ", " << set_offset(t2) << endl;
				}
				else if((type2=="char") && type1=="short int"){
					int size = get_size(type2);
					cout << "movw " << set_offset(t1) << ", " << "%ax" << endl;
					cout << "mov" << sizechar(size) << " " << retregs[size] << ", " << set_offset(t2) << endl;
				}

				// Between floats and doubles
				else if(type1=="float" && type2=="double"){
					cout << "cvtss2sd " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0, " << set_offset(t2) << endl;
				}
				else if(type1=="double" && type2=="float"){
					cout << "cvtsd2ss " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0, " << set_offset(t2) << endl;
				}

				// From ints to floats/doubles
				else if((type1=="char" || type1=="short int") && type2=="float"){
					int size = get_size(type1);
					cout << "movs" << sizechar(size) << "l " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "cvtsi2ssl " << "%eax, " << "%xmm0" << endl;
					cout << "movss " << "%xmm0, " << set_offset(t2) << endl;
				}
				else if((type1=="int" || type1=="long int") && type2=="float"){
					int size = get_size(type1);
					cout << "cvtsi2ss" << sizechar(size) << " " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movss " << "%xmm0, " << set_offset(t2) << endl;
				}
				else if((type1=="char" || type1=="short int") && type2=="double"){
					int size = get_size(type1);
					cout << "movs" << sizechar(size) << "l " << set_offset(t1) << ", " << "%eax" << endl;
					cout << "cvtsi2sdl " << "%eax, " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0, " << set_offset(t2) << endl;
				}
				else if((type1=="int" || type1=="long int") && type2=="double"){
					int size = get_size(type1);
					cout << "cvtsi2sd" << sizechar(size) << " " << set_offset(t1) << ", " << "%xmm0" << endl;
					cout << "movsd " << "%xmm0, " << set_offset(t2) << endl;
				}

				// From floats/doubles to ints
				else if((type2=="char" || type2=="short int" || type2=="int" || type2=="long int") && type1=="float"){
					int size = get_size(type2);
					cout << "movss " << set_offset(t1) << ", " << "%xmm0" << endl;
					if(type2=="long int")
						cout << "cvttss2siq " << "%xmm0, " << "%rax" << endl;
					else
						cout << "cvttss2sil " << "%xmm0, " << "%eax" << endl;
					cout << "mov" << sizechar(size) << " " << retregs[size] << ", " << set_offset(t2) << endl;
				}

				else if((type2=="char" || type2=="short int" || type2=="int" || type2=="long int") && type1=="double"){
					int size = get_size(type2);
					cout << "movsd " << set_offset(t1) << ", " << "%xmm0" << endl;
					if(type2=="long int")
						cout << "cvttsd2siq " << "%xmm0, " << "%rax" << endl;
					else
						cout << "cvttsd2sil " << "%xmm0, " << "%eax" << endl;
					cout << "mov" << sizechar(size) << " " << retregs[size] << ", " << set_offset(t2) << endl;
				}
			}
		}
	}
	dump_const_labels();
}
