#pragma once

#include "header.hpp"

class t_rule : public t_rule_base1 {
	typedef vector<t_rule_order> tv_rule_order;
	typedef vector<tv_rule_order> tvv_rule_order;
protected:
	tv_meta meta;
	tvv_rule_order rule_class;
	tvv_rule_order rule_fun;
	tvv_rule_order rule_val;
	type_code tc = tc_none;
	stack<t_metadata_base> st;
public:
	// void destroy_meta(t_metadata_base* m)
	// {
	// 	for (t_metadata_base* ptr : m->sub) {
	// 		destroy_meta(mb.sub);
	// 		delete ptr;
	// 	}
	// 	delete m;
	// }

	// void destroy_meta(tv_meta m)
	// {
	// 	for (t_metadata_base* ptr : m)
	// 		destroy_meta(ptr);
	// }

	// bool create_meta(type_meta type)
	// {
	// 	t_metadata_base *ptr = t_metadata_factory::get(type);
	// 	if (ptr == nullptr)
	// 		return false;
		
	// 	st.push(ptr);
	// 	return true;
	// }

	tvv_rule_order get_rules(type_meta type)
	{
		switch (type) {
			case tm_class: return rule_class;
			case tm_fun: return rule_fun;
			case tm_val: return rule_val;
			default: return {};
		}
	}

	tv_meta get_meta()
	{
		if (st.size())
			cout << "\nstack not free\n\n";
		return meta;
	}

	
	bool get_st_size(string str)
	{
		cout << str + " st = " << st.size() << "\n";
		return false;
	}
public:
	virtual vector<string> split(string str) { return {}; }
	virtual bool try_val(vector<string> v, int &i) { return false; }
	virtual bool try_class(vector<string> v, int &i) { return false; }
	virtual bool try_fun(vector<string> v, int &i) { return false; }
	virtual bool try_typedef(vector<string> v, int &i) { return false; }
};

class t_rule_cpp : public t_rule {
public:
	t_rule_cpp()
	{
		// sep = {';', ',', '=', '(', ')', '{', '}', ':'};
		sep = {";", ",", "=", "(", ")", "{", "}", ":"};
		keyword = {"class"};
		visible = {"public", "private", "protected"};
		spec = {"static", "const", "default", "final", "virtual", "using"};
		type_simple = {
			"bool",  "byte",
			"char",  "short",
			"int",   "long",
			"float", "double",
			"void"
		};
		type_diff = {
			"std::string", 
			"std::unique_ptr<Style>",
			"std::optional<StyleOptions>&&",
			"std::function<void()>"
		};

		tc = tc_cpp;
	}

	vector<string> split(string str)
	{
		vector<string> res;
		int ps = 0, pc = 0;
		int count = 0;
		while (pc < str.size()) {
			if (str[pc] == '<' || str[pc] == '>') {
				count += str[pc] == '<' ? 1 : -1;
			} else if (!count) {
				if (str[pc] == ':' && pc + 1 < str.size() && str[pc+1] == ':') {
					pc += 2;
					continue;
				}
				string s(str, pc, 1);
				if (!isalnum(str[pc]) && is_sep(s)) {
					if (ps != pc)
						res.push_back(string(str, ps, pc - ps));
					res.push_back(s);
					ps = pc + 1;
				}
			}
			pc++;
		}
		if (ps != pc)
			res.push_back(string(str, ps, pc - ps + 1));
		return res;
	}

// #define check_rule(is_fun) \
// 	if (i >= v.size() || !is_fun(v[i])) { \
// 		ret(ptr, i, start_i); \
// 		return false; \
// 	}

// #define check_exp(exp) \
// 	if (i >= v.size() || v[i] != exp) { \
// 		ret(ptr, i, start_i); \
// 		return false; \
// 	}

// #define check_spec() \
// 	while (i < v.size() && is_spec(v[i])) \
// 		i++;

#define check_type_st(is_fun) \
	if (i >= v.size() || !is_fun(v[i])) { \
		i = start_i; \
		if (mb.tm != tm_val) \
			st.pop(); \
		return false; \
	}

#define check_rule_st(is_fun) \
	if (i >= v.size() || !is_fun(v[i])) { \
		i = start_i; \
		if (mb.tm != tm_val) \
			st.pop(); \
		return false; \
	}

#define check_exp_st(exp) \
	if (i >= v.size() || v[i] != exp) { \
		i = start_i; \
		if (mb.tm != tm_val) \
			st.pop(); \
		return false; \
	}

#define check_rule(is_fun) \
	if (i >= v.size() || !is_fun(v[i])) { \
		i = start_i; \
		return false; \
	}

#define check_exp(exp) \
	if (i >= v.size() || v[i] != exp) { \
		i = start_i; \
		return false; \
	}

#define check_spec() \
	while (i < v.size() && is_spec(v[i])) \
		i++;

	bool try_val(vector<string> v, int &i)
	{
		// spec type name;
		t_metadata_base mb = {.tm = tm_val};
		int start_i = i;
		
		// spec
		check_spec();
		// cout << "try_val i = " << i << "\n";

		// type
		check_rule(is_type);
		mb.type = v[i++];

		// name
		check_rule(is_no_special);
		mb.name = v[i++];

		// ;
		check_exp(";");
		i++;

		// cout << "try_val i = " << i << "\n";

		if (st.size())
			st.top().sub.push_back(mb);
		else
			meta.push_back(mb);
		return true;
	}

	bool try_val_fun(vector<string> v, int &i)
	{
		// spec type <name>
		t_metadata_base mb = {.tm = tm_val};
		int start_i = i;

		// cout << "0 try_val_fun i = " << i << "\n";
		
		// spec
		check_spec();
		// cout << "1 try_val_fun i = " << i << "\n";

		// type
		check_rule(is_type);
		mb.type = v[i++];

		// cout << "2 try_val_fun i = " << i << "\n";

		// name
		if (i < v.size() && v[i] != "," && v[i] != ")") {
			// name
			check_rule(is_no_special);
			mb.name = v[i++];
		}

		if (st.size())
			st.top().sub.push_back(mb);

		// cout << "3 try_val_fun i = " << i << "\n";
		
		return true;
	}

	// void ret(t_metadata_base *ptr, int &i, int start_i)
	// {
	// 	if (ptr.tm != tm_val) {
	// 		destroy_meta(ptr);
	// 		if (st.size())
	// 			st.pop();
	// 	}
	// 	delete ptr;
	// 	i = start_i;
	// }

	bool try_class_inher(vector<string> v, int &i)
	{
		// : public type
		int start_i = i;

		// cout << "0 try_class_inher i = " << i << "\n";

		// :
		check_exp(":");
		i++;

		// :
		check_exp("public");
		i++;
		
		// type
		check_rule(is_type);
		i++;

		return true;
	}

	bool find_scope(vector<string> v, int &i, string scope)
	{
		int start_i = i;

		// cout << "0 find_scope i = " << i << "\n";
		
		while (i < v.size() && v[i] != "}") {
			if (try_scope(v, i, "public"))
				return true;
			i++;
		}

		// cout << "0 find_scope i = " << i << "\n";

		if (i >= v.size() || v[i] != "}")
			i = start_i;
		return false;
	}

	bool try_scope(vector<string> v, int &i, string str = "")
	{
		// public:
		int start_i = i;

		// visible
		if (str.size()) {
			check_exp(str);
		} else {
			check_rule(is_visible);
		}
		i++;

		// :
		check_exp(":");
		i++;

		return true;
	}

	bool try_class(vector<string> v, int &i)
	{
		cout << "try_class begin stack = " << st.size() << "\n";
		// class name <:public type> { };
		int start_i = i;
		t_metadata_base mb = {.tm = tm_class};
		st.push(mb);

		// cout << "0 try_class i = " << i << "\n";

		// class
		check_exp_st("class");
		i++;

		// cout << "1 try_class i = " << i << "\n";

		// name
		check_rule_st(is_no_special);
		st.top().name = v[i++];
		type_tmp_add(st.top().name);

		// cout << "2 try_class i = " << i << "\n";

		// <:public type>
		try_class_inher(v, i);

		// cout << "3 try_class i = " << i << "\n";

		// {
		check_exp_st("{");
		i++;

		// cout << "4 try_class i = " << i << "\n";
		cout << "try_class before stack = " << st.size() << "\n";

		while (i < v.size() && find_scope(v, i, "public")) {
			while (i < v.size() && v[i] != "}" && !try_scope(v, i)) {
				if (try_val(v, i)) {
					cout << "find val\n";
				} else if (try_fun(v, i)) {
					cout << "find fun\n";

				} else if (try_class(v, i)) {
					cout << "find class\n";
				} else if (try_typedef(v, i)) {
					cout << "find using\n";
				} else if (try_constr(v, i)) {
					cout << "find constructor\n";
				}else{
					cout << "didn't find type\n";
					i++;
				}
			}
		}

		// cout << "5 try_class i = " << i << "\n";

		// }
		check_exp_st("}");
		i++;

		// cout << "6 try_class i = " << i << "\n";

		// ;
		check_exp_st(";");
		i++;

		// cout << "7 try_class i = " << i << "\n";

		cout << "try_class stack = " << st.size() << "\n";
		mb = st.top();
		st.pop();
		if (st.size())
			st.top().sub.push_back(mb);
		else
			meta.push_back(mb);
		type_tmp_merge();
		return true;
	}

	bool try_constr(vector<string> v, int &i)
	{
		// type ( type name, type name ) spec sep 0;
		int start_i = i;
		t_metadata_base mb = {.tm = tm_fun};
		st.push(mb);

		// cout << "0 try_fun i = " << i << "\n";

		// type
		check_rule_st(is_type);
		st.top().type = v[i++];

		// cout << "1 try_fun i = " << i << "\n";

		// (
		check_exp_st("(");
		i++;

		// cout << "2 try_fun i = " << i << "\n";

		// args -> mb.sub;
		while (i < v.size() && try_val_fun(v, i)) {
			if (i >= v.size() || v[i] != ",")
				break;
			i++;
		}

		// cout << "3 try_fun i = " << i << "\n";

		// )
		check_exp_st(")");
		i++;

		// cout << "4 try_fun i = " << i << "\n";

		// spec
		set<string> sp = {"=", "default", "final", "0"};
		while (i < v.size() && sp.count(v[i]))
			i++;

		// cout << "5 try_fun i = " << i << "\n";

		// ;
		check_exp_st(";");
		i++;

		// cout << "6 try_fun i = " << i << "\n";

		mb = st.top();
		st.pop();
		if (st.size())
			st.top().sub.push_back(mb);
		else
			meta.push_back(mb);

		return true;
	}
	
	bool try_fun(vector<string> v, int &i)
	{
		cout << "try_fun begin st = " << st.size() << "\n";
		// spec type name ( type name, type name ) spec sep 0;
		int start_i = i;
		t_metadata_base mb = {.tm = tm_fun};
		st.push(mb);

		// cout << "0 try_fun i = " << i << "\n";

		// spec
		check_spec();

		// cout << "1 try_fun i = " << i << "\n";

		// type
		check_rule_st(is_type);
		st.top().type = v[i++];

		// cout << "2 try_fun i = " << i << "\n";

		// name
		check_rule_st(is_no_special);
		st.top().name = v[i++];

		// cout << "3 try_fun i = " << i << "\n";

		// (
		check_exp_st("(");
		i++;

		// cout << "4 try_fun i = " << i << "\n";

		// args -> mb.sub;
		while (i < v.size() && try_val_fun(v, i)) {
			// cout << "4.1 try_fun i = " << i << "; v[i] = " << v[i] << "\n";
			if (i >= v.size() || v[i] != ",")
				break;
			// cout << "4.2 try_fun i = " << i << "; v[i] = " << v[i] << "\n";
			i++;
		}

		// cout << "5 try_fun i = " << i << "\n";

		// )
		check_exp_st(")");
		i++;

		// cout << "6 try_fun i = " << i << "\n";

		// spec
		set<string> sp = {"=", "default", "final", "0", "const"};
		while (i < v.size() && sp.count(v[i]))
			i++;

		// cout << "7 try_fun i = " << i << "\n";

		// ;
		check_exp_st(";");
		i++;

		// cout << "8 try_fun i = " << i << "\n";

		mb = st.top();
		st.pop();
		if (st.size()) {
			cout << "!!!!!!!!!! add fun to stack.sub\n";
			st.top().sub.push_back(mb);
		} else
			meta.push_back(mb);

		cout << "try_fun end st = " << st.size() << "\n";

		return true;
	}
	
	bool try_typedef(vector<string> v, int &i)
	{
		// using none = type;
		int start_i = i;
		
		// cout << "1 try_typedef i = " << i << "\n";

		// using
		check_exp("using");
		i++;

		// cout << "2 try_typedef i = " << i << "\n";

		// name
		check_rule(is_no_special);
		string name = v[i++];

		// cout << "3 try_typedef i = " << i << "\n";

		// =
		check_exp("=");
		i++;

		// cout << "4 try_typedef i = " << i << "\n";

		// type
		check_rule(is_type);
		i++;

		// ;
		check_exp(";");
		i++;

		// cout << "5 try_typedef i = " << i << "\n";

		type_tmp_add(name);
		return true;
	}

};

class t_rule_java : public t_rule {
public:
	t_rule_java()
	{
		tc = tc_java;
	}

	vector<string> split(string str) { return {}; }
	bool try_val(vector<string> v, int &i) { return false; }
	bool try_class(vector<string> v, int &i) { return false; }
	bool try_fun(vector<string> v, int &i) { return false; }
	bool try_typedef(vector<string> v, int &i) { return false; }
};

class t_rule_factory {
public:
	static
	t_rule* create(type_code type)
	{
		switch (type) {
			case tc_cpp: return new t_rule_cpp;
			case tc_java: return new t_rule_java;
			default: return nullptr;
		}
	}
};
