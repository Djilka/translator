#pragma once
#include "language.hpp"

class t_generator {
	void write_file(string file);
	void gen_run(tv_meta vmeta, string folder);
public:

	virtual void gen_include() {}
	virtual void gen_head() {}
	virtual void gen_end() {}
	
	virtual void gen_fun_head(t_meta_function *meta) {}
	virtual void gen_fun_body(t_meta_function *meta) {}
	virtual void gen_fun(t_meta_function *meta) {}
	
	virtual void gen_constr(t_meta_function *meta) {}
	virtual void gen_destroy(t_meta_function *meta) {}
	
	string gen_type_jni(t_type type);
	void gen_args(tv_meta meta);

	void run(tv_meta vmeta, string file);
protected:
	void vstr_clear();
	void vstr_init(int i_max);
	void args_clear();
public:
	set<string> type_header;
	map<string, string> mtype;
	string type_pointer = "long";
	vector<vector<string>> vargs;
	vector<string> arg_result;
	vector<string> arg_handle;
	t_type type_handle;

	string file_name;

	string name_class;
	string name_value = "handle";
	string name_result = "result";
	string name_constructor = "initClass";
	string name_destructor = "destroyClass";

	vector<string> exe;
	vector<string> vstr;
	vector<string> vstr_header;

	// FIX remove it
	vector<vector<string>> vargs_java;
	vector<string> arg_result_java;
	vector<string> arg_handle_java;
	t_info info;
	bool need_fun;
	bool is_fun_generate = false;
};

class t_generator_jni : public t_generator {
	enum t_type_file {
		tf_c, tf_h, tf_none
	};
	
	void gen_type_cast_pod(t_type_variable tv, string to_cast, string &name);
	void gen_type_cast_string(string to_cast, string &name);
	void gen_type_cast_add(t_type_variable tv, string to_cast, string &name);
	void gen_type_cast_unique_ptr(t_type_variable tv, string to_cast, string &name);
	void gen_type_cast_optional(t_type_variable tv, string to_cast, string &name);
	void gen_type_cast_fun(string to_cast, string &name);
public:
	t_generator_jni(t_info i);
	
	void init_type();
	
	void gen_args_cast(tv_meta vmeta);
	void gen_arg_cast(t_type type, string &name);
	
	void gen_include();
	void gen_head();
	void gen_end();
	
	string gen_fun_type();
	void gen_fun_head(t_meta_function *meta);
	string gen_fun_invoke(string name, string sub_fun = "");
	void gen_fun_body(t_meta_function *meta);
	void gen_fun(t_meta_function *meta);
	
	void gen_constr(t_meta_function *meta);
	void gen_destroy(t_meta_function *meta);
};

class t_generator_java : public t_generator {
	enum t_type_file {
		tf_java, tf_none
	};
	string type_fun_name = "IFun";
	// FIX IT
	map<string, string> mtype_java;
public:
	t_generator_java(t_info i);
	
	void init_type();
	void init_type_java();
	
	void gen_arg_cast(t_type type, string &name);
	void gen_args_cast(tv_meta vmeta);
	void gen_args_java(tv_meta meta);
	string gen_type_java(t_type type);
	
	void gen_include();
	void gen_head();
	void gen_end();
	
	void gen_fun_type();
	string gen_fun_head(bool is_native, bool is_static, string type, string name,
		vector<vector<string>> args);
	void gen_fun_head(t_meta_function *meta);
	string gen_fun_invoke(string name, string sub_fun = "");
	void gen_fun_body(t_meta_function *meta);
	void gen_fun(t_meta_function *meta);
	
	void gen_constr(t_meta_function *meta);
	void gen_destroy(t_meta_function *meta);
};
