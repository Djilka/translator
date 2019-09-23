#pragma once
#include "rule.hpp"

class t_language {
protected:
	t_type_access type_access = ta_private;
	vector<string> *v_ptr;
	int i = 0;

	stack<t_meta*> st;
	tv_meta meta;
	t_meta *curr;

	t_info info;
	map<t_type_object, t_rule> mrule;

	bool try_rules_sub(t_rule rule)
	{
		t_type_object type = rule.get_type_object();
		if (!init_data(type))
			return false;

		int i_start = i;
		if (rule.is_always()) {
			return clear_data(type, try_rules(rule.get_sub()), i != i_start);
		} else {
			int j = i - 1;
			while (i < v_ptr->size() && try_rules(rule.get_sub()) && i != j) {
				clear_data(type, true, true);
				if (!init_data(type))
					return false;
				j = i;
			}
			i--;
			return clear_data(type, true, false);
		}
	}

	bool try_rules(tv_rule rules)
	{
		int start_i = i;
		for (t_rule rule : rules) {
			if (i >= v_ptr->size() || !((rule.is_sub() && try_rules_sub(rule)) ||
				(!rule.is_sub() && rule.check((*v_ptr)[i], info)) )) {
				i = start_i;
				return false;
			}

			if (!rule.is_sub() && rule.is_fun())
				update_data(rule, (*v_ptr)[i]);
			i++;
		}

		return true;
	}

	bool init_data(t_type_object type)
	{
		if (type == to_none)
			return true;
		curr = t_manager_meta::create(type);
		if (!curr)
			return false;

		if (type == to_class)
			type_access = ta_private;
		curr->type_access = type_access;
		st.push(curr);
		return true;
	}

	template <class T>
	bool add_type(string str)
	{
		if (T *ptr = dynamic_cast<T*>(st.top())) {
			ptr->type = {
				.type = str, 
				.type_variable = info.get_type_variable(str),
				.is_temp = info.is_temp(str)
			};
			return true;
		}
		return false;
	}

	void update_data(t_rule rule, string str)
	{
		switch (rule.get_type_check_fun()) {
			case tcf_name: {
				st.top()->name = str;
				if (st.top()->type_object == to_typedef ||
					st.top()->type_object == to_class)
					info.add_type_tmp(str);
				break;
			}
			case tcf_type: {
				if (add_type<t_meta_variable>(str));
				else if (add_type<t_meta_function>(str));
				else if (add_type<t_meta_typedef>(str));
				break;
			}
			case tcf_access: {
				type_access = info.get_type_access(str);
				break;
			}
			case tcf_spec: {
				if (st.top()->type_object == to_variable) {
					t_meta_variable *ptr = reinterpret_cast<t_meta_variable*>(st.top());
					ptr->is_static = info.is_static(str);
					ptr->is_const = info.is_const(str);
				} else if (st.top()->type_object == to_function) {
					t_meta_function *ptr = reinterpret_cast<t_meta_function*>(st.top());
					ptr->is_static = info.is_static(str);
					if (info.is_virtual(str))
						ptr->type_specifier = ts_virtual;
				}
				break;
			}
			default:
				cout << "t_info: check: ERROR\n";
		};
	}

	bool clear_data(t_type_object type, bool res, bool change)
	{
		if (type == to_none) {
			return res;
		}
		if (res) {
			t_meta *curr = st.top();
			if (st.size() == 1) {
				meta.push_back(curr);
				info.add_type();
			} else {
				curr->type_access = type_access;
				if (!curr->name.size() && curr->type_object == to_function) {
					t_meta_function *ptr = reinterpret_cast<t_meta_function*>(curr);
					ptr->type_function = tf_constructor;
					if (ts_virtual == ptr->type_specifier) {
						st.pop();
						if (st.top()->type_object == to_class) {
							t_meta_class *ptr_class = reinterpret_cast<t_meta_class*>(st.top());
							ptr_class->type_specifier = ptr->type_specifier;
						}
						st.push(curr);
					}
				}
				if (curr->type_object == to_typedef) {
					t_meta_typedef *ptr = reinterpret_cast<t_meta_typedef*>(curr);
					info.add_type_name(ptr->name, ptr->type.type);
					t_manager_meta::destroy(ptr);
				} else {
					st.pop();

					if (change) {
						st.top()->add(curr);
					}
					st.push(curr);
				}
			}
		} else {
			info.clear_type_tmp();
			t_manager_meta::destroy(st.top());
		}
		st.pop();
		return res;
	}

	int level = 0;
public:
	// private
	bool run_rule(t_type_object type)
	{
		if (!init_data(type))
			return false;

		int i_start = i;
		return clear_data(type, try_rules(mrule[type].get_sub()), i_start != i);
	}

	bool run(vector<string> v)
	{
		i = 0;
		v_ptr = &v;
		while (i < v_ptr->size()) {
			int j = i;
			for (int type = 0; type < to_none; type++) {
				if (run_rule((t_type_object)type))
					break;
			}
			if (j == i) {
				return false;
			}
		}
		return true;
	}

	tv_meta get_meta() { return meta; }
	t_info get_info() { return info; }
};


class t_language_cpp : public t_language {
	t_rule create_rule_typedef()
	{
		bool always = true;

		// using name = type ;
		tv_rule sub = {
			t_rule("using", always),
			t_rule(tcf_name, always),
			t_rule("=", always),
			t_rule(tcf_type, always),
			t_rule(";", always)
		};

		// < using name = type ; >
		t_rule rule(sub, !always, to_typedef);
		return rule;
	}

	t_rule create_rule_variable()
	{
		bool always = true;

		// , name
		tv_rule sub_1 = {
			t_rule(",", always),
			t_rule(tcf_name, always)
		};

		// spec
		tv_rule sub_2 = {
			t_rule(tcf_spec, always)
		};

		// <spec> type name <, name> ;
		tv_rule sub_3 = {
			t_rule(sub_2, !always, to_none),
			t_rule(tcf_type, always),
			t_rule(tcf_name, always),
			t_rule(sub_1, !always, to_variable),
			t_rule(";", always)
		};

		// < <spec> type name <, name> ; >
		t_rule rule(sub_3, !always, to_variable);
		return rule;
	}

	t_rule create_rule_function()
	{
		bool always = true;

		// < spec >
		tv_rule sub_4 = {
			t_rule(tcf_spec, always)
		};

		// < name >
		tv_rule sub_1 = {
			t_rule(tcf_name, always)
		};

		// < , type < name > >
		tv_rule sub_2 = {
			t_rule(",", always),
			t_rule(sub_4, !always, to_none),
			t_rule(tcf_type, always),
			t_rule(sub_1, !always, to_none)
		};

		// type < name > < , type < name > >
		tv_rule sub_3 = {
			t_rule(sub_4, !always, to_none),
			t_rule(tcf_type, always),
			t_rule(sub_1, !always, to_none),
			t_rule(sub_2, !always, to_variable)
		};

		// <spec> type name (< type < name > < , type < name > > > ) ;
		tv_rule sub_5 = {
			t_rule(sub_4, !always, to_none),
			t_rule(tcf_type, always),
			t_rule(tcf_name, always),
			t_rule("(", always),
			t_rule(sub_3, !always, to_variable),
			t_rule(")", always),
			t_rule(sub_4, !always, to_none),
			t_rule(";", always)
		};

		// FIX add new spec
		// < <spec> type name (< type < name > < , type < name > > > ) <spec> ; >
		t_rule rule(sub_5, !always, to_function);
		return rule;
	}

	t_rule create_rule_constructor()
	{
		bool always = true;

		// < spec >
		tv_rule sub_4 = {
			t_rule(tcf_spec, always)
		};

		// < name >
		tv_rule sub_1 = {
			t_rule(tcf_name, always)
		};

		// < , type < name > >
		tv_rule sub_2 = {
			t_rule(",", always),
			t_rule(sub_4, !always, to_none),
			t_rule(tcf_type, always),
			t_rule(sub_1, !always, to_none)
		};

		// type < name > < , type < name > >
		tv_rule sub_3 = {
			t_rule(sub_4, !always, to_none),
			t_rule(tcf_type, always),
			t_rule(sub_1, !always, to_none),
			t_rule(sub_2, !always, to_variable)
		};

		// <spec> type ( < type < name > < , type < name > > > ) <spec> ;
		tv_rule sub_5 = {
			t_rule(sub_4, !always, to_none),
			t_rule(tcf_type, always),
			t_rule("(", always),
			t_rule(sub_3, !always, to_variable),
			t_rule(")", always),
			t_rule(sub_4, !always, to_none),
			t_rule(";", always)
		};

		// FIX add new spec
		// < <spec> type ( < type < name > < , type < name > > > ) <spec> ; >
		t_rule rule(sub_5, !always, to_function);
		return rule;
	}

	t_rule create_rule_class()
	{
		bool always = true;

		// < access_type >
		tv_rule sub_1 = {
			t_rule(tcf_access, always)
		};

		// : < access_type > type
		tv_rule sub_2 = {
			t_rule(":", always),
			t_rule(tcf_access, !always),
			t_rule(tcf_type, always)
		};

		// < access_type : >
		tv_rule sub_3 = {
			t_rule(tcf_access, always),
			t_rule(":", always)
		};

		// <
		// 	<access_type :>
		// 	variables
		// 	function
		// 	typedef
		// 	class
		// 	constructor
		// >
		tv_rule sub_4 = {
			t_rule(sub_3, !always, to_none),
			t_rule(mrule[to_variable].get_sub(), !always, to_variable),
			t_rule(mrule[to_function].get_sub(), !always, to_function),
			t_rule(mrule[to_typedef].get_sub(), !always, to_typedef),
			// FIX add sub rule for class
			// t_rule(rule_variable.get_sub(), !always, to_class),
			t_rule(create_rule_constructor().get_sub(), !always, to_function)
		};

		// class name <: <access_type> type> {
		// <
		// 	<access_type :>
		// 	variables
		// 	function
		// 	typedef
		// 	class
		// 	constructor
		// >
		// };
		tv_rule sub_5 = {
			t_rule("class", always),
			t_rule(tcf_name, always),
			t_rule(sub_2, !always, to_none),
			t_rule("{", always),
			t_rule(sub_4, !always, to_none),
			t_rule("}", always),
			t_rule(";", always)
		};

		t_rule rule(sub_5, !always, to_class);
		return rule;
	}
public:
	t_language_cpp()
	{
		mrule[to_typedef] = create_rule_typedef();
		mrule[to_variable] = create_rule_variable();
		mrule[to_function] = create_rule_function();
		mrule[to_class] = create_rule_class();
	}
};
