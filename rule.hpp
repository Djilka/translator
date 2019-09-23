#include "info.hpp"

class t_rule;
typedef vector<t_rule> tv_rule;
typedef vector<tv_rule> tvv_rule;

enum t_type_rule {
	tr_key, tr_sub, tr_fun, tr_none
};

class t_rule {
	t_type_rule type = tr_none;
	bool always;
	
	t_type_object type_object = to_none;
	tv_rule sub;

	string key;

	t_type_check_fun type_check_fun;
public:
	t_rule()
	{
		always = false;
	}

	t_rule(string s, bool a)
	{
		always = a;
		type = tr_key;
		key = s;
	}

	t_rule(t_type_check_fun t, bool a)
	{
		always = a;
		type = tr_fun;
		type_check_fun = t;
	}

	t_rule(tv_rule r, bool a, t_type_object t)
	{
		always = a;
		type = tr_sub;
		sub = r;
		type_object = t;
	}

	bool is_always() { return always; }
	bool is_sub() { return type == tr_sub; }
	bool is_object() { return type_object == to_none; }
	bool is_fun() { return type == tr_fun; }

	tv_rule get_sub() { return sub; }
	t_type_object get_type_object() { return type_object; }
	t_type_check_fun get_type_check_fun() { return type_check_fun; }

	bool check(string str, t_info info)
	{
		switch (type) {
			case tr_fun: return info.check(type_check_fun, str) ? true : !always;
			case tr_key: return key == str ? true : !always;
		};
		return false;
	}
};
