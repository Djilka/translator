#pragma once
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <stack>
using namespace std;


enum t_type_object {
	to_variable, to_function, to_class, to_typedef, to_none
};

enum t_type_access {
	ta_public, ta_protected, ta_private
};

enum t_type_specifier {
	ts_virtual, ts_abstract, ts_none
};

enum t_type_variable {
	tv_variable, tv_pointer, tv_reference, 
	tv_move, tv_temp, tv_none
};

enum t_type_function {
	tf_constructor, tf_destructor, tf_none
};

struct t_type {
	string type;
	t_type_variable type_variable = tv_none;
	bool is_temp = false;

	void add(string t, t_type_variable tv, bool temp)
	{
		type = t;
		type_variable = tv;
		is_temp = temp;
	}

	void print()
	{
		cout << "\ttype : " << type << "\n";
		cout << "\ttype_variable : " << type_variable << "\n";
	}
};

class t_meta {
public:
	t_type_object type_object = to_none;
	t_type_access type_access = ta_public;
	string name;
	virtual void add(t_meta *ptr) {}

	void add_access(t_type_access type) { type_access = type; }
	void add_name(string str) { name = str;}
	virtual void print()
	{
		cout << "t_meta:\n";
		cout << "\tname : " << name << "\n";
		cout << "\ttype_access : " << type_access << "\n";
		cout << "\ttype_object : " << type_object << "\n";
		cout << "\n\n";
	}
};

typedef vector<t_meta*> tv_meta;

class t_meta_variable : public t_meta {
public:
	t_type type;
	bool is_static = false;
	bool is_const = false;

	void add(t_meta *ptr) {}
	void print()
	{
		cout << "t_meta_variable:\n";
		cout << "\tname : " << name << "\n";
		type.print();
		cout << "\ttype_access : " << type_access << "\n";
		cout << "\ttype_object : " << type_object << "\n";
		cout << "\tis_static : " << is_static << "\n";
		cout << "\tis_const : " << is_const << "\n";
		cout << "\n\n";
	}
};

class t_meta_function : public t_meta {
public:
	t_type type;
	t_type_specifier type_specifier = ts_none;
	t_type_function type_function = tf_none;
	bool is_static = false;
	tv_meta sub;

	void add(t_meta *ptr) { sub.push_back(ptr); }
	void print()
	{
		cout << "t_meta_variable:\n";
		cout << "\tname : " << name << "\n";
		type.print();
		cout << "\ttype_access : " << type_access << "\n";
		cout << "\ttype_object : " << type_object << "\n";
		cout << "\ttype_specifier : " << type_specifier << "\n";
		cout << "\ttype_function : " << type_function << "\n";
		cout << "\tis_static : " << is_static << "\n";
		for (t_meta *ptr : sub)
			ptr->print();
		cout << "\n\n";
	}
};

class t_meta_class : public t_meta {
public:
	t_type_specifier type_specifier = ts_none;
	tv_meta sub;

	void add(t_meta *ptr) { sub.push_back(ptr); }
	void print()
	{
		cout << "t_meta_variable:\n";
		cout << "\tname : " << name << "\n";
		cout << "\ttype_access : " << type_access << "\n";
		cout << "\ttype_object : " << type_object << "\n";
		cout << "\ttype_specifier : " << type_specifier << "\n";
		for (t_meta *ptr : sub)
			ptr->print();
		cout << "\n\n";
	}
};

class t_meta_typedef : public t_meta {
public:
	t_type type;

	void add(t_meta *ptr) {}
	void print()
	{
		cout << "t_meta_variable:\n";
		cout << "\tname : " << name << "\n";
		type.print();
		cout << "\ttype_access : " << type_access << "\n";
		cout << "\ttype_object : " << type_object << "\n";
		cout << "\n\n";
	}
};

class t_manager_meta {
public:
	static
	t_meta* create(t_type_object type)
	{
		t_meta *ptr = nullptr;
		switch (type) {
			case to_class:
				ptr = new t_meta_class;
				break;
			case to_function:
				ptr = new t_meta_function;
				break;
			case to_variable:
				ptr = new t_meta_variable;
				break;
			case to_typedef:
				ptr = new t_meta_typedef;
				break;
			default:
				cout << "t_create_meta: ERROR\n";
				ptr = new t_meta;
		}
		if (ptr)
			ptr->type_object = type;
		return ptr;
	}

	static
	void destroy(t_meta *ptr)
	{
		if (nullptr == ptr)
			return;
		switch (ptr->type_object) {
			case to_class:
				destroy(reinterpret_cast<t_meta_class*>(ptr)->sub);
				break;
			case to_function:
				destroy(reinterpret_cast<t_meta_function*>(ptr)->sub);
				break;
		}

		delete ptr;
	}

	static
	void destroy(tv_meta meta)
	{
		for (t_meta *ptr : meta)
			destroy(ptr);
	}

};
