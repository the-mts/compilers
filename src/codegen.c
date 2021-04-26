#include "codegen.h"

vector<string> code;
// unordered_map<int, string> gotoLabels;
// unordered_map<string, > //
map<pair<int, int>, string> intregs;
vector<qi> params_list;

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
			cout<<"movb "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)\n";
			break;
		case 2:
			cout<<"movw "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)\n";
			break;
		case 4:
			cout<<"movl "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)\n";
			break;
		case 8:
			cout<<"movq "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)\n";
			break;
	}
}

void codegen(){
	setregmap();
	for(int bno=0; bno!=-1; bno = blocks[bno].next){
		if(blocks[bno].isglobal){
			//Process global variables
			continue;
		}

		if(blocks[bno].pred.size()!=0 && !(blocks[bno].pred.size()==1 && blocks[bno].pred[0] == bno-1)){
			cout<<".L"<<bno<<":\n";
		}

		for(int i=0; i<blocks[bno].code.size(); i++){
			quad instr = blocks[bno].code[i];
			if (instr.op == "FUNC_START"){
				string fname = instr.op1.first;
				cout<<".text\n";
				cout<<".globl "<<fname<<endl;
				cout<<".type "<<fname<<", @function"<<endl;
				cout<<fname<<":\n";
				cout<<"endbr64\n";
				cout<<"pushq %rbp\n";
				cout<<"movq %rsp, %rbp\n";
				//// cout<<"addq "; //// ADD WIDTH OF TABLE


				int int_char = 0, double_float = 0;
				auto args = *(instr.op1.second->arg_list);
				for(auto p: args){
					int flag = 0;
					if(p.first.find("int") != string::npos || p.first.find("char") != string::npos || p.first.back() == ']' || p.first.back() == '*'){
						flag = 1;
					}
					else if(p.first.find("double") != string::npos || p.first.find("float") != string::npos){
						flag = 2;
					}
					else{
						flag = 3;
					}
					if(flag == 1 && int_char<6){
						int size = get_size(p.first);
						moveintarg(int_char+1, - (*(instr.op1.second->sym_table))[p.second]->offset, size);
						int_char++;
					}
					else if(flag == 2 && double_float<8){
						//// HANDLE FLOAT/DOUBLE ARGS COPY
						double_float++;
					}
				}
			}

			if (instr.op == "RETURN_VOID" || instr.op == "FUNC_END"){
				cout<<"leave\n";
				cout<<"ret\n";
			}

			if (instr.op == "GOTO"){
				cout<<"jmp .L"<<instr.goto_addr<<endl;
			}

			if(instr.op == "IF_TRUE_GOTO"){
				string p = instr.op1.second->type;
				if(p.find("int") != string::npos || p.find("char") != string::npos || p.back() == ']' || p.back() == '*'){
					int size = get_size(p);
					if(p.back() == ']') size = 8;
					cout<<"cmp"<<sizechar(size)<<" $0, "<<-instr.op1.second->offset<<"(%rbp)\n";
					cout<<"jne .L"<<instr.goto_addr<<endl;
				}
			}

			if(instr.op == "PARAM"){
				params_list.push_back(instr.op1);
			}

			if(instr.op == "CALL"){
				int int_char = 0, double_float = 0;
				int param_stk_size = 0;
				vector<qi> stk_params;
				for(auto x: params_list){
					string p = x.second->type;
					int flag = 0;
					if(p.find("int") != string::npos || p.find("char") != string::npos || p.back() == ']' || p.back() == '*'){
						flag = 1;
					}
					else if(p.find("double") != string::npos || p.find("float") != string::npos){
						flag = 2;
					}
					else{
						flag = 3;
					}
					if(flag == 1 && int_char<6){
						int size = get_size(p);
						if(p.back() == ']') size = 8;
						cout<<"mov"<<sizechar(size)<<" "<<-x.second->offset<<"(%rbp), "<<intregs[{int_char+1, size}]<<endl;
						int_char++;
					}
					else if(flag == 2 && double_float<8){
						// HANDLE FLOAT/DOUBLE ARGS COPY
						double_float++;
					}
					else{
						stk_params.push_back(x);
					}
				}
				for(int x = stk_params.size()-1; x>=0; x--){
					string p = stk_params[x].second->type;
					int size = get_size(p);
					if(p.back() == ']') size = 8;
					if (size<=8) cout<<"pushq "<<-stk_params[x].second->offset<<"(%rbp)\n";
					else{
						// HANDLE LONG DOUBLE
						;
					}
					param_stk_size += size;
				}

				cout<<"call "<< instr.op1.first<<endl;
				if(param_stk_size) cout<<"subq $"<<param_stk_size<<", %rsp\n";

				stk_params.clear();
				params_list.clear();
			}

			if(instr.op == "+int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"addl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"addq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"addw "<<-t2.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"addl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"addl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == '*' && type2 == "short int"){
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}
				else if(type2.back() == '*' && type1 == "short int"){
					swap(t1,t2);
					swap(type1,type2);
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}

				else if(type1.back() == '*' && type2 == "int"){
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}
				else if(type2.back() == '*' && type1 == "int"){
					swap(t1,t2);
					swap(type1,type2);
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}

				else if(type1.back() == '*' && type2 == "long int"){
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}
				else if(type2.back() == '*' && type1 == "long int"){
					swap(t1,t2);
					swap(type1,type2);
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}
			}

			if(instr.op == "-int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"subl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"subq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"subw "<<-t2.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"subl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"subl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == '*' && type2 == "short int"){
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}
				else if(type2.back() == '*' && type1 == "short int"){
					swap(t1,t2);
					swap(type1,type2);
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}

				else if(type1.back() == '*' && type2 == "int"){
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}
				else if(type2.back() == '*' && type1 == "int"){
					swap(t1,t2);
					swap(type1,type2);
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}

				else if(type1.back() == '*' && type2 == "long int"){
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}
				else if(type2.back() == '*' && type1 == "long int"){
					swap(t1,t2);
					swap(type1,type2);
					type1.pop_back();
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
				}
			}

			if(instr.op == "*int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"imull "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"imulw "<<-t2.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"imull "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"imull "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "/int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"idivw "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%ecx"<<endl;
					cout<<"idivl "<<"%ecx"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					cout<<"movswl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					cout<<"movswq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					cout<<"movslq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "%"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%edx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"idivw "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<"%dx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%ecx"<<endl;
					cout<<"idivl "<<"%ecx"<<endl;
					cout<<"movl "<<"%edx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					cout<<"movswl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%edx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					cout<<"movswq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					cout<<"movslq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "&"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"andl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"andq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"andw "<<-t2.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"andl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"andl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"andw "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "|"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"orl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"orq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"orw "<<-t2.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"orl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"orl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"orw "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "^"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"xorl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"xorq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"xorw "<<-t2.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"xorl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"xorl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"xorw "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "++x"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout<<"addl $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"addq $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"addw $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "x++"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"addl $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"addq $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"addw $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "--x"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout<<"subl $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"subq $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"subw $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "x--"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"subl $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"subq $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"subw $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
			}

			if(instr.op == "<<"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<-t2.second->offset<<"(%rbp), "<<"%ecx"<<endl;
					cout<<"sall %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<-t2.second->offset<<"(%rbp), "<<"%cx"<<endl;
					cout<<"salw %cl, %ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"sall %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"sall %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}
			if(instr.op == ">>"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"movl "<<-t2.second->offset<<"(%rbp), "<<"%ecx"<<endl;
					cout<<"sarl %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"movq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp), "<<"%ax"<<endl;
					cout<<"movw "<<-t2.second->offset<<"(%rbp), "<<"%cx"<<endl;
					cout<<"sarw %cl, %ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"sarl %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp), "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp), "<<"%eax"<<endl;
					cout<<"sarl %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp), "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp), "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}



		}
	}
}
