vector<string> code;
// unordered_map<int, string> gotoLabels;
// unordered_map<string, > //
unordered_map<pair<int, int>, string> intregs;

void setregmap();

char sizechar(int size){
	switch (size)
	{
		case 1: return 'b';
		case 2: return 'w';
		case 4: return 'l';
		case 8: return 'q';
	}
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
			cout<<"movb "<<intregs[{arg, size}]<<", "<<offset<<"(%rbp)\n";
			break;
	}
}

{
	int int_char = 0, double_float = 0;
	int curr_offset = -16;
	for(auto p: func_params){
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
			st_entry* st = add_entry(p.second, p.first, get_size(p.first), accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
			int_char++;
		}
		else if(flag == 2 && double_float<8){
			st_entry* st = add_entry(p.second, p.first, get_size(p.first), accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
			double_float++;
		}
		else if(flag == 1 && int_char == 6){
			st_entry* st = add_entry(p.second, p.first, 0, accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
			st->size = get_size(p.first);
			st->offset = curr_offset;
			curr_offset-=get_size(p.first);
		}
		else if(flag == 2 && double_float == 8){
			st_entry* st = add_entry(p.second, p.first, 0, accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
			st->size = get_size(p.first);
			st->offset = curr_offset;
			curr_offset-=get_size(p.first);
		}
		else if(flag == 3){
			st_entry* st = add_entry(p.second, p.first, 0, accumulate(offset.begin()+1, offset.end(), 0), IS_VAR);    //IS_VAR to be changed
			st->size = get_size(p.first);
			st->offset = curr_offset;
			curr_offset-=get_size(p.first);
		}
	}
}

void codegen(){
	setregmap();
	for(int bno=0; bno<blocks.size(); bno++){
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
				cout<<"pushq %rsp\n";
				cout<<"movq %rsp, %rbp\n";

				// Copy Func parameters to stack
				int int_char = 0, double_float = 0;
				auto args = instr.op1.second;
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
						moveintarg(int_char+1, -instr.op1.second->offset, size);
						int_char++;
					}
					else if(flag == 2 && double_float<8){
						// HANDLE FLOAT/DOUBLE ARGS COPY
						double_float++;
					}
				}
			}

			if (instr.op == "RETURN_VOID"){
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

			if(instr.op == "CALL"){
				for(int j = i-instr.goto_addr; j<i; j++){
					// TO DO
				}
			}
		}
	}
}