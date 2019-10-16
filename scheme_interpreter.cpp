#include <iostream>
#include <sstream>
#include <string>

using namespace std;

#define Hash_size 1031
#define FL_size 30
#define num_predefined_symbols 19
#define max_num_params 10

#define NIL 0
#define BLANK_A 32
#define LEFT_PAREN_A 40
#define RIGHT_PAREN_A 41
#define LEFT_PAREN_H 1
#define RIGHT_PAREN_H 2
#define EOS "EOS"

void garbage_collection(int);

string predefined_symbols[num_predefined_symbols] = { EOS, "(", ")", "#t", "#f", "+", "-", "*", "number?", "symbol?",
		"null?", "cons", "cond", "else", "car", "cdr", "define", "quote", "lambda" };

// error를 출력,
void print_error(int error_type) {
	cout << "*** Error ***" << endl;
	cout << "Error Code: ";
	if (error_type == 2) {
		cout << "Hash Table Fully Occupied Error(2)" << endl << endl;
		exit(2);
	}
	if (error_type == 3) {
		cout << "Cond Without Else Error(3)" << endl << endl;
		exit(3);
	}
}

string str;
int str_idx;
// char i가 괄호라면, return true,
bool is_parenthesis(char i) {
	if ((int)i == LEFT_PAREN_A || (int)i == RIGHT_PAREN_A) return true;
	return false;
}
// char i가 BLANK라면, return true,
bool is_blank(char i) {
	if ((int)i == BLANK_A) return true;
	return false;
}
// str이 모두 BLANK라면, return true,
bool all_blank(const string& str) {
	for (int i = 0; i < str.size(); ++i)
		if ((int)str[i] != BLANK_A)
			return true;
	return false;
}
void get_string() {
	str_idx = 0;
	cout << "> ";
	do {
		getline(cin, str);
	} while (!all_blank(str));
	for (int i = 0; i < str.size(); ++i)
		str[i] = tolower(str[i]);
}
string get_next_token() {
	string ret;
	if (str_idx < str.size()) {
		// 최초에 공백인 경우 무시,
		while ((int)str[str_idx] == BLANK_A)
			++str_idx;
		if (!is_parenthesis(str[str_idx])) {
			while (!is_blank(str[str_idx]) && !is_parenthesis(str[str_idx]) && str_idx < str.size())
				ret.push_back(str[str_idx++]);
			return ret;
		}
		else return ret = str[str_idx++];
	}
	else return EOS;
}
void push_back_to_paren() {
	if ((int)str[str_idx] == LEFT_PAREN_A) --str_idx;
	while ((int)str[str_idx] != LEFT_PAREN_A) --str_idx;
}
void push_back_to_blank() {
	if ((int)str[str_idx] == (int)' ') --str_idx;
	while ((int)str[str_idx] != (int)' ') --str_idx;
}

string hash_str[Hash_size];
int hash_ptr[Hash_size];
int hash_occupied[Hash_size];
// 현재까지 저장된 원소의 수,
int hash_num_elem;
// Hash Table에 저장,
// Table에서 저장 된 idx 반환,
int hash_save(const string& input) {
	for (int i = 0; i < num_predefined_symbols; ++i) {
		if (input == predefined_symbols[i])
			return i;
	}
	// Hash Table이 모두 Occupied된 경우 예외 처리,
	if (hash_num_elem >= Hash_size) print_error(2);
	int total = 0;
	for (int i = 0; i < input.size(); ++i)
		total += (int)input[i] * (int)input[i];
	total %= Hash_size;
	// 해당 hash 값이 비어 있는 경우,
	if (hash_occupied[total] == 0) {
		hash_str[total] = input;
		hash_occupied[total] = 1;
	}
	// 해당 hash 값이 채워져 있는 경우,
	else {
		while (hash_occupied[total] != 0) {
			++total;
			if (total == Hash_size)
				total = 0;
		}
		hash_str[total] = input;
		hash_occupied[total] = 1;
	}
	++hash_num_elem;
	return total;
}
void hash_init() {
	for (int i = 0; i < Hash_size; ++i) {
		hash_ptr[i] = 0;
		hash_occupied[i] = 0;
	}
	for (int i = 0; i < num_predefined_symbols; ++i) {
		hash_str[i] = predefined_symbols[i];
		hash_occupied[i] = 1;
	}
	hash_num_elem = num_predefined_symbols;
}
// 해당 문자열이 있는 경우, idx 반환,
// 해당 문자열이 없는 경우, -1 반환,
int hash_find(const string& input) {
	for (int i = 0; i < num_predefined_symbols; ++i) {
		if (input == predefined_symbols[i])
			return i;
	}
	int total = 0;
	for (int i = 0; i < input.size(); ++i)
		total += (int)input[i] * (int)input[i];
	total %= Hash_size;
	while (hash_str[total] != input) {
		++total;
		if (total == Hash_size) total = 0;
		if (hash_occupied[total] == 0) return -1;
	}
	return total;
}
// 해당 문자열이 없는 경우, 저장 & idx 반환,
// 해당 문자열이 있는 경우, idx 반환,
int get_hash_value(const string& input) {
	// input이 predefined_symbols인 경우,
	for (int i = 0; i < num_predefined_symbols; ++i) {
		if (input == predefined_symbols[i])
			return i;
	}
	if (hash_find(input) != -1) return hash_find(input);
	return hash_save(input);
}
// Hash Table 출력,
void print_hash_all() {
	cout << ":::HASH TABLE:::" << endl;
	for (int i = 1; i < Hash_size; ++i) {
		if (hash_occupied[i] == 1)
			cout << "HASH VALUE: " << i << " SYMBOL: " << hash_str[i] << " LINK: " << hash_ptr[i] << endl;
	}
	cout << endl;
}

int mem_left[FL_size + 1];
int mem_right[FL_size + 1];
// 현재 명령어의 시작 node,
// preprocessing function에서 설정,
int processing_root;
// Garbage Collection 방지 표시,
bool keep_mem[FL_size + 1];
int fl_head;
void fl_init() {
	fl_head = 1;
	mem_left[0] = mem_right[0] = 0;
	for (int i = 1; i < FL_size; ++i) {
		mem_right[i] = i + 1;
		keep_mem[i] = false;
	}
	mem_left[FL_size] = mem_right[FL_size] = 0; keep_mem[FL_size] = false;
}
int alloc() {
	int ret = fl_head;
	fl_head = mem_right[fl_head];
	mem_right[ret] = NIL;
	if (fl_head == FL_size - 1) garbage_collection(ret);
	return ret;
}
void dealloc(int temp) {
	mem_left[temp] = 0;
	mem_right[temp] = fl_head;
	fl_head = temp;
}
void print_mem_all() {
	cout << ":::MEMORY TABLE:::" << endl;
	for (int i = 1; i < FL_size; ++i)
		cout << "NODE: " << i << " LEFT: " << mem_left[i] << " RIGHT: " << mem_right[i] << " FLAG: " << keep_mem[i] << endl;
	cout << endl;
}
void print_list(int root) {
	if (mem_left[root] >= 0) cout << "Current: " << root << " Left: " << mem_left[root] << " Right: " << mem_right[root] << endl;
	else cout << "Current: " << root << " Left: " << hash_str[-mem_left[root]] << " Right: " << mem_right[root] << endl;
	if (mem_left[root] > 0) print_list(mem_left[root]);
	if (mem_right[root] > 0) print_list(mem_right[root]);
}

class param_stack {
public:
	param_stack() : num_layers(4), current_layer(-1) {
		stack = new int*[num_layers];
		for (int i = 0; i < num_layers; ++i)
			stack[i] = new int[max_num_params];
	}
	bool isempty() const {
		return current_layer == -1;
	}
	bool top(int* _param_values, int _num_params) {
		if (isempty()) return false;
		for (int i = 0; i < _num_params; ++i) {
			_param_values[i] = stack[current_layer][i];
		}
		return true;
	}
	void push(int* _param_values, int _num_params) {
		if (current_layer == num_layers - 1) {
			int** temp = new int*[num_layers * 2];
			for (int i = 0; i < num_layers * 2; ++i)
				temp[i] = new int[max_num_params];
			for (int i = 0; i < num_layers; ++i) {
				for (int j = 0; j < max_num_params; ++j)
					temp[i][j] = stack[i][j];
			}
			// previous memory deallocation,
			for (int i = 0; i < num_layers; ++i)
				delete[] stack[i];
			delete[] stack;
			// change to new memory,
			stack = temp;
			num_layers *= 2;
		}
		++current_layer;
		// Init,
		for (int i = 0; i < max_num_params; ++i)
			stack[current_layer][i] = -1;
		for (int i = 0; i < _num_params; ++i)
			stack[current_layer][i] = _param_values[i];
	}
	bool pop(int *_param_values, int _num_params) {
		if (isempty()) return false;
		for (int i = 0; i < _num_params; ++i) {
			_param_values[i] = stack[current_layer][i];
		}
		--current_layer;
		return true;
	}
	~param_stack() {
		for (int i = 0; i < num_layers; ++i)
			delete[] stack[i];
		delete[] stack;
	}
public:
	int** stack;
	int num_layers;
	int current_layer;
};
param_stack p_stack;

// 해당 Node와 연결된 모든 node의 keep_mem을 true로 변경,
void garbage_collection_sub(int node) {
	if (mem_left[node] > 0) garbage_collection_sub(mem_left[node]);
	if (mem_right[node] > 0) garbage_collection_sub(mem_right[node]);
	keep_mem[node] = true;
}
// Garbage Collection,
void garbage_collection(int now_node) {
	cout << endl << ":::Full Memory Detected:::" << endl;
	cout << "--> Garbage Collection Activated" << endl;
	for (int i = 1; i < FL_size + 1; ++i)
		keep_mem[i] = false;
	// 현재 명령어 파싱 트리의 keep_mem을 true로 변경,
	garbage_collection_sub(processing_root);
	// Hash Table과 연결된 모든 node의 keep_mem을 true로 변경,
	for (int i = 1; i < Hash_size; ++i) {
		if (hash_occupied[i] == 1 && hash_ptr[i] > 0) garbage_collection_sub(hash_ptr[i]);
	}
	// Stack과 연결된 모든 node의 keep_mem을 true로 변경,
	for (int layer = 0; layer <= p_stack.current_layer; ++layer) {
		for (int i = 0; i < max_num_params; ++i) {
			if (p_stack.stack[layer][i] > 0) garbage_collection_sub(p_stack.stack[layer][i]);
		}
	}
	keep_mem[0] = keep_mem[FL_size] = true;
	// 현재 생성되는 Node 또한 true로 변경,
	keep_mem[now_node] = true;
	// Releasing,
	for (int i = 1; i < FL_size; ++i) {
		if (keep_mem[i] == false) dealloc(i);
	}
	// 현재 생성되는 Node를 false로 복구,
	// 출력 format에 맞추기 위함,
	keep_mem[now_node] = false;
	cout << "--> Garbage Collection Process Completed" << endl << endl;
}

int read() {
	int root = NIL;
	bool first = true;
	int temp;
	int hash_value = get_hash_value(get_next_token());
	if (hash_value == LEFT_PAREN_H) {
		while ((hash_value = get_hash_value(get_next_token())) != RIGHT_PAREN_H) {
			if (first == true) {
				root = temp = alloc();
				first = false;
			}
			else temp = mem_right[temp] = alloc();
			if (hash_value == LEFT_PAREN_H) {
				push_back_to_paren();
				mem_left[temp] = read();
			}
			else mem_left[temp] = -hash_value;
			mem_right[temp] = NIL;
		}
		return root;
	}
	else return -hash_value;
}

// User-defined symbol라면 return true,
// (define list '(1 2 3))이후 cmd가 list인 경우,
bool cmd_is_defined_symbol(int root) {
	return hash_str[-root] == str.substr(0, hash_str[-root].size());
}

// Free List Root, List Root, Current Tree, Hash Table, Input cmd를 모두 출력,
// Only for debugging,
// ss: Preprocessed Input cmd,
stringstream ss_sys;
void print_sys_sub_before_eval(int root, bool startList) {
	if (root == NIL) {
		ss_sys << "()";
		return;
	}
	else if (root < 0) ss_sys << hash_str[-root] << " ";
	else {
		if (startList == true) ss_sys << "( ";
		print_sys_sub_before_eval(mem_left[root], true);
		if (mem_right[root] != NIL) print_sys_sub_before_eval(mem_right[root], false);
		else ss_sys << ") ";
	}
}
void print_sys_before_eval(int root) {
	ss_sys.str("");
	cout << endl;
	cout << ":::System INFO(Before Eval):::" << endl;
	cout << "Free List Root: " << fl_head << endl;
	cout << "List Root: " << root << endl << endl;
	print_mem_all();
	cout << ":::Current Tree:::" << endl; print_list(root); cout << endl;
	print_hash_all();
	print_sys_sub_before_eval(root, true);
	cout << ":::Input Command:::" << endl;
	cout << ss_sys.str();
	cout << endl << endl;
}

// Eval후 Result를 출력,
// ss_res: Output,
stringstream ss_res;
void print_res_sub(int root, bool startList) {
	if (root == NIL) {
		ss_res << "() ";
		return;
	}
	else if (root < 0) {
		if (hash_ptr[-root] == 0) ss_res << hash_str[-root] << " ";
		// List의 출력,
		else if (cmd_is_defined_symbol(root)) print_res_sub(hash_ptr[-root], true);
		else print_res_sub(hash_ptr[-root], false);
	}
	else {
		if (startList == true) ss_res << "( ";
		print_res_sub(mem_left[root], true);
		if (mem_right[root] != NIL) print_res_sub(mem_right[root], false);
		else ss_res << ") ";
	}
}
void print_res(int root) {
	// define의 경우 출력하지 않음,
	if (root == FL_size + 1) return;
	ss_res.str("");
	cout << "] ";
	print_res_sub(root, true);
	cout << ss_res.str();
	cout << endl << endl;
}

string new_command;
void preprocessing_sub() {
	string token;
	while ((token = get_next_token()) != EOS) {
		if (token == "define") {
			new_command += "define ";
			token = get_next_token();
			if (token == "(") {
				token = get_next_token();
				new_command += token;
				new_command += " ( lambda ( ";
				preprocessing_sub();
				new_command += ") ";
			}
			else push_back_to_blank();
		}
		else if (token == "'") {
			new_command += "(quote ";
			int number_of_left_paren = 0;
			do {
				token = get_next_token();
				new_command += token;
				new_command += " ";
				if (token == "(") number_of_left_paren += 1;
				else if (token == ")") number_of_left_paren -= 1;
			} while (number_of_left_paren > 0);
			new_command += ") ";
		}
		else {
			new_command += token;
			new_command += " ";
		}
	}
}
void preprocessing() {
	new_command.clear();
	preprocessing_sub();
	str = new_command;
	str_idx = 0;
	processing_root = fl_head;
}

// 각각 root의 left, right child를 return,
// *** eval 함수에서만 사용,
int go_left(int root) {
	return mem_left[root];
}
int go_right(int root) {
	return mem_right[root];
}

// (음수) Hash 값에 대응되는 string을 double로 변환하여 return,
// 숫자이거나 숫자인 심볼의 경우에만 사용,
double get_val(int hash_value) {
	// 숫자인 경우,
	if (hash_ptr[-hash_value] == 0)	return atof(hash_str[-hash_value].c_str());
	// 숫자인 symbol의 경우,
	// (define x 3)
	return atof(hash_str[-hash_ptr[-hash_value]].c_str());
}
double fabs(double a, double b) {
	if (a >= b) return a - b;
	return b - a;
}
double max(double a, double b) {
	if (a >= b) return a;
	return b;
}
bool double_equal(double a, double b) {
	double diff = fabs(a, b);
	if (diff < 1e-10) return true;
	return diff <= 1e-8 * max(fabs(a), fabs(b));
}
// to_string function의 변형,
// num이 int인 경우 int 형태의 string을 return,
string to_string_(double num) {
	if (double_equal(num, (int)num)) return to_string((int)num);
	return to_string(num);
}

// (음수) Hash 값에 대응되는 string이 숫자라면 return true,
bool is_number(int hash_value) {
	// Memory의 ptr라면,
	if (hash_value >= 0) return false;
	// List인 경우,
	// (define x '(1 2 3))
	if (hash_ptr[-hash_value] > 0) return false;
	// Symbol인 경우,
	// (define x 3)
	if (hash_ptr[-hash_value] != 0) {
		string temp = hash_str[-hash_ptr[-hash_value]];
		int num_dots = 0, i = 0;
		if ((int)temp[i] == (int) '-') ++i;
		for (; i < temp.size(); ++i) {
			if ((int)temp[i] >= 48 && (int)temp[i] <= 57) continue;
			if ((int)temp[i] == (int) '.') {
				++num_dots;
				if (num_dots >= 2) return false;
				continue;
			}
			return false;
		}
		return true;
	}
	string temp = hash_str[-hash_value];
	int num_dots = 0, i = 0;
	if ((int)temp[i] == (int) '-') ++i;
	for (; i < temp.size(); ++i) {
		if ((int)temp[i] >= 48 && (int)temp[i] <= 57) continue;
		if ((int)temp[i] == (int) '.') {
			++num_dots;
			if (num_dots >= 2) return false;
			continue;
		}
		return false;
	}
	return true;
}
// 현재 cmd(str)가 function이라면 return true,
bool is_func() {
	string cmd = str;
	string cmp = "lambda";
	if (cmd.empty()) return false;
	for (int idx = 0; idx < cmd.size() - 5; ++idx) {
		if (cmd[idx] == 'l') {
			int flag = 1;
			for (int k = 1; k <= 5; ++k) {
				if (cmd[idx + k] != cmp[k]) {
					flag = 0;
					break;
				}
			}
			if (flag == 1) return true;
		}
	}
	return false;
}
// 현재 token이 userdef_func라면 return true,
bool is_userdef_func(const int token_index) {
	// Symbol인 경우,
	if (hash_ptr[-token_index] == 0) return false;
	// List인 경우,
	if (go_left(hash_ptr[-token_index]) != -hash_find("lambda")) return false;
	return true;
}

// stack에 저장할 주소를 반환,
// 숫자인 경우,
// symbol이 숫자인 경우,
// symbol이 list인 경우,
// stack에는 주소만 저장,
int get_val_all_types(int token_index) {
	// 숫자인 경우,
	if (token_index < 0 && hash_ptr[-token_index] == 0 && is_number(token_index)) return token_index;
	// symbol이 숫자인 경우,
	else return hash_ptr[-token_index];
}

int eval(int root) {
	// root < 0: Symbol인 경우,
	if (root < 0) {
		// 숫자인 경우,
		if (hash_ptr[-root] == 0 && is_number(root)) return root;
		// symbol이 숫자 또는 list인 경우,
		else return hash_ptr[-root];
	}
	// token_index < 0: Hash Table의 Symbol,
	// token_index > 0: Memory의 주소,
	// token_index = 0: NIL,
	int token_index = go_left(root);
	// 다른 Node인 경우,
	if (token_index > 0) return eval(token_index);
	int ret;
	if (token_index == -hash_find("+"))
		return -get_hash_value(to_string_(get_val(eval(go_left(go_right(root)))) + get_val(eval(go_left(go_right(go_right(root)))))));
	else if (token_index == -hash_find("-"))
		return -get_hash_value(to_string_(get_val(eval(go_left(go_right(root)))) - get_val(eval(go_left(go_right(go_right(root)))))));
	else if (token_index == -hash_find("*"))
		return -get_hash_value(to_string_(get_val(eval(go_left(go_right(root)))) * get_val(eval(go_left(go_right(go_right(root)))))));
	else if (token_index == -hash_find("number?")) {
		if (is_number(eval(go_left(go_right(root))))) return -hash_find("#t");
		return -hash_find("#f");
	}
	else if (token_index == -hash_find("symbol?")) {
		int result = eval(go_left(go_right(root)));
		if (result != 0 && is_number(result) == false) return -hash_find("#t");
		return -hash_find("#f");
	}
	else if (token_index == -hash_find("null?")) {
		if (go_right(root) == NIL || eval(go_left(go_right(root))) == NIL) return -hash_find("#t");
		return -hash_find("#f");
	}
	else if (token_index == -hash_find("cons")) {
		int new_memory = alloc();
		mem_left[new_memory] = eval(go_left(go_right(root)));
		mem_right[new_memory] = eval(go_left(go_right(go_right(root))));
		return new_memory;
	}
	else if (token_index == -hash_find("cond")) {
		while (go_right(go_right(root)) != NIL) {
			root = go_right(root);
			if (eval(go_left(go_left(root))) == -hash_find("#t")) return eval(go_right(go_left(root)));
		}
		if (go_left(go_left(go_right(root))) != -hash_find("else"))	print_error(3);
		return eval(go_left(go_right(go_left(go_right(root)))));
	}
	else if (token_index == -hash_find("car")) {
		// defined list의 경우,
		if (go_left(go_right(root)) < 0 && hash_ptr[-go_left(go_right(root))] > 0) ret = go_left(hash_ptr[-go_left(go_right(root))]);
		else ret = mem_left[eval(go_left(go_right(root)))];
		return ret;
	}
	else if (token_index == -hash_find("cdr")) {
		// defined list의 경우,
		if (go_left(go_right(root)) < 0 && hash_ptr[-go_left(go_right(root))] > 0) ret = go_right(hash_ptr[-go_left(go_right(root))]);
		else ret = mem_right[eval(go_left(go_right(root)))];
		return ret;
	}
	// define의 경우 FL_size + 1을 return,
	// FL_size + 1이 return되면 아무것도 출력되지 않음,
	else if (token_index == -hash_find("define")) {
		if (is_func()) hash_ptr[-go_left(go_right(root))] = go_left(go_right(go_right(root)));
		else {
			int temp;
			// (define x 3)의 경우,
			if ((temp = eval(go_left(go_right(go_right(root))))) < 0) hash_ptr[-go_left(go_right(root))] = temp;
			// (define x '(1 2 3 4))의 경우,
			else hash_ptr[-go_left(go_right(root))] = temp;
		}
		return FL_size + 1;
	}
	else if (token_index == -hash_find("quote")) return mem_left[go_right(root)];
	else if (is_userdef_func(token_index)) {
		// num_params: 매개변수의 개수,
		// param_new_values: 매개변수 값 배열,
		int num_params = 0;
		int param_new_values[max_num_params];
		int now = go_right(root);
		if (now != NIL) {
			while (true) {
				param_new_values[num_params] = eval(go_left(now));
				++num_params;
				if (go_right(now) != NIL) now = go_right(now);
				else break;
			}
		}
		now = go_left(go_right(hash_ptr[-token_index]));
		// param_old_values: 매개변수 값의 이전 값,
		int param_old_values[max_num_params];
		for (int i = 0; i < num_params; ++i) {
			param_old_values[i] = hash_ptr[-go_left(now)];
			hash_ptr[-go_left(now)] = param_new_values[i];
			now = go_right(now);
		}
		// p_stack에 param_old_values 저장,
		p_stack.push(param_old_values, num_params);
		// 함수 실행,
		int result = eval(go_left(go_right(go_right(hash_ptr[-token_index]))));
		// param_values_from_stack: stack.top()의 매개변수 값,
		int param_values_from_stack[max_num_params];
		p_stack.pop(param_values_from_stack, num_params);
		// 매개변수 값을 stack.top() 값으로 되돌림,
		now = go_left(go_right(hash_ptr[-token_index]));
		for (int i = 0; i < num_params; ++i) {
			// Undefined 변수의 경우,
			if (param_values_from_stack[i] == 0) hash_ptr[-go_left(now)] = 0;
			// Defined 변수의 경우,
			else hash_ptr[-go_left(now)] = param_values_from_stack[i];
			now = go_right(now);
		}
		return result;
	}
	else return get_val_all_types(token_index);
}

int main()
{
	hash_init();
	fl_init();
	while (true) {
		get_string();
		preprocessing();
		int root = read();
		int result = eval(root);
		print_res(result);
		cout << "Free List Root: " << fl_head << endl;
		cout << "List Root: " << root << endl << endl;
		print_mem_all();
		print_hash_all();
		/*
		//print_sys_before_eval(root);
		cout << ":::System INFO(After Eval):::" << endl;
		cout << "Free List Root: " << fl_head << endl << endl;
		print_mem_all();
		print_hash_all();
		*/
	}
	return 0;
}
