#include "generator.hpp"

void t_generator::write_file(string file)
{
	for (int i = 0; i < vstr.size(); i++) {
		ofstream stream(file + exe[i]);
		if (stream.good()) {
			stream << vstr_header[i];
			stream << vstr[i];
		}
		stream.close();
	}
	vstr_clear();
}

void t_generator::gen_run(tv_meta vmeta, string folder)
{
	for (t_meta *m : vmeta) {
		if (m->type_object != to_class)
			continue;
		
		t_meta_class *ptr_class = reinterpret_cast<t_meta_class*>(m);
		name_class = ptr_class->name;
		gen_head();
		need_fun = false;
		is_fun_generate = false;
		bool is_destroy = false;

		for (t_meta *mf : ptr_class->sub) {
			t_meta_function *ptr = reinterpret_cast<t_meta_function*>(mf);
			if (ptr->type_object != to_function ||
				ptr->type_access != ta_public ||
				ptr->type_specifier != ts_none)
				continue;

			if (ptr->type_function == tf_constructor) {
				gen_constr(ptr);
				if (!is_destroy)
					gen_destroy(ptr);
				is_destroy = true;
			} else
				gen_fun(ptr);
		}
		gen_end();
		gen_include();
		write_file(folder + m->name);
	}
}

string t_generator::gen_type_jni(t_type type)
{
	string convert_type = type.type;
	
	if (type.type_variable == tv_pointer)
		return mtype[type_pointer];

	if (info.is_type_name(info.get_type_free(convert_type)))
		convert_type = info.get_type_name(info.get_type_free(convert_type));

	if (type.type_variable != tv_variable)
		convert_type = info.get_type_free(convert_type);

	if (info.is_temp(convert_type)) {
		convert_type = info.get_type_std(convert_type);
		need_fun = info.is_function(convert_type) || need_fun;
	} else if (info.is_type_add(convert_type))
		convert_type = type_pointer;

	return mtype[convert_type];
}

void t_generator::gen_args(tv_meta meta)
{
	string type, name;
	for (t_meta *m : meta) {
		if (m->type_object != to_variable)
			continue;
		t_meta_variable *ptr = reinterpret_cast<t_meta_variable*>(m);
		name = ptr->name.size() ? ptr->name : "arg";
		type = gen_type_jni(ptr->type);
		vargs.push_back({type, name});
	}
}

void t_generator::run(tv_meta vmeta, string file)
{
	file_name = file;
	string folder = "";
	gen_run(vmeta, folder);
}

void t_generator::vstr_clear()
{
	for (int i = 0; i < vstr.size(); i++) {
		vstr[i].clear();
		vstr_header[i].clear();
	}
}

void t_generator::vstr_init(int i_max)
{
	for (int i = 0; i < i_max; i++) {
		vstr.push_back("");
		vstr_header.push_back("");
	}
}

void t_generator::args_clear()
{
	vargs.clear();
	arg_result.clear();
	arg_handle.clear();

	vargs_java.clear();
	arg_result_java.clear();
	arg_handle_java.clear();
}

void t_generator_jni::gen_type_cast_pod(t_type_variable tv, string to_cast, string &name)
{
	string name_cast = name + "_cast";
	if (tv == tv_pointer) {
		// pointer
		// type* ptr = (type*)val;
		vstr[tf_c] += "\t" + to_cast + " *" + name_cast + " = (" + to_cast + "*)" + name + ";\n";
		name = name_cast;
	} else if (tv == tv_move) {
		// move = type&&
		name = "std::move((" + to_cast + ")" + name + ")";
	}
}

void t_generator_jni::gen_type_cast_string(string to_cast, string &name)
{
	string name_cast = name + "_cast";
	string str = name + "_char";
	vstr[tf_c] += "\tconst char *" + str + " = env->GetStringUTFChars(" + name + ", NULL);\n";
	vstr[tf_c] += "\t" + to_cast + " " + name_cast + ";\n";
	vstr[tf_c] += "\tif (NULL != " + str + ")\n";
	vstr[tf_c] += "\t\t" + name_cast + " = " + to_cast + "(" + str + ");\n";
	name = name_cast;
}

void t_generator_jni::gen_type_cast_add(t_type_variable tv, string to_cast, string &name)
{
	string name_cast = name + "_cast";
	if (tv == tv_pointer) {
		// pointer
		// type* ptr = (type*)val;
		vstr[tf_c] += "\t" + to_cast + " *" + name_cast + " = (" + to_cast + "*)" + name + ";\n";
		name = name_cast;
	} else if (tv == tv_move) {
		// move = type&&
		name = "std::move(*((" + to_cast + "*)" + name + "))";
	} else if (tv == tv_reference) {
		// reference = type&
		vstr[tf_c] += "\t" + to_cast + " &" + name_cast + " = *((" + to_cast + "*)" + name + ");\n";
		name = name_cast;
	} else {
		// variable
		vstr[tf_c] += "\t" + to_cast + " " + name_cast + " = *((" + to_cast + "*)" + name + ");\n";
		name = name_cast;
	}
}

void t_generator_jni::gen_type_cast_unique_ptr(t_type_variable tv, string to_cast, string &name)
{
	string type = info.get_type_temp(to_cast);
	string name_cast = name + "_cast";
	if (tv == tv_move) {
		// move = type&&
		vstr[tf_c] += "\tstd::unique_ptr<" + type + "> ";
		vstr[tf_c] += name_cast + "((" + type + "*)" + name + ");\n";
		name = "std::move(" + name_cast + ")";
	} else if (tv == tv_reference) {
		// reference = type&
		vstr[tf_c] += "\t" + to_cast + " &" + name_cast + " = (" + type + "*)" + name + ";\n";
		name = name_cast;
	} else if (tv == tv_variable) {
		// variable
		vstr[tf_c] += "\t" + to_cast + " " + name_cast + " = (" + type + "*)" + name + ";\n";
		name = name_cast;
	}
}

void t_generator_jni::gen_type_cast_optional(t_type_variable tv, string to_cast, string &name)
{
	string type = info.get_type_temp(to_cast);
	string name_cast = name + "_cast";
	if (tv == tv_move) {
		// move = type&&
		name = "std::move(*((" + type + "*)" + name + "))";
	} else if (tv == tv_reference) {
		// reference = type&
		vstr[tf_c] += "\t" + to_cast + " &" + name_cast + " = *(" + type + "*)" + name + ";\n";
		name = name_cast;
	} else if (tv == tv_variable) {
		// variable
		vstr[tf_c] += "\t" + to_cast + " " + name_cast + " = *(" + type + "*)" + name + ";\n";
		name = name_cast;
	}
}

void t_generator_jni::gen_type_cast_fun(string to_cast, string &name)
{
	string name_cast = name + "_cast";
	need_fun = true;
	vstr[tf_c] += "\ts_obj = " + name + ";\n";
	vstr[tf_c] += "\ts_env = env;\n";
	vstr[tf_c] += "\tjclass cls = env->GetObjectClass(" + name + ");\n";
	vstr[tf_c] += "\ts_mid = env->GetMethodID(cls, \"fun\", \"()V\");\n";
	vstr[tf_c] += "\tif (s_mid == 0)\n";
	vstr[tf_c] += "\t\treturn;\n";
	name = "fun";
}

t_generator_jni::t_generator_jni(t_info i)
{
	init_type();
	exe = {".c", ".h"};
	vstr_init(tf_none);
	info = i;
}

void t_generator_jni::init_type()
{
	mtype["bool"] = "jboolean"; // unsigned 8 bits
	mtype["byte"] = "jbyte"; // signed 8 bits
	mtype["char"] = "jchar"; // unsigned 16 bits
	mtype["short"] = "jshort"; // signed 16 bits
	mtype["int"] = "jint"; // signed 32 bits
	mtype["long"] = "jlong"; // unsigned 64 bits
	mtype["float"] = "jfloat"; // 32 bits
	mtype["double"] = "jdouble"; // 64 bits
	mtype["void"] = "void"; // N/A
	mtype["std::string"] = "jstring";
	mtype["std::optional"] = "jlong";
	mtype["std::unique_ptr"] = "jlong";
	mtype["std::function"] = "jobject";
}

void t_generator_jni::gen_args_cast(tv_meta vmeta)
{
	int i = 0;
	for (t_meta *m : vmeta) {
		if (m->type_object != to_variable)
			continue;
		t_meta_variable *ptr = reinterpret_cast<t_meta_variable*>(m);
		gen_arg_cast(ptr->type, vargs[i++][1]);
	}
}

void t_generator_jni::gen_arg_cast(t_type type, string &name)
{
	string to_cast = info.get_type_free(type.type);
	string name_cast = name + "_cast";
	if (info.is_type_pod(to_cast)) {
		gen_type_cast_pod(type.type_variable, to_cast, name);
	} else if (to_cast == "std::string") {
		gen_type_cast_string(to_cast, name);
	} else if (info.is_type_name(to_cast)) {
		string real_name = info.get_type_name(to_cast);
		t_type real_type = type;
		real_type.type = real_name;
		gen_arg_cast(real_type, name);
	} else if (info.is_type_add(to_cast)) {
		gen_type_cast_add(type.type_variable, to_cast, name);
	} else if (info.is_function(to_cast)) {
		gen_type_cast_fun(to_cast, name);
	} else if (info.is_unique_ptr(to_cast)) {
		gen_type_cast_unique_ptr(type.type_variable, to_cast, name);
	} else if (info.is_optional(to_cast)) {
		gen_type_cast_optional(type.type_variable, to_cast, name);;
	}
}

void t_generator_jni::gen_include()
{
	for (int i = 0; i < tf_none; i++)
		vstr_header[i] += "#include <jni.h>\n\n";

	vstr_header[tf_c] += "#include \"" + name_class + ".h\"\n";
	vstr_header[tf_c] += "#include \""+ file_name +"\"\n\n";
}

void t_generator_jni::gen_head()
{
	// header
	vstr[tf_h] += "#ifndef _Included_" + name_class + "\n";
	vstr[tf_h] += "#define _Included_" + name_class + "\n\n";
	vstr[tf_h] += "#ifdef __cplusplus\n";
	vstr[tf_h] += "extern \"C\" {\n";
	vstr[tf_h] += "#endif\n\n";
}

void t_generator_jni::gen_end()
{
	// header
	vstr[tf_h] += "#ifdef __cplusplus\n";
	vstr[tf_h] += "}\n";
	vstr[tf_h] += "#endif\n";
	vstr[tf_h] += "#endif\n\n";
}

string t_generator_jni::gen_fun_type()
{
	string gen = "static jmethodID s_mid;\n";
	gen += "static jobject s_obj;\n";
	gen += "static JNIEnv *s_env;\n";
	gen += "typedef void t_fun();\n\n";
	gen += "void fun()\n";
	gen += "{\n";
	gen += "\ts_env->CallVoidMethod(s_obj, s_mid);\n";
	gen += "}\n\n";
	is_fun_generate = true;
	return gen;
}

void t_generator_jni::gen_fun_head(t_meta_function *meta)
{
	// JNIEXPORT jlong JNICALL Java_MyJavaClass_Function1(args)
	args_clear();
	gen_args(meta->sub);

	string type_result = "void";
	if (meta->type.type != "void") {
		type_result = gen_type_jni(meta->type);
		arg_result = { gen_type_jni(meta->type), name_result };

	}
	if (meta->type_function != tf_constructor && !meta->is_static) {
		arg_handle = { "jlong", name_value };
		type_handle.add(name_class + "*", tv_pointer, false);
	}

	if (need_fun && !is_fun_generate)
		vstr[tf_c] += gen_fun_type();

	string gen;
	gen += "JNIEXPORT " + type_result + " JNICALL Java_" + name_class;
	gen += "_n"+ meta->name +"(\n";
	gen += "\tJNIEnv *env, jobject obj";
	if (arg_handle.size())
		gen += ", " + arg_handle[0] + " " + arg_handle[1];
	for (vector<string> arg : vargs)
		gen += ", " + arg[0] + " " + arg[1];
	gen += ")";

	vstr[tf_h] += gen + ";\n\n";
	vstr[tf_c] += gen + "\n";
}

string t_generator_jni::gen_fun_invoke(string name, string sub_fun)
{
	string oper;
	int i = 0;
	for (vector<string> v : vargs) {
		oper += (i++ ? ", " : "") + v[1];
	}

	string res = name + "(" + oper + ")" + sub_fun + "";
	return arg_handle.size() ? arg_handle[1] + "->" + res : name_class + "::" + res;
}

void t_generator_jni::gen_fun_body(t_meta_function *meta)
{
	vstr[tf_c] += "{\n";
	// cast handler
	if (arg_handle.size())
		gen_arg_cast(type_handle, arg_handle[1]);
	// cast args
	if (vargs.size())
		gen_args_cast(meta->sub);
	// cast result

	vstr[tf_c] += "\ttry {\n";
	string str_invoke = gen_fun_invoke(meta->name);
	if (!arg_result.size()) {
		vstr[tf_c] += "\t\t" + str_invoke + ";\n";
	} else {
		// create type
		string to_cast = info.get_type_free(meta->type.type);

		if (info.is_type_pod(to_cast)) {
			vstr[tf_c] += "\t\treturn " + str_invoke + ";\n";
		} else if (info.is_unique_ptr(to_cast)) {
			vstr[tf_c] += "\t\treturn (jlong)" + str_invoke + ".release();\n";
		} else {
			vstr[tf_c] += "\t\treturn (jlong)&" + str_invoke + ";\n";
		}
	}
	vstr[tf_c] += "\t} catch(...) {\n";
	vstr[tf_c] += "\t\tenv->ThrowNew(env->FindClass(\"java/land/Exception\"), \"Error\");\n";
	vstr[tf_c] += "\t}\n";

	// cast result
	vstr[tf_c] += "}\n\n";
}

void t_generator_jni::gen_fun(t_meta_function *meta)
{
	gen_fun_head(meta);
	gen_fun_body(meta);
}

void t_generator_jni::gen_constr(t_meta_function *meta)
{
	t_meta_function meta_tmp = *meta;
	meta_tmp.name = name_constructor;
	gen_fun_head(&meta_tmp);

	// body
	string oper;
	int i = 0;
	for (vector<string> v : vargs) {
		oper += (i++ ? ", " : "") + v[1];
	}
	vstr[tf_c] += "{\n";
	vstr[tf_c] += "\t" + name_class + " *ptr = new " + name_class + "(" + oper + ");\n";
	vstr[tf_c] += "\treturn (jlong)ptr;\n";
	vstr[tf_c] += "}\n\n";
}

void t_generator_jni::gen_destroy(t_meta_function *meta)
{
	t_meta_function meta_tmp = t_meta_function();
	meta_tmp.name = name_destructor;
	meta_tmp.type = {.type = "void"};
	meta_tmp.sub.clear();
	gen_fun_head(&meta_tmp);

	// body
	vstr[tf_c] += "{\n";
	vstr[tf_c] += "\tdelete (" + name_class + "*)" + name_value + ";\n";
	vstr[tf_c] += "}\n\n";
}

t_generator_java::t_generator_java(t_info i)
{
	init_type_java();
	init_type();
	exe = {".java"};
	vstr_init(tf_none);
	info = i;
}

void t_generator_java::init_type()
{
	mtype["bool"] = "boolean"; // unsigned 8 bits
	mtype["byte"] = "byte"; // signed 8 bits
	mtype["char"] = "char"; // unsigned 16 bits
	mtype["short"] = "short"; // signed 16 bits
	mtype["int"] = "int"; // signed 32 bits
	mtype["long"] = "long"; // unsigned 64 bits
	mtype["float"] = "float"; // 32 bits
	mtype["double"] = "double"; // 64 bits
	mtype["void"] = "void"; // N/A
	mtype["std::string"] = "String";
	mtype["std::optional"] = "long";
	mtype["std::unique_ptr"] = "long";
	mtype["std::function"] = "IFun";
}

void t_generator_java::init_type_java()
{
	mtype_java["bool"] = "boolean"; // unsigned 8 bits
	mtype_java["byte"] = "byte"; // signed 8 bits
	mtype_java["char"] = "char"; // unsigned 16 bits
	mtype_java["short"] = "short"; // signed 16 bits
	mtype_java["int"] = "int"; // signed 32 bits
	mtype_java["long"] = "long"; // unsigned 64 bits
	mtype_java["float"] = "float"; // 32 bits
	mtype_java["double"] = "double"; // 64 bits
	mtype_java["void"] = "void"; // N/A
	mtype_java["std::string"] = "String";
	mtype_java["std::optional"] = "Optional";
	mtype_java["std::unique_ptr"] = "long";
	mtype_java["std::function"] = "IFun";
}

void t_generator_java::gen_args_java(tv_meta meta)
{
	string type, name;
	for (t_meta *m : meta) {
		if (m->type_object != to_variable)
			continue;
		t_meta_variable *ptr = reinterpret_cast<t_meta_variable*>(m);
		name = ptr->name.size() ? ptr->name : "arg";
		type = gen_type_java(ptr->type);
		vargs_java.push_back({type, name});
	}
}

string t_generator_java::gen_type_java(t_type type)
{
	string convert_type = info.get_type_free(type.type);
	
	if (info.is_temp(convert_type)) {
		string real_name = info.get_type_temp(convert_type);
		convert_type = info.get_type_std(convert_type);
		t_type real_type = type;
		real_type.type = real_name;
		if (info.is_function(convert_type)) {
			need_fun = true;
			return mtype_java[convert_type];
		} else if (info.is_unique_ptr(convert_type))
			return gen_type_java(real_type);
		else {
			type_header.insert(mtype_java[convert_type]);
			return mtype_java[convert_type] + "<" + gen_type_java(real_type) + ">";
		}
	} else if (info.is_type_name(convert_type)) {
		type.type = info.get_type_name(convert_type);
		return gen_type_java(type);
	} else if (info.is_type_add(convert_type))
		return convert_type;
	
	return mtype_java[convert_type];
}

void t_generator_java::gen_include()
{
	for (string name : type_header)
		vstr_header[tf_java] += "import java.util."+ name + ";\n";
	vstr_header[tf_java] += "\n";
	type_header.clear();
}

void t_generator_java::gen_head()
{
	vstr[tf_java] += "public class " + name_class + " {\n";

	vstr[tf_java] += "\tprivate long " + name_value + ";\n\n";
	
	vstr[tf_java] += "\tpublic long get_" + name_value + "()\n";
	vstr[tf_java] += "\t{\n";
	vstr[tf_java] += "\t\treturn " + name_value + ";\n";
	vstr[tf_java] += "\t}\n\n";

	vstr[tf_java] += "\tprivate " + name_class + "(long h)\n";
	vstr[tf_java] += "\t{\n";
	vstr[tf_java] += "\t\t" + name_value + " = h;\n";
	vstr[tf_java] += "\t}\n\n";
}

void t_generator_java::gen_end()
{
	vstr[tf_java] += "}";
}

void t_generator_java::gen_fun_type()
{
	vstr[tf_java] += "\tinterface " + type_fun_name + " {\n";
	vstr[tf_java] += "\t\tvoid fun();\n\n";
	vstr[tf_java] += "\t\tdefault\n";
	vstr[tf_java] += "\t\tvoid print() {System.out.println(\"IFun default fun\");}\n";
	vstr[tf_java] += "\t}\n";
	is_fun_generate = true;
}

string t_generator_java::gen_fun_head(bool is_native, bool is_static, string type, string name,
	vector<vector<string>> args)
{
	string gen = is_static ? "\tstatic\n" : "";
	gen += is_native ? "\tnative private " : "\tpublic ";
	gen += type + (is_native ? " n" : " ") + name + "(";
	int i = 0;
	if (arg_handle.size() && is_native) {
		gen += arg_handle[0] + " " + arg_handle[1];
		i++;
	}
	for (vector<string> arg : args) {
		gen += (i ? ", " : "")+ arg[0] + " " + arg[1];
		i++;
	}
	gen += ")";
	return gen;
}

void t_generator_java::gen_fun_head(t_meta_function *meta)
{
	args_clear();
	gen_args(meta->sub);
	gen_args_java(meta->sub);

	if (need_fun && !is_fun_generate)
		gen_fun_type();

	string type_result = "void", type_result_java = "void";
	if (meta->type.type != "void") {
		type_result = gen_type_jni(meta->type);
		arg_result = { gen_type_jni(meta->type), name_result };
		type_result_java = gen_type_java(meta->type);
		arg_result_java = { gen_type_java(meta->type), name_result };
	}
	if (meta->type_function != tf_constructor && !meta->is_static) {
		arg_handle = { "long", name_value };
		type_handle.add(name_class + "*", tv_pointer, false);
	}

	vstr[tf_java] += gen_fun_head(true, meta->is_static, type_result, meta->name, vargs) + ";\n";
	vstr[tf_java] += gen_fun_head(false, meta->is_static, type_result_java, meta->name, vargs_java) + "\n";
}

string t_generator_java::gen_fun_invoke(string name, string sub_fun)
{
	string oper;
	int i = 0;
	if (arg_handle.size())
		oper += (i++ ? ", " : "") + arg_handle[1];
	for (vector<string> v : vargs_java) {
		oper += (i++ ? ", " : "") + v[1];
	}

	return "n" + name + "(" + oper + ")" + sub_fun;
}

void t_generator_java::gen_arg_cast(t_type type, string &name)
{
	string to_cast = info.get_type_free(type.type);
	string name_cast = name + "_cast";
	bool is_ptr = info.is_unique_ptr(to_cast);
	
	if ((!info.is_type_name(to_cast) && info.is_type_add(to_cast)) ||
		(is_ptr && info.is_type_add(info.get_type_temp(to_cast)))) {
		name = name + ".get_" + name_value + "()";
	} else if (info.is_optional(to_cast)) {
		string tmp = name + "_cast";
		vstr[tf_java] += "\t\tlong " + tmp + " = " + name;
		vstr[tf_java] += ".isPresent() ? " + name + ".get().get_handle() : 0;\n";
		name = tmp;
	}
}

void t_generator_java::gen_args_cast(tv_meta vmeta)
{
	int i = 0;
	for (t_meta *m : vmeta) {
		if (m->type_object != to_variable)
			continue;
		t_meta_variable *ptr = reinterpret_cast<t_meta_variable*>(m);
		gen_arg_cast(ptr->type, vargs_java[i++][1]);
	}
}

void t_generator_java::gen_fun_body(t_meta_function *meta)
{
	string to_cast = info.get_type_free(meta->type.type);
	if (info.is_unique_ptr(to_cast))
		to_cast = info.get_type_temp(to_cast);
	vstr[tf_java] += "\t{\n";

	// cast args
	if (vargs_java.size())
		gen_args_cast(meta->sub);
	
	string str_invoke = gen_fun_invoke(meta->name);
	if (meta->is_static) {
		str_invoke = name_class + "." + str_invoke;
	}

	if (to_cast == "void")
		vstr[tf_java] += "\t\t" + str_invoke + ";\n";
	else if (info.is_type_pod(to_cast)
		|| meta->type.type_variable == tv_pointer) {
		vstr[tf_java] += "\t\treturn " + str_invoke + ";\n";
	} else if (info.is_type_add(to_cast)) {
		vstr[tf_java] += "\t\treturn new ";
		vstr[tf_java] += name_class + "(" + str_invoke + ");\n";
	}

	vstr[tf_java] += "\t}\n\n";
}

void t_generator_java::gen_fun(t_meta_function *meta)
{
	gen_fun_head(meta);
	gen_fun_body(meta);
}

void t_generator_java::gen_constr(t_meta_function *meta)
{
	vstr[tf_java] += "\tnative private long n" + name_constructor + "();\n";

	vstr[tf_java] += "\tpublic " + name_class + "()\n";
	vstr[tf_java] += "\t{\n";
	vstr[tf_java] += "\t\t" + name_value + " = n" + name_constructor + "();\n";
	vstr[tf_java] += "\t}\n\n";
}

void t_generator_java::gen_destroy(t_meta_function *meta)
{
	string gen = "\tnative private void n" + name_destructor + "(long n" + name_value + ");\n\n";
	auto lambda = [&](string head)
		{
			gen += head;
			gen += "\t{\n";
			gen += "\t\tn" + name_destructor + "(" + name_value + ");\n";
			gen += "\t\t" + name_value + " = 0L;\n";
			gen += "\t}\n\n";
		};

	lambda("\tpublic void destroy()\n");
	lambda("\tprotected void finalize() throws Throwable\n");

	vstr[tf_java] += gen;
}

