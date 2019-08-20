#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <stack>
using namespace std;

enum type_code {
	tc_cpp, tc_java, tc_none
};

enum type_word {
	tw_sep, tw_key, tw_vis, tw_spec,
	tw_type_simple, tw_type_add, tw_type,
	tw_sub, tw_skip, tw_fun, tw_none
};

string get_string(type_word type)
{
	switch (type) {
		case tw_sep: return "tw_sep";
		case tw_key: return "tw_key";
		case tw_vis: return "tw_vis";
		case tw_spec: return "tw_spec";
		case tw_type_simple: return "tw_type_simple";
		case tw_type_add: return "tw_type_add";
		case tw_type: return "tw_type";
		case tw_sub: return "tw_sub";
		case tw_skip: return "tw_skip";
		case tw_fun: return "tw_fun";
		case tw_none: return "tw_none";
		default : return "default";
	};
};

struct t_rule_order {
	type_word type;
	bool once;
	string name;
	vector<t_rule_order> sub;
};

enum type_meta {
	tm_class, tm_fun, tm_val, tm_none
};

class t_metadata_base;
// typedef vector<t_metadata_base*> tv_meta;
typedef vector<t_metadata_base> tv_meta;
typedef vector<tv_meta> tvv_meta;
typedef map<type_code, tv_meta> tmv_meta;

class t_metadata_base {
public:
	type_meta tm = tm_none;
	tv_meta sub;
	string name;
	string type;
	// vector<string> spec;
};

class t_metadata_class : public t_metadata_base {
public:
	t_metadata_class()
	{
		tm = tm_class;
	}
};

class t_metadata_fun : public t_metadata_base {
public:
	t_metadata_fun()
	{
		tm = tm_fun;
	}
	string type;
	vector<string> spec;
};

class t_metadata_val : public t_metadata_base {
public:
	t_metadata_val()
	{
		cout << "fff!\n";
		tm = tm_val;
	}
	string type;
	vector<string> spec;
};

class t_metadata_factory {
public:
	static
	t_metadata_base* get(type_meta type)
	{
		switch (type) {
			case tm_val: return new t_metadata_val;
			case tm_fun: return new t_metadata_fun;
			case tm_class: return new t_metadata_class;
			default: return nullptr;
		}
	}

	static
	void add_type(t_metadata_base* ptr, string str)
	{
		switch (ptr->tm) {
			case tm_val:
				if (t_metadata_val *ptr_val = reinterpret_cast<t_metadata_val*>(ptr))
					ptr_val->type = str;
				break;
			case tm_fun:
				if (t_metadata_fun *ptr_fun = reinterpret_cast<t_metadata_fun*>(ptr))
					ptr_fun->type = str;
				break;
			default:
				cout << "\n error: add_type: wrong type\n\n";
		}
	}

	static
	void add_spec(t_metadata_base* ptr, string str)
	{
		switch (ptr->tm) {
			case tm_val:
				if (t_metadata_val *ptr_val = reinterpret_cast<t_metadata_val*>(ptr))
					ptr_val->spec.push_back(str);
				break;
			case tm_fun:
				if (t_metadata_fun *ptr_fun = reinterpret_cast<t_metadata_fun*>(ptr))
					ptr_fun->spec.push_back(str);
				break;
			default:
				cout << "\n error: add_spec: wrong type\n\n";
		}
	}
};

class t_rule_base1 {
public:
	set<string> sep;
	set<string> keyword;
	set<string> visible;
	set<string> spec;
	set<string> type_simple;
	set<string> type_diff;
	set<string> type;
	set<string> type_tmp;
public:
	bool is_sep(string str) { return sep.count(str); }
	bool is_keyword(string str) { return keyword.count(str); }
	bool is_visible(string str) { return visible.count(str); }
	bool is_spec(string str) { return spec.count(str); }
	bool is_type_simple(string str) { return type_simple.count(str); }
	bool is_type_diff(string str) { return type_diff.count(str); }
	bool is_type_add(string str) { return type.count(str); }
	bool is_type_tmp(string str) { return type_tmp.count(str); }
	bool is_type(string str)
	{
		return is_type_add(str) || is_type_diff(str)
			|| is_type_simple(str) || is_type_tmp(str)
			|| is_std_ptr(str) || is_type_ptr(str) || is_type_ref(str);
	}
	bool is_special(string str)
	{ return is_sep(str) || is_keyword(str) || is_visible(str)
		|| is_spec(str) || is_type(str); }

	bool is_no_special(string str) {
		return !is_special(str);
	}

	bool cmp_types(string name, string str)
	{
		return  name == str ||
				name == str + "&" ||
				name == str + "&&";
	}
	
	void type_add(string str) { type.insert(str); }
	void type_add(set<string> strs)
	{
		for (string str : strs)
			type.insert(str);
	}
	void type_tmp_add(string str)
	{
		type_tmp.insert(str);
		// type_tmp.insert(str + "&");
		// type_tmp.insert(str + "&&");
	}
	void type_tmp_merge() {
		type_add(type_tmp);
		type_tmp.clear();
	}

	bool is_std_ptr(string str)
	{
		return str.find("std::unique_ptr") == 0;
	}

	bool is_type_ref(string str)
	{
		return str.find("&") != string::npos;
	}

	bool is_type_ptr(string str)
	{
		return str.find("*") != string::npos;
	}

	string get_type_ref(string str)
	{
		string t;
		int index = str.find("&");
		if (index != string::npos)
			t = string(str, 0, index);
		return t;
	}

	string get_type_ptr(string str)
	{
		string t;
		int index = str.find("*");
		if (index != string::npos)
			t = string(str, 0, index);
		return t;
	}

	string get_type_std_ptr(string str)
	{
		string t;
		int index = str.find("<");
		index = index == string::npos ? str.size() : index;
		for (; index < str.size(); index++) {
			if (str[index] == '>')
				break;
			t += str[index];
		}
		return t;
	}
};
