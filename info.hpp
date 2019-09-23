#pragma once

#include <functional>
#include <utility>
#include "metadata.hpp"

using namespace std;

enum t_type_check_fun {
	tcf_type, tcf_access, tcf_name, tcf_spec, tcf_none
};

class t_info {
	set<string> keyword;

	set<string> type_pod;
	set<string> type_diff;
	set<string> type_add;
	map<string, string> type_name;
	set<string> type_add_tmp;
	map<string, string> type_header;

	map<string, t_type_access> access;
	set<string> sep;
	set<string> spec;

	void init_info()
	{
		keyword = {
			"class"
		};

		type_pod = {
			"bool",  "byte",
			"char",  "short",
			"int",   "long",
			"float", "double",
			"void"
		};

		type_diff = {
			"std::string",
			"std::unique_ptr",
			"std::optional",
			"std::function"
		};

		access["public"] = ta_public;
		access["private"] = ta_private;
		access["protected"] = ta_protected;

		sep = {
			";", ",", "=", "(", ")", "{", "}", ":"
		};

		spec = {
			"static", "const", "default", "final", "virtual", "using", "=", "0"
		};
	}
public:
	t_info()
	{
		init_info();
	}

	bool is_keyword(string str) { return keyword.count(str); }
	bool is_type_pod(string str) { return type_pod.count(str); }
	bool is_type_diff(string str) { return type_diff.count(str); }
	bool is_type_add(string str) { return type_add.count(str); }
	bool is_type_name(string str) { return type_name.count(str); }
	bool is_type_add_tmp(string str) { return type_add_tmp.count(str); }
	bool is_access(string str) { return access.count(str); }
	bool is_sep(string str) { return sep.count(str); }
	bool is_spec(string str) { return spec.count(str); }

	bool is_type(string str)
	{
		string type = get_type_free(str);
		return is_type_add(type) || is_type_diff(get_type_std(type))
			|| is_type_pod(type) || is_type_name(type)
			|| is_type_add_tmp(type);
	}

	bool is_special(string str)
	{
		return is_keyword(str) || is_type(str) || is_access(str)
			|| is_sep(str) || is_spec(str);
	}

	bool is_no_special(string str) { return !is_special(str); }

	// adding info
	void add_type_name(string name, string type) { type_name[name] = type; }
	void add_type()
	{
		for (string str : type_add_tmp)
			type_add.insert(str);
		type_add_tmp.clear();
	}
	void add_type_tmp(string str) { type_add_tmp.insert(str); }
	void clear_type_tmp() { type_add_tmp.clear(); }

	// get
	string get_type_name(string name) { return type_name[name]; }

	// private
	bool is_sub(string str, string sub)
	{
		return str.find(sub, str.size() - sub.size()) != string::npos;
	}

	// checking type
	bool is_ptr(string str) { return is_sub(str, "*"); }
	bool is_ref(string str) { return !is_lval(str) && is_sub(str, "&"); }
	bool is_lval(string str) { return is_sub(str, "&&"); }
	bool is_temp(string str)
	{
		int idx_b = str.find("<");
		int idx_e = str.find(">");
		return  idx_b != string::npos 
				&& idx_e != string::npos
				&& idx_b < idx_e;
	}

	bool is_unique_ptr(string str) { return str.find("std::unique_ptr") == 0; }
	bool is_optional(string str) { return str.find("std::optional") == 0; }
	bool is_function(string str) { return str.find("std::function") == 0; }

	// private
	string get_type(string str, string sub)
	{
		int index = str.find(sub, str.size() - sub.size());
		if (index != string::npos)
			return string(str, 0, index);
		return str;
	}

	string get_type_std(string str)
	{
		int index = str.find("<");
		if (index != string::npos)
			return string(str, 0, index);
		return str;
	}
	string get_type_free(string str)
	{
		str = get_type(str, "*");
		str = get_type(str, "&&");
		str = get_type(str, "&");
		return str;
	}

	string get_type_temp(string str)
	{
		string t;
		int index = str.find("<") + 1;
		for (; index < str.size(); index++) {
			if (str[index] == '>')
				break;
			t += str[index];
		}
		return t;
	}

	bool check(t_type_check_fun type, string str)
	{
		switch (type) {
			case tcf_type: return is_type(str);
			case tcf_access: return is_access(str);
			case tcf_name: return is_no_special(str);
			case tcf_spec: return is_spec(str);
			default:
				cout << "t_info: check: ERROR\n";
		};
	}

	t_type_access get_type_access(string str) { return access[str]; }
	t_type_variable get_type_variable(string str)
	{
		if (is_lval(str))
			return tv_move;
		else if (is_ref(str))
			return tv_reference;
		else if (is_ptr(str))
			return tv_pointer;
		else
			return tv_variable;
	}
	bool is_static(string str) { return "static" == str; }
	bool is_const(string str) { return "const" == str; }
	bool is_virtual(string str)
	{
		return "virtual" == str || "0" == str;
	}
};
