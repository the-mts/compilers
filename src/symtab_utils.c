#include "symtab.h"
using namespace std;
vector<symtab*> table_scope;
vector<typtab*> type_scope;
symtab global;
typtab types_table;
table_tree* st_root;
table_tree* curr;
unordered_map<string, string> equiv_types;

void init_equiv_types(){
	equiv_types.insert({"int","int"});
    equiv_types.insert({"char", "char"});
    equiv_types.insert({"float", "float"});
    equiv_types.insert({"void", "void"});
    equiv_types.insert({"double", "double"});

    equiv_types.insert({"signed", "int"});
    equiv_types.insert({"unsigned", "unsigned int"});

    equiv_types.insert({"long", "long int"});
    equiv_types.insert({"short", "short int"});

    equiv_types.insert({"long int", "long int"});
    equiv_types.insert({"short int", "short int"});

    equiv_types.insert({"signed long", "long int"});
    equiv_types.insert({"signed short", "short int"});
    equiv_types.insert({"signed long int", "long int"});
    equiv_types.insert({"signed short int", "short int"});

    equiv_types.insert({"signed int", "int"});
    equiv_types.insert({"unsigned int", "unsigned int"});

    equiv_types.insert({"unsigned long", "unsigned long int"});
    equiv_types.insert({"unsigned short", "unsigned short int"});
    equiv_types.insert({"unsigned long int", "unsigned long int"});
    equiv_types.insert({"unsigned short int", "unsigned short int"});

    equiv_types.insert({"long double", "long double"});
    equiv_types.insert({"unsigned char", "char"});
}

string get_eqtype(string type){
    vector<pair<int, string>> a;
    unordered_map<string, int> m;
    m["struct"] = 0;
    m["enum"] = 0;
    m["signed"] = 1;
    m["unsigned"] = 1;
    m["short"] = 2;
    m["long"] = 2;
    m["int"] = 3;
    m["double"] = 4;
    m["char"] = 5;
    m["float"] = 6;
    m["void"] = 7;

    string tmp ="";
    for(auto i : type){
        if(i==' '){
            a.push_back({(m.find(tmp)!=m.end() ? m[tmp] : 10)  , tmp});
            tmp = "";
        }
        else tmp += i;
    }
    a.push_back({(m.find(tmp)!=m.end() ? m[tmp] : 10)  , tmp});

    sort(a.begin(), a.end());
    
    if(a[0].first==0){
        if(a.size()!=2) return "";
        else return type;
    }

    string new_type="";
    for(auto i:a){
        if(new_type!="") new_type += " ";
        new_type += i.second;
    }

    if(equiv_types.find(new_type)==equiv_types.end()) return "";
    return equiv_types[new_type];
}

void init_symtab(){
	table_scope.push_back(&global);
	type_scope.push_back(&types_table);
	st_root = new table_tree(&global, &types_table);
	curr = st_root;

}

st_entry* add_entry(string key, string type, unsigned long size, long offset, enum sym_type type_name){
	st_entry * new_entry = new st_entry(type, size, offset, type_name);
	//assert(table_scope.size() != 0);//check scope stack
	symtab * temp = table_scope.back();
	temp->insert({key, new_entry});
	return new_entry;
}

tt_entry* add_type_entry(string key, string type){
	tt_entry * new_entry = new tt_entry;
	typtab * temp = type_scope.back();
	temp->insert({key, new_entry});
	return new_entry;
}

st_entry* lookup(string key){
	for(int i = (int)(table_scope.size())-1; i >= 0; i--){
		auto it = table_scope[i]->find(key);
		if(it != table_scope[i]->end())
			return it->second;
	}
	return NULL;
}

tt_entry* type_lookup(string key){
	for(int i = (int)(type_scope.size())-1; i >= 0; i--){
		auto it = type_scope[i]->find(key);
		if(it != type_scope[i]->end())
			return it->second;
	}
	return NULL;
}

void new_scope(){
	symtab * new_table= new symtab;
	typtab * new_type_table = new typtab;
	table_tree * temp = new table_tree(new_table, new_type_table, curr);
	curr->v.push_back(temp);
	curr = temp;
	table_scope.push_back(new_table);
	type_scope.push_back(new_type_table);
}

void scope_cleanup(){
	curr = curr->par;
	table_scope.pop_back();
}

unsigned long power(int x, int p){
	unsigned long ans = 1;
	for(int i = 0; i < p; i++){
		ans*=p;
	}
	return ans;
}

pair<string, int> get_equivalent_pointer(string s){
	int n = s.length();
	int x = n-1;
	int cnt=0;
	while(s[x]==' ' || s[x]=='[' || s[x]==']' || (s[x]>='0' && s[x]<='9')){
		if(s[x]=='['){
			cnt++;
		}
		x--;
	}
	string s1="";
	int flag = 0;
	int cnt1=0;
	for(int i=0;i<=x;i++){
		s1+=s[i];
		if(s[i]=='*'){
			flag = 1;
			cnt1++;
		}
	}
	if(cnt==0){
		return {s1,cnt1};
	}
	if(cnt1==0){
		s1+=" ";
	}
	for(int i=0;i<cnt;i++){
		s1+="*";
	}
	return {s1, cnt+cnt1};
}

string reduce_pointer_level(string s){
	int n = s.length();
	int cnt = 0, cnt1 = 0, start=-1, end=-1, f=-1;
	for(int i=0;i<n;i++){
		if(s[i]=='*'){
			cnt++;
			f=i;
		}
		if(s[i]=='[')
			cnt1++;
	}
	for(int i=0;i<n;i++){
		if(s[i]=='['){
			start = i;
			break;
		}
	}
	for(int i=0;i<n;i++){
		if(s[i]==']'){
			end = i;
			break;
		}
	}
	if(cnt==0 && cnt1==0){
		printf("\e[1;31mError [line %d]:\e[0m Variable is neither pointer nor array.\n",line);
		exit(-1);
	}
	if(cnt==1 && cnt1==0){
		f-=2;
		string s1="";
		for(int i=0;i<=f;i++){
			s1+=s[i];
		}
		return s1;
	}
	if(cnt1==0){
		f--;
	}
	if(cnt1<=1){
		string s1="";
		for(int i=0;i<=f;i++){
			s1+=s[i];
		}
		return s1;
	}
	string s1="";
	for(int i=0;i<n;i++){
		if(i>=start && i<=end);
		else{
			s1+=s[i];
		}
	}
	return s1;
}

string arithmetic_type_upgrade(string type1, string type2, string op){
	// float double long double int long unsigned int unsigned long int char pointer
	if(type_lookup(type1) || type_lookup(type2)){
		printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, op.c_str());
		exit(-1);
	}
	unordered_map<string, int> m1;
	m1["float"] = 1;
	m1["double"] = 1;
	m1["long double"] = 1;
	m1["int"] = 1;
	m1["long int"] = 1;
	m1["unsigned int"] = 1;
	m1["unsigned long int"] = 1;
	m1["char"] = 1;
	string ans = "arithmetic_typecast did not work as expected";
	if(type1 == "long double"){
		if(m1.count(type2)){
			ans = "long double";
			return ans;
		}
		else{
			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, op.c_str());
			exit(-1);
		}
	}
	if(type2 == "long double"){
		if(m1.count(type1)){
			ans = "long double";
			return ans;
		}
		else{
			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, op.c_str());
			exit(-1);
		}
	}
	if(type1 == "double"){
		if(m1.count(type2)){
			ans = "double";
			return ans;
		}
		else{
			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, op.c_str());
			exit(-1);
		}
	}
	if(type2 == "double"){
		if(m1.count(type1)){
			ans = "double";
			return ans;
		}
		else{
			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, op.c_str());
			exit(-1);
		}
	}
	if(type1 == "float"){
		if(m1.count(type2)){
			ans = "float";
			return ans;
		}
		else{
			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, op.c_str());
			exit(-1);
		}
	}
	if(type2 == "float"){
		if(m1.count(type1)){
			ans = "float";
			return ans;
		}
		else{
			printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, op.c_str());
			exit(-1);
		}
	}
	if(type1 == "unsigned long int"){
		if(m1.count(type2)){
			ans = "unsigned long int";
			return ans;
		}
		else{
			return type2;
		}
	}
	if(type2 == "unsigned long int"){
		if(m1.count(type1)){
			ans = "unsigned long int";
			return ans;
		}
		else{
			return type1;
		}
	}
	if(type1 == "long int" && type2 == "unsigned int"){
		ans = "unsigned long int";
		return ans;
	}
	if(type2 == "long int" && type1 == "unsigned int"){
		ans = "unsigned long int";
		return ans;
	}
	if(type1 == "long int"){
		if(m1.count(type2)){
			ans = "long int";
			return ans;		}

		else{
			return type2;
		}
	}
	if(type2 == "long int"){
		if(m1.count(type1)){
			ans = "long int";
			return ans;
		}
		else{
			return type1;
		}
	}
	if(type1 == "unsigned int"){
		if(m1.count(type2)){
			ans = "unsigned int";
			return ans;		}

		else{
			return type2;
		}
	}
	if(type2 == "unsigned int"){
		if(m1.count(type1)){
			ans = "unsigned int";
			return ans;
		}
		else{
			return type1;
		}
	}
	if(type1 == "int"){
		if(m1.count(type2)){
			ans = "int";
			return ans;		}

		else{
			return type2;
		}
	}
	if(type2 == "int"){
		if(m1.count(type1)){
			ans = "int";
			return ans;
		}
		else{
			return type1;
		}
	}
	if(type1 == "char"){
		if(m1.count(type2)){
			ans = "int";
			return ans;		}

		else{
			return type2;
		}
	}
	if(type2 == "char"){
		if(m1.count(type1)){
			ans = "int";
			return ans;
		}
		else{
			return type1;
		}
	}
	printf("\e[1;31mError [line %d]:\e[0m Incompatible types for operator %s.\n",line, op.c_str());
	exit(-1);
}

pair<constant, enum const_type> parse_constant(string s){
	unordered_map <char, int> m; 
	for(int i=0; i<10; i++){
		m['0'+i] = i;
	}
	for(int i=10; i<16; i++){
		m['A'+i-10] = i;
		m['a'+i-10] = i;
	}
	pair<constant, enum const_type> ans;
	int is_float_or_double = 0;
	int length = s.length();

	for(int i=0; i<length; i++){
		if(s[i] == '.'){
			is_float_or_double = 1;
		}
		if(s[i] == 'e'){
			is_float_or_double = 1;
		}
	}

	if(is_float_or_double){
		if(s[length-1] == 'f' || s[length-1] == 'F'){
			ans.second = IS_FLOAT;
			float temp = 0;
			string tmp_num = "";
			for(int i=0; i<length-1; i++){
				tmp_num+=s[i];
			}
			temp = stof(tmp_num);
			ans.first.float_const = temp;
		}
		else if(s[length-1] == 'l' || s[length-1] == 'L'){
			ans.second = IS_LONG_DOUBLE;
			long double temp = 0;
			string tmp_num = "";
			for(int i=0; i<length-1; i++){
				tmp_num+=s[i];
			}
			temp = stold(tmp_num);
			ans.first.long_double_const = temp;
		}
		else{
			ans.second = IS_DOUBLE;
			double temp = stod(s);
			ans.first.double_const = temp;
		}
	}
	else{
		if(s[0] == '0'){
			if(length>1 && s[1] == 'x'){
				// cout<<"YES\n";
				if(s[length-1] == 'u' || s[length-1] == 'U'){
					if(s[length-2] == 'l' || s[length-2] == 'L'){
						ans.second = IS_U_LONG;
						unsigned long temp = 0;
						for(int i=length-3; i>=2; i--){
							temp+=(unsigned long)(m[s[i]])*(unsigned long)power(16, length-3-i);
						}
						ans.first.u_long_const = temp;
					}
					else{
						ans.second = IS_U_INT;
						unsigned int temp = 0;
						for(int i=length-2; i>=2; i--){
							temp+=(unsigned int)(m[s[i]])*(unsigned int)power(16,length-2-i);
						}
						ans.first.u_int_const = temp;

					}
				}
				else if(s[length-1] == 'l' || s[length-1] == 'L'){
					if(s[length-2] == 'u' || s[length-2] == 'U'){
						ans.second = IS_U_LONG;
						unsigned long temp = 0;
						for(int i=length-3; i>=2; i--){
							temp+=(unsigned long)(m[s[i]])*(unsigned long)power(16, length-3-i);
						}
						ans.first.u_long_const = temp;
					}
					else{
						ans.second = IS_LONG;
						long temp = 0;
						for(int i=length-2; i>=2; i--){
							temp+=(long)(m[s[i]])*(long)power(16,length-2-i);
						}
						ans.first.long_const = temp;
					}
				}
				else{
					ans.second = IS_INT;
					int temp=0;
					for(int i=length-1; i>=2; i--){
						temp+=(int)(m[s[i]])*(int)power(16, length-1-i);
					}
					ans.first.int_const = temp;
				}

			}
			else{
				if(length>1){

					if(s[length-1] == 'u' || s[length-1] == 'U'){
						if(s[length-2] == 'l' || s[length-2] == 'L'){
							ans.second = IS_U_LONG;
							unsigned long temp = 0;
							for(int i=length-3; i>=1; i--){
								temp+=(unsigned long)(m[s[i]])*(unsigned long)power(8, length-3-i);
							}
							ans.first.u_long_const = temp;
						}
						else{
							ans.second = IS_U_INT;
							unsigned int temp = 0;
							for(int i=length-2; i>=1; i--){
								temp+=(unsigned int)(m[s[i]])*(unsigned int)power(8,length-2-i);
							}
							ans.first.u_int_const = temp;

						}
					}
					else if(s[length-1] == 'l' || s[length-1] == 'L'){
						if(s[length-2] == 'u' || s[length-2] == 'U'){
							ans.second = IS_U_LONG;
							unsigned long temp = 0;
							for(int i=length-3; i>=1; i--){
								temp+=(unsigned long)(m[s[i]])*(unsigned long)power(8, length-3-i);
							}
							ans.first.u_long_const = temp;
						}
						else{
							ans.second = IS_LONG;
							long temp = 0;
							for(int i=length-2; i>=1; i--){
								temp+=(long)(m[s[i]])*(long)power(8,length-2-i);
							}
							ans.first.long_const = temp;
						}
					}
					else{
						ans.second = IS_INT;
						int temp = 0;
						for(int i=length-1; i>=1; i--){
							temp+=(int)(m[s[i]])*(int)power(8, length-1-i);
						}
						ans.first.int_const = temp;
					}

				}
				else{
					ans.second = IS_INT;
					int temp = 0;
					temp = 0;
					ans.first.int_const = temp;
				}
			}
		}
		else{
			if(length>1){
				if(s[length-1] == 'u' || s[length-1] == 'U'){
					if(s[length-2] == 'l' || s[length-2] == 'L'){
						ans.second = IS_U_LONG;
						string tmp_num = "";
						for(int i=0; i<length-2; i++){
							tmp_num+=s[i];
						}
						unsigned long temp = stoul(tmp_num);
						ans.first.u_long_const = temp;
					}
					else{
						ans.second = IS_U_INT;
						string tmp_num = "";
						for(int i=0; i<length-1; i++){
							tmp_num+=s[i];
						}
						unsigned int temp = (unsigned int) stoi(tmp_num);
						ans.first.u_int_const = temp;
					}
				}
				else if(s[length-1] == 'l' || s[length-1] == 'L'){
					if(s[length-2] == 'u' || s[length-2] == 'U'){
						ans.second = IS_U_LONG;
						string tmp_num = "";
						for(int i=0; i<length-2; i++){
							tmp_num+=s[i];
						}
						unsigned long temp = stoul(tmp_num);
						ans.first.u_long_const = temp;
					}
					else{
						ans.second = IS_LONG;
						string tmp_num = "";
						for(int i=0; i<length-1; i++){
							tmp_num+=s[i];
						}
						long temp = stol(tmp_num);
						ans.first.long_const = temp;
					}
				}
				else{
					ans.second = IS_INT;
					int temp = stoi(s);
					ans.first.int_const = temp;
				}
			}
			else{
				ans.second = IS_INT;
				int temp = stoi(s);
				ans.first.int_const = temp;
			}

		}

	}
	return ans;
}
