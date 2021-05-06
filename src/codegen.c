#include "codegen.h"

vector<string> code;
// unordered_map<int, string> gotoLabels;
// unordered_map<string, > //
map<pair<int, int>, string> intregs;
map<int, string> retregs;
map<pair<int, int>, string> genregs;
vector<qi> params_list;

void dump_const_labels(){
	for (auto i : constLabels){
		cout<<i.first<<":"<<endl;
		cout<<i.second.second<<endl;
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
			cout<<"movb "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)"<<endl;
			break;
		case 2:
			cout<<"movw "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)"<<endl;
			break;
		case 4:
			cout<<"movl "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)"<<endl;
			break;
		case 8:
			cout<<"movq "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)"<<endl;
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
			cout<<".L"<<bno<<":"<<endl;
		}

		for(int i=0; i<blocks[bno].code.size(); i++){
			quad instr = blocks[bno].code[i];
			if (instr.op == "FUNC_START"){
				string fname = instr.op1.first;
				cout<<".text"<<endl;
				cout<<".globl "<<fname<<endl;
				cout<<".type "<<fname<<", @function"<<endl;
				cout<<fname<<":"<<endl;
				cout<<"endbr64"<<endl;
				cout<<"pushq %rbp"<<endl;
				cout<<"movq %rsp, %rbp"<<endl;
				cout<<"subq $320, %rsp"<<endl;


				int int_char = 0, double_float = 0;
				auto args = *(instr.op1.second->arg_list);
				for(auto p: args){
					int flag = 0;
					if(!is_struct_or_union(p.first.first) && (p.first.first.find("int") != string::npos || p.first.first.find("char") != string::npos || p.first.first.back() == ']' || p.first.first.back() == '*')){
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
						double_float++;
					}
				}
			}

			else if(instr.op == "RETURN_VOID" || instr.op == "FUNC_END"){
				cout<<"leave"<<endl;
				cout<<"ret"<<endl;
			}
			
			else if(instr.op == "RETURN"){
				qi t1 = instr.op1;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
				}
				else if(type1.back()=='*'){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				}
				cout<<"leave"<<endl;
				cout<<"ret"<<endl;
			}

			if (instr.op == "GOTO"){
				cout<<"jmp .L"<<instr.goto_addr<<endl;
			}

			else if(instr.op == "IF_TRUE_GOTO"){
				string p = instr.op1.second->type;
				if(p.find("int") != string::npos || p.find("char") != string::npos || p.back() == ']' || p.back() == '*'){
					int size = get_size(p);
					if(p.back() == ']') size = 8;
					cout<<"cmp"<<sizechar(size)<<" $0, "<<-instr.op1.second->offset<<"(%rbp)"<<endl;
					cout<<"jne .L"<<instr.goto_addr<<endl;
				}
			}

			else if(instr.op == "PARAM"){
				params_list.push_back(instr.op1);
			}

			else if(instr.op == "CALL"){
				int int_char = 0, double_float = 0;
				int param_stk_size = 0;
				vector<qi> stk_params;
				for(auto x: params_list){
					if(x.first[0]=='.' && constLabels.find(x.first)!=constLabels.end()){
						auto tmp = constLabels[x.first];
						if (tmp.first == ".string"){
							if(int_char<6){
								cout<<"leaq "<<x.first<<"(%rip)"<<", "<<intregs[{int_char+1, 8}]<<endl;
								int_char++;
								continue;
							}
							else{
								// Should do something here
							}
						}
					}
					string p = x.second->type;
					int flag = 0;
					if(!is_struct_or_union(p) && (p.find("int") != string::npos || p.find("char") != string::npos || p.back() == ']' || p.back() == '*')){
						flag = 1;
					}
					else if(!is_struct_or_union(p) && (p.find("double") != string::npos || p.find("float") != string::npos)){
						flag = 2;
					}
					else{
						flag = 3;
					}
					if(flag == 1 && int_char<6){
						int size = get_size(p);
						if(p.back() == ']') size = 8;
						cout<<"mov"<<sizechar(size)<<" "<<-x.second->offset<<"(%rbp)"<<", "<<intregs[{int_char+1, size}]<<endl;
						int_char++;
					}
					else if(flag == 2 && double_float<8){
						if(p.find("float") != string::npos){
							if(instr.op1.second->type_name == IS_BUILTIN_FUNC){
								cout<<"cvtss2sd "<<-x.second->offset<<"(%rbp)"<<", "<<"%xmm"<<double_float<<endl;
							}
							else{
								cout<<"movss "<<-x.second->offset<<"(%rbp)"<<", "<<"%xmm"<<double_float<<endl;
							}
						}
						else if(p.find("double") != string::npos && p.find("long") == string::npos){
							cout<<"movsd "<<-x.second->offset<<"(%rbp)"<<", "<<"%xmm"<<double_float<<endl;
						}
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
					if (size<=8) cout<<"pushq "<<-stk_params[x].second->offset<<"(%rbp)"<<endl;
					else{
						// HANDLE LONG DOUBLE
						;
					}
					param_stk_size += size;
				}

				cout<<"movl "<<"$"<<double_float<<", "<<"%eax"<<endl;
				cout<<"call "<<instr.op1.first<<endl;
				if(param_stk_size) cout<<"addq $"<<param_stk_size<<", %rsp"<<endl;

				qi t1 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout<<"movl "<<"%eax, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout<<"movw "<<"%ax, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout<<"movq "<<"%rax, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<"%al, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back()=='*'){
					cout<<"movq "<<"%rax, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1=="float"){
					cout<<"movss "<<"%xmm0, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1=="double"){
					cout<<"movsd "<<"%xmm0, "<<-t1.second->offset<<"(%rbp)"<<endl;
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
				/*if (args.size() != params_list.size()) {
					cout<<"Args: "<<args.size()<<", Params: "<<params_list.size()<<endl;
					cout<<"Dead!"<<endl;
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
										cout<<"movss ";
									} else {
										cout<<"movsd ";
									}
									cout<<-param.second->offset<<"(%rbp), "<<"%xmm"<<freef<<endl; 
									freef++;
								}
								else {
									size = get_size(type);
									param2reg[var] = genregs[{freei, size}];
									cout<<"mov"<<sizechar(size)<<" "<<-param.second->offset<<"(%rbp), "<<param2reg[var]<<endl;
									freei++;
								}
								break;
							}
						}
					}
				}*/
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
								cout<<"movss ";
							} else {
								cout<<"movsd ";
							}
							cout<<-param.second->offset<<"(%rbp), "<<"%xmm"<<freef<<endl; 
							freef++;
						}
						else {
							size = get_size(type);
							param2reg[var] = genregs[{freei, size}];
							cout<<"mov"<<sizechar(size)<<" "<<-param.second->offset<<"(%rbp), "<<param2reg[var]<<endl;
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
							cout<<"movsd "<<param2reg[var]<<", "<< -(*(f.second->sym_table))[arg]->offset <<"(%rbp)"<<endl;
						}
						else if (type == "float"){
							cout<<"movss "<<param2reg[var]<<", "<<  -(*(f.second->sym_table))[arg]->offset  <<"(%rbp)"<<endl;
						}
						/*else if (size == 8){
							cout<<"movq "<<param2reg[var]<<", "<<  -(*(f.second->sym_table))[arg]->offset <<"(%rbp)"<<endl;
						}*/
						else {
							cout<<"mov"<<sizechar(size)<<" "<<param2reg[var]<<", "<< -(*(f.second->sym_table))[arg]->offset  <<"(%rbp)"<<endl;
						}
					}
					//return;
				}
				cout<<"jmp .L"<<instr.goto_addr<<endl;
				//return;
				params_list.clear();
			}

			else if(instr.op == "+int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if((type1 == "int" && type2 == "int")){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"addl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "long int")){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "short int" && type2 == "short int")){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"addw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "char" && type2 == "char")){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"addb "<<-t2.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "short int" && type2 == "char")){
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"addw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "short int" && type1 == "char")){
					swap(t1,t2);
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"addw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "int" && type2 == "char")){
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"addl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "int" && type1 == "char")){
					swap(t1,t2);
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"addl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "int" && type2 == "short int")){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"addl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "int" && type1 == "short int")){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"addl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "char")){
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "char")){
					swap(t1,t2);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "short int")){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "short int")){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if((type1 == "long int" && type2 == "int")){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "int")){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back() == '*' && (type2 == "char")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2.back() == '*' && (type1 == "char")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back() == '*' && (type2 == "short int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2.back() == '*' && (type1 == "short int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == '*' && (type2 == "int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2.back() == '*' && (type1 == "int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == '*' && (type2 == "long int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2.back() == '*' && (type1 == "long int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == ']' && (type2 == "char")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2.back() == ']' && (type1 == "char")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back() == ']' && (type2 == "short int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2.back() == ']' && (type1 == "short int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == ']' && (type2 == "int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2.back() == ']' && (type1 == "int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == ']' && (type2 == "long int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2.back() == ']' && (type1 == "long int")){
					swap(t1,t2);
					swap(type1,type2);
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rax"<<endl;
					cout<<"addq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "-int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if((type1 == "int" && type2 == "int")){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"subl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "long int")){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "short int" && type2 == "short int")){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"subw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "char" && type2 == "char")){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"subb "<<-t2.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "short int" && type2 == "char")){
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"subw "<<"%cx, "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "short int" && type1 == "char")){
					cout<<"movsbw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"subw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "int" && type2 == "char")){
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"subl "<<"%ecx, "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "int" && type1 == "char")){
					cout<<"movsbl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"subl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "int" && type2 == "short int")){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"subl "<<"%ecx, "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "int" && type1 == "short int")){
					cout<<"movswl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"subl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "char")){
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<"%rcx, "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "char")){
					cout<<"movsbq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "short int")){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<"%rcx, "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "short int")){
					cout<<"movswq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if((type1 == "long int" && type2 == "int")){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<"%rcx, "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "int")){
					cout<<"movslq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back() == '*' && (type2 == "char")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<"%rcx, "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back() == '*' && (type2 == "short int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<"%rcx, "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back() == '*' && (type2 == "int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<"%rcx, "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back() == '*' && (type2 == "long int")){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"imulq $"<<size<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<"%rcx, "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back() == '*' && type2.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"subq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"movq $"<<size<<", %rcx"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "*int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if((type1 == "int" && type2 == "int")){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"imull "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "long int")){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "short int" && type2 == "short int")){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"imulw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "char" && type2 == "char")){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"imulb "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "short int" && type2 == "char")){
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"imulw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "short int" && type1 == "char")){
					swap(t1,t2);
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"imulw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if((type1 == "int" && type2 == "char")){
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"imull "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "int" && type1 == "char")){
					swap(t1,t2);
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"imull "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "int" && type2 == "short int")){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"imull "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "int" && type1 == "short int")){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"imull "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "char")){
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "char")){
					swap(t1,t2);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "short int")){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "short int")){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1 == "long int" && type2 == "int")){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2 == "long int" && type1 == "int")){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"imulq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "/int"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"cwtd"<<endl;
					cout<<"idivw "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout<<"movsbl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<"%ecx"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"cwtd"<<endl;
					cout<<"idivw "<<"%cx"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					cout<<"movsbw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"cwtd"<<endl;
					cout<<"idivw "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<"%ecx"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "char"){
					cout<<"movsbl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<"%ecx"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					cout<<"movswl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					cout<<"movsbq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					cout<<"movswq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					cout<<"movslq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				// else if(type1 == "unsigned long int"){
				// 	if(type2 == "unsigned long int" || type2 == "long int"){
				// 		cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				// 		cout<<"movl $0, %edx"<<endl;
				// 		cout<<"divq "<<-t2.second->offset<<"(%rbp)"<<endl;
				// 		cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// 	}
				// 	else if(type2 == "unsigned int" || type2 == "int"){
				// 		cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				// 		cout<<"movl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
				// 		cout<<"movl $0, %edx"<<endl;
				// 		cout<<"divq "<<"%rcx"<<endl;
				// 		cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// 	}
				// }
				// else if(type2 == "unsigned long int"){
				// 	if(type1 == "unsigned long int" || type1 == "long int"){
				// 		cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				// 		cout<<"movl $0, %edx"<<endl;
				// 		cout<<"divq "<<-t2.second->offset<<"(%rbp)"<<endl;
				// 		cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// 	}
				// 	else if(type1 == "unsigned int" || type1 == "int"){
				// 		cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
				// 		cout<<"movl $0, %edx"<<endl;
				// 		cout<<"divq "<<-t2.second->offset<<"(%rbp)"<<endl;
				// 		cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// 	}
				// }
				/* CHECK THE NEXT TWO RULES */

				// else if(type1 == "long int" && type2 == "unsigned int"){
				// 	cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				// 	cout<<"movl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
				// 	cout<<"movl $0, %edx"<<endl;
				// 	cout<<"divq "<<"%rcx"<<endl;
				// 	cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// }
				// else if(type1 == "unsigned int" && type2 == "long int"){
				// 	cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
				// 	cout<<"movl $0, %edx"<<endl;
				// 	cout<<"divq "<<-t2.second->offset<<"(%rbp)"<<endl;
				// 	cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// }

				// else if(type1 == "unsigned int"){
				// 	if(type2 == "unsigned int"){
				// 		cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
				// 		cout<<"mov $0, %edx"<<endl;
				// 		cout<<"divl "<<-t2.second->offset<<"(%rbp)"<<endl;
				// 		cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// 	}
				// 	else if(type2 == "int"){
				// 		cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
				// 		cout<<"mov $0, %edx"<<endl;
				// 		cout<<"divl "<<-t2.second->offset<<"(%rbp)"<<endl;
				// 		cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// 	}
				// }
				// else if(type2 == "unsigned int"){
				// 	if(type1 == "unsigned int"){
				// 		cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
				// 		cout<<"mov $0, %edx"<<endl;
				// 		cout<<"divl "<<-t2.second->offset<<"(%rbp)"<<endl;
				// 		cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// 	}
				// 	else if(type1 == "int"){
				// 		cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
				// 		cout<<"mov $0, %edx"<<endl;
				// 		cout<<"divl "<<-t2.second->offset<<"(%rbp)"<<endl;
				// 		cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				// 	}
				// }
			}

			else if(instr.op == "%"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%edx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"cwtd"<<endl;
					cout<<"idivw "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<"%dx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout<<"movsbl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<"%ecx"<<endl;
					cout<<"movb "<<"%dl, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"cwtd"<<endl;
					cout<<"idivw "<<"%cx"<<endl;
					cout<<"movw "<<"%dx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					cout<<"movsbw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"cwtd"<<endl;
					cout<<"idivw "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<"%dx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<"%ecx"<<endl;
					cout<<"movl "<<"%edx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "char"){
					cout<<"movsbl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%edx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<"%ecx"<<endl;
					cout<<"movl "<<"%edx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					cout<<"movswl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"cltd"<<endl;
					cout<<"idivl "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<"%edx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					cout<<"movsbq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					cout<<"movswq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<"%rcx"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					cout<<"movslq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"cqto"<<endl;
					cout<<"idivq "<<-t2.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<"%rdx, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "&"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"andl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"andq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"andw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"andb "<<-t2.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"andw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1, t2);
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"andw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"andl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"andl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"andl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"andl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"andq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "|"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"orl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"orq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"orw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"orb "<<-t2.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"orw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1, t2);
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"orw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"orl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"orl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"orl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"orl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"orq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "^"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"xorl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"xorq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"xorw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"xorb "<<-t2.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"xorw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1, t2);
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"xorw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"xorl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"xorl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"xorl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"xorl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"xorq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "++x"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout<<"addl $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"addq $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"addw $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"addb $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"addq $"<<size<<", "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "x++"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"addl $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"addw $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"addq $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"addb $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"addq $"<<size<<", "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "--x"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout<<"subl $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"subq $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"subw $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"subb $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"subq $"<<size<<", "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "x--"){
				qi t1 = instr.op1;
				string type1 = instr.op1.second->type;
				if(type1 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"subl $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"subq $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"subw $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"subb $1, "<<-t1.second->offset<<"(%rbp)"<<endl;
				}

				else if(type1.back() == '*'){
					type1 = reduce_pointer_level(type1);
					int size = get_size(type1);
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
					cout<<"subq $"<<size<<", "<<-t1.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "<<"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"sall %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"salw %cl, %ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cl"<<endl;
					cout<<"salb %cl, %al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"salw %cl, %ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"salw %cl, %ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"sall %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"sall %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"sall %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"sall %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"salq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == ">>"){
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				string type1 = instr.op1.second->type, type2 = instr.op2.second->type;
				if(type1 == "int" && type2 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"sarl %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"sarw %cl, %ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char" && type2 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cl"<<endl;
					cout<<"sarb %cl, %al"<<endl;
					cout<<"movb "<<"%al, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" && type2 == "char"){
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"sarw %cl, %ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "short int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbw "<<-t2.second->offset<<"(%rbp)"<<", "<<"%cx"<<endl;
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"sarw %cl, %ax"<<endl;
					cout<<"movw "<<"%ax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "char"){
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"sarl %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"sarl %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "int" && type2 == "short int"){
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"sarl %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswl "<<-t2.second->offset<<"(%rbp)"<<", "<<"%ecx"<<endl;
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"sarl %cl, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "char"){
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "char"){
					swap(t1,t2);
					cout<<"movsbq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "short int"){
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "short int"){
					swap(t1,t2);
					cout<<"movswq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" && type2 == "int"){
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
				else if(type2 == "long int" && type1 == "int"){
					swap(t1,t2);
					cout<<"movslq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"sarq %cl, %rax"<<endl;
					cout<<"movq "<<"%rax, "<<-instr.res.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "<int" || instr.op == ">int" || instr.op == "<=int" || 
					instr.op == ">=int" || instr.op == "==int" || instr.op == "!=int"){
				string op = instr.op;
				qi t1 = instr.op1;
				qi t2 = instr.op2;
				qi r  = instr.res;
				string type1 = t1.second->type;
				if (type1.back() == '*') type1 = "long int";
				string type2 = t2.second->type;
				if (type1.back() == '*') type2 = "long int";
				map<string, int> rank = {{"char", 1}, {"short int", 2}, {"int", 4}, {"long int", 8}};
				
				if (rank[type1] > rank[type2]){
					swap(t1, t2);
					swap(type1, type2);
					if (op == "<int") op = ">int";
					else if (op == ">int") op = "<int";
					else if (op == "<=int") op = ">=int";
					else if (op == ">=int") op = "<=int";
				}

				if (type1 == "char") 			cout<<"movsbq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				else if (type1 == "short int")  cout<<"movswq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				else if (type1 == "int") 		cout<<"movslq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				else if (type1 == "long int") 	cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;

				if (type2 == "char") 			cout<<"cmpb "<<-t2.second->offset<<"(%rbp)"<<", %al"<<endl;	
				else if (type2 == "short int")	cout<<"cmpw "<<-t2.second->offset<<"(%rbp)"<<", %ax"<<endl;	
				else if (type2 == "int") 		cout<<"cmpl "<<-t2.second->offset<<"(%rbp)"<<", %eax"<<endl;	
				else if (type2 == "long int") 	cout<<"cmpq "<<-t2.second->offset<<"(%rbp)"<<", %rax"<<endl;

				cout<<"set";
				if (op == "<int") 		cout<<"l";
				else if (op == ">int") 	cout<<"g";
				else if (op == "<=int")	cout<<"le";
				else if (op == ">=int") cout<<"ge";
				else if (op == "==int") cout<<"e";
				else if (op == "!=int") cout<<"ne";
				cout<<" %al"<<endl;

				cout<<"movzbl %al, %eax"<<endl;
				cout<<"movl %eax, "<<-r.second->offset<<"(%rbp)"<<endl;

			}

			else if(instr.op == "UNARY+"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back()=='*'){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "UNARY-"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"negl "<<"%eax"<<endl;
					cout<<"movl "<<"%eax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"negw "<<"%ax"<<endl;
					cout<<"movw "<<"%ax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"negq "<<"%rax"<<endl;
					cout<<"movq "<<"%rax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"negb "<<"%al"<<endl;
					cout<<"movb "<<"%al"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "UNARY!"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout<<"cmpl "<<"$0, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"sete %al"<<endl;
					cout<<"movzbl %al, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout<<"cmpw "<<"$0, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"sete %al"<<endl;
					cout<<"movzbl %al, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout<<"cmpq "<<"$0, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"sete %al"<<endl;
					cout<<"movzbl %al, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"cmpb "<<"$0, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"sete %al"<<endl;
					cout<<"movzbl %al, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1.back()=='*'){
					cout<<"cmpq "<<"$0, "<<-t1.second->offset<<"(%rbp)"<<endl;
					cout<<"sete %al"<<endl;
					cout<<"movzbl %al, %eax"<<endl;
					cout<<"movl "<<"%eax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "UNARY~"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				if(type1 == "int" || type1 == "unsigned int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"notl "<<"%eax"<<endl;
					cout<<"movl "<<"%eax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"notw "<<"%ax"<<endl;
					cout<<"movw "<<"%ax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"notq "<<"%rax"<<endl;
					cout<<"movq "<<"%rax"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"notb "<<"%al"<<endl;
					cout<<"movb "<<"%al"<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "UNARY&"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				cout<<"leaq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
				cout<<"movq "<<"%rax, "<<-t2.second->offset<<"(%rbp)"<<endl;
			}

			else if(instr.op == "UNARY*"){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t2.second->type;
				cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
				if(type1 == "int" || type1 == "unsigned int"){
					cout<<"movl "<<"(%rcx)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "short int" || type1 == "unsigned short int"){
					cout<<"movw "<<"(%rcx)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "long int" || type1 == "unsigned long int" || type1.back()=='*'){
					cout<<"movq "<<"(%rcx)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<"(%rcx)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
			}

			else if(instr.op == "="){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t2.second->type;
				if(t1.first[0]=='$'){ //Immediate
					if(type1 == "int" || type1 == "unsigned int"){
						cout<<"movl "<<t1.first<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "short int" || type1 == "unsigned short int"){
						cout<<"movw "<<t1.first<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "long int" || type1 == "unsigned long int"){
						cout<<"movq "<<t1.first<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "char"){
						cout<<"movb "<<t1.first<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1.back()=='*'){
						cout<<"movq "<<t1.first<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
				}
				else if(t1.first[0]=='.'){ //RIP address for floats
					if(type1 == "float"){
						cout<<"movss "<<t1.first<<"(%rip)"<<", "<<"%xmm0"<<endl;
						cout<<"movss "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "double"){
						cout<<"movsd "<<t1.first<<"(%rip)"<<", "<<"%xmm0"<<endl;
						cout<<"movsd "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "long double"){
						cout<<"fldt "<<t1.first<<"(%rip)"<<endl;
						cout<<"fstpt "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
				}
				else{
					if(type1 == "int"){
						cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
						cout<<"movl "<<"%eax, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "short int"){
						cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
						cout<<"movw "<<"%ax, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "long int"){
						cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
						cout<<"movq "<<"%rax, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "char"){
						cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
						cout<<"movb "<<"%al, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1.back()=='*'){
						cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
						cout<<"movq "<<"%rax, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}

					else if(type1 == "float"){
						cout<<"movss "<<-t1.second->offset<<"(%rbp)"<<", "<<"%xmm0"<<endl;
						cout<<"movss "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
					else if(type1 == "double"){
						cout<<"movsd "<<-t1.second->offset<<"(%rbp)"<<", "<<"%xmm0"<<endl;
						cout<<"movsd "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
					}
				}
			}
			else if(instr.op == "ADDR="){
				qi t1 = instr.op1;
				qi t2 = instr.res;
				string type1 = t1.second->type;
				cout<<"movq "<<-t2.second->offset<<"(%rbp)"<<", "<<"%rcx"<<endl;
				if(type1 == "int"){
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<"(%rcx)"<<endl;
				}
				else if(type1 == "short int"){
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<"(%rcx)"<<endl;
				}
				else if(type1 == "long int"){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<"(%rcx)"<<endl;
				}
				else if(type1 == "char"){
					cout<<"movb "<<-t1.second->offset<<"(%rbp)"<<", "<<"%al"<<endl;
					cout<<"movb "<<"%al, "<<"(%rcx)"<<endl;
				}
				else if(type1.back()=='*'){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<"(%rcx)"<<endl;
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

				// Between ints and pointers
				if(type1.back()=='*' && type2.back()=='*'){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				if(type1.back()==']' && type2.back()=='*'){
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1=="char" || type1=="short int" || type1=="int") && (type2=="long int" || type2.back()=='*')){
					int size = get_size(type1);
					cout<<"movs"<<sizechar(size)<<"q "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"movq "<<"%rax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1=="char" || type1=="short int") && type2=="int"){
					int size = get_size(type1);
					cout<<"movs"<<sizechar(size)<<"l "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"movl "<<"%eax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1=="char") && type2=="short int"){
					int size = get_size(type1);
					cout<<"movs"<<sizechar(size)<<"w "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"movw "<<"%ax, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2=="char" || type2=="short int" || type2=="int") && (type1=="long int" || type1.back()=='*')){
					int size = get_size(type2);
					cout<<"movq "<<-t1.second->offset<<"(%rbp)"<<", "<<"%rax"<<endl;
					cout<<"mov"<<sizechar(size)<<" "<<retregs[size]<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2=="char" || type2=="short int") && type1=="int"){
					int size = get_size(type2);
					cout<<"movl "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"mov"<<sizechar(size)<<" "<<retregs[size]<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type2=="char") && type1=="short int"){
					int size = get_size(type2);
					cout<<"movw "<<-t1.second->offset<<"(%rbp)"<<", "<<"%ax"<<endl;
					cout<<"mov"<<sizechar(size)<<" "<<retregs[size]<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}

				// Between floats and doubles
				else if(type1=="float" && type2=="double"){
					cout<<"cvtss2sd "<<-t1.second->offset<<"(%rbp)"<<", "<<"%xmm0"<<endl;
					cout<<"movsd "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if(type1=="double" && type2=="float"){
					cout<<"cvtsd2ss "<<-t1.second->offset<<"(%rbp)"<<", "<<"%xmm0"<<endl;
					cout<<"movss "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}

				// From ints to floats/doubles
				else if((type1=="char" || type1=="short int") && type2=="float"){
					int size = get_size(type1);
					cout<<"movs"<<sizechar(size)<<"l "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"cvtsi2ssl "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movss "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1=="int" || type1=="long int") && type2=="float"){
					int size = get_size(type1);
					cout<<"cvtsi2ss"<<sizechar(size)<<" "<<-t1.second->offset<<"(%rbp)"<<", "<<"%xmm0"<<endl;
					cout<<"movss "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1=="char" || type1=="short int") && type2=="double"){
					int size = get_size(type1);
					cout<<"movs"<<sizechar(size)<<"l "<<-t1.second->offset<<"(%rbp)"<<", "<<"%eax"<<endl;
					cout<<"cvtsi2sdl "<<"%eax, "<<"%xmm0"<<endl;
					cout<<"movsd "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
				else if((type1=="int" || type1=="long int") && type2=="double"){
					int size = get_size(type1);
					cout<<"cvtsi2sd"<<sizechar(size)<<" "<<-t1.second->offset<<"(%rbp)"<<", "<<"%xmm0"<<endl;
					cout<<"movsd "<<"%xmm0, "<<-t2.second->offset<<"(%rbp)"<<endl;
				}

				// From floats/doubles to ints
				else if((type2=="char" || type2=="short int" || type2=="int" || type2=="long int") && type1=="float"){
					int size = get_size(type2);
					cout<<"movss "<<-t1.second->offset<<"(%rbp)"<<", "<<"%xmm0"<<endl;
					if(type2=="long int")
						cout<<"cvttss2siq "<<"%xmm0, "<<"%rax"<<endl;
					else
						cout<<"cvttss2sil "<<"%xmm0, "<<"%eax"<<endl;
					cout<<"mov"<<sizechar(size)<<" "<<retregs[size]<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}

				else if((type2=="char" || type2=="short int" || type2=="int" || type2=="long int") && type1=="double"){
					int size = get_size(type2);
					cout<<"movsd "<<-t1.second->offset<<"(%rbp)"<<", "<<"%xmm0"<<endl;
					if(type2=="long int")
						cout<<"cvttsd2siq "<<"%xmm0, "<<"%rax"<<endl;
					else
						cout<<"cvttsd2sil "<<"%xmm0, "<<"%eax"<<endl;
					cout<<"mov"<<sizechar(size)<<" "<<retregs[size]<<", "<<-t2.second->offset<<"(%rbp)"<<endl;
				}
			}
		}
	}
	dump_const_labels();
}
