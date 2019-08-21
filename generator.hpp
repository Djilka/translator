
class t_generator_base {
protected:
	
public:
	t_generator_base()
	{
		// init_type();
		to = nullptr;
	}
	
	t_generator_base(type_code rsc)
	{
		// init_type();
		to = t_rule_factory::create(rsc);
	}

	virtual void init_type() {}
	virtual void gen_run(string folder, string file) {}
	virtual string gen_head() { return ""; }
	virtual string gen_end() { return ""; }
	virtual string gen_fun(t_metadata_base meta) { return ""; }
	virtual string gen_constr(t_metadata_base meta) { return ""; }
	virtual string gen_destroy() { return ""; }

	bool is_OK()
	{
		return to != nullptr;
	}

	void run(t_rule *rule, string file)
	{
		from = rule;
		
		int index = file.find(".");
		index = index == string::npos ? file.size() : index;
		file_name = string(file, 0, index);
		
		string folder = "jni_test/";
		
		gen_run(folder, file_name);
	}

	void write_file(string file, string gen)
	{
		ofstream stream(file);
		if (stream.good())
			stream << gen;
		stream.close();
	}

	string gen_oper()
	{
		string gen;
		for (int i = 0; i < names.size(); i++)
			gen += (i ? ", " : "") + names[i];
		return gen;
	}

	string gen_arg(string type, string name)
	{
		string gen;
		string nm = name.size() ? name : "args";
		names.push_back(nm);

		string t = from->get_type_ref(type);
		gen += mtype.count(t) ? mtype[t] : mtype["long"];
		gen += " " + nm;
		return gen;
	}

	string gen_args(t_metadata_base meta, string sep)
	{
		names.clear();
		string gen;
		int i = 0;
		for (t_metadata_base val : meta.sub) {
			if (val.tm != tm_val)
				continue;
			gen += i++ ? ", " : sep;
			gen += gen_arg(val.type, val.name);
		}
		return gen;
	}

public:
	t_rule *to, *from;
	map<string, string> mtype;
	vector<string> names;
	string file_name;
	string name_class;
	string ptr_class = "handle";
};

class t_generator_jni : public t_generator_base {
public:
	t_generator_jni(type_code rsc) : t_generator_base(rsc)
	{
		init_type();
	}

	void init_type()
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
	}

	void gen_run(string folder, string file)
	{
		string gen = gen_head();

		for (t_metadata_base m : from->get_meta()) {
			if (m.tm != tm_class)
				continue;
			
			name_class = m.name;
			bool is_const = false;

			for (t_metadata_base mf : m.sub) {
				if (mf.tm != tm_fun)
					continue;
				cout << "try print jni fun;\n";
				if (m.name == mf.type && mf.name == "") {
					// is_const = true;
					// gen += gen_jni_constr(m.name, mf);
				} else
					gen += gen_fun(mf);
			}

			if (!is_const)
				gen += gen_constr(t_metadata_base());
			gen += gen_destroy();
		}

		gen += gen_end();
		write_file(folder + "jni_" + file + ".hpp", gen);
	}

	string gen_head()
	{
		string gen;
		gen += "#include <jni.h>\n";
		gen += "#include <" + file_name + ".h>\n\n";
		gen += "#ifndef _Included_" + file_name + "\n";
		gen += "#define _Included_" + file_name + "\n";
		gen += "#ifdef __cplusplus\n";
		gen += "extern \"C\" {\n";
		gen += "#endif\n\n";
		return gen;
	}

	string gen_end()
	{
		string gen;
		gen += "#ifdef __cplusplus\n";
		gen += "}\n";
		gen += "#endif\n";
		gen += "#endif\n\n";
		return gen;
	}

	string gen_fun_head(string type, string name, string args)
	{
		// JNIEXPORT jlong JNICALL Java_MyJavaClass_Function1(args) {
		string gen;
		gen += "JNIEXPORT " + type + " JNICALL Java_" + name_class;
		gen += "__"+ name +"(JNIEnv *env, jobject obj, jlong " + ptr_class;
		gen += (args.size() ? ", " : "") + args + ")\n";
		gen += "{\n";
		return gen;
	}

	string gen_arg_cast(string type, string &name)
	{
		string gen;
		string t = from->get_type_ptr(from->get_type_ref(type));
		cout << "gen_arg_cast type = " << type << "; jtype = " << t << "\n";
		string ptr = name + "_cast";

		if (t == "std::string") {
			string str = name + "_ptr";
			gen += "\tconst char *" + str + " = env->GetStringUTFChars(" + name + ", NULL);\n";
			gen += "\t" + t + " " + ptr + ";\n";
			gen += "\tif (NULL != " + str + ")\n";
			gen += "\t\t" + ptr + " = " + t + "(" + str + ");\n";
		} else if (from->is_std_ptr(t)) {
			gen += "\t" + t + " " + ptr + " = (" + from->get_type_std_ptr(t) + "*)" + name + ";\n";
		} else if (from->is_optional(t)) {
			gen += "\t" + t + " " + ptr + " = *((" + from->get_type_std_ptr(t) + "*)" + name + ");\n";
		} else if (!from->is_type_simple(type)) {
			gen += "\t" + t + " *" + ptr + " = (" + t + "*)" + name + ";\n";
		}
		name = ptr;
		return gen;
	}

	string gen_args_cast(t_metadata_base meta)
	{
		string gen;
		int i = 0;
		for (t_metadata_base val : meta.sub) {
			if (val.tm != tm_val || from->is_type_simple(val.type))
				continue;

			gen += gen_arg_cast(val.type, names[i++]);
			i++;
		}
		return gen;
	}

	string gen_ptr(string type, string ptr, string name)
	{
		// MyClass* ptr = (MyClass*)lp;
		return 	"\t" + type + " *" + ptr + " = (" + type + "*)" + name + ";\n";
	}

	string gen_ptr(string type, string type_case, string ptr, string name)
	{
		// MyClass* ptr = (MyClass*)lp;
		return 	"\t" + type + " *" + ptr + " = (" + type_case + "*)" + name + ";\n";
	}

	string gen_ptr_new(string type, string ptr)
	{
		return 	"\t" + type + " *" + ptr + " = new " + type;
	}

	string gen_ptr_ret(string ptr)
	{
		return "\treturn (jlong)" + ptr + ";\n";
	}

	string gen_fun_invoke(string name, string sub_fun = "")
	{
		return ptr_cast + "->" + name + "(" + gen_oper() + ")" + sub_fun + ";\n";
	}	

	string gen_fun(t_metadata_base meta)
	{
		string jtype = mtype.count(meta.type) ? mtype[meta.type] : mtype["long"];
		cout << "gen_fun type = " << meta.type << "; jtype = " << jtype << "\n";
		string gen = gen_fun_head(jtype, meta.name, gen_args(meta, ""));
		gen += gen_args_cast(meta);
		string header = ptr_class;
		gen += gen_arg_cast(name_class, ptr_class);

		string type = from->get_type_ref(meta.type);

		if (type == "void") {
			gen += "\t" + gen_fun_invoke(meta.name);
		} else if (from->is_std_ptr(type)) {
			gen += "\treturn (jlong) " + gen_fun_invoke(meta.name, ".release()");
		} else if (from->is_type_ptr(type)) {
			gen += "\treturn (jlong) " + gen_fun_invoke(meta.name);
		} else if (from->is_type_simple(type)) {
			gen += "\treturn " + gen_fun_invoke(meta.name);
		} else {
			gen += "\t" + type + " *" + ptr_ret + " = new " + type + ";\n";
			gen += "\t*" + ptr_ret + " = " + gen_fun_invoke(meta.name);
			gen += "\treturn (jlong) " + ptr_ret + ";\n";
		}

		ptr_class = header;
		gen += "}\n\n";
		return gen;
	}
	
	string gen_constr(t_metadata_base meta)
	{
		// JNIEXPORT jlong JNICALL Java_MyJavaClass_Function1() {
		//	 MyClass* ptr = new MyClass(...);
		//	 return (long)ptr;
		// }
		string args = gen_args(meta, "");
		string gen = gen_fun_head(mtype["long"], "initClass", args);
		gen += gen_args_cast(meta);
		gen += gen_ptr_new(name_class, ptr_cast) + "(" + gen_oper() + ");\n";
		gen += gen_ptr_ret(ptr_cast);
		gen += "}\n\n";
		return gen;
	}
	
	string gen_destroy()
	{
		string gen = gen_fun_head("void", "destroyClass", "");
		gen += gen_ptr(name_class, ptr_cast, ptr_class);
		gen += "\tdelete " + ptr_cast + ";\n";
		gen += "}\n\n";
		return gen;
	}

	string ptr_cast = ptr_class + "_cast";
	string ptr_ret = "result";
};

class t_generator_java : public t_generator_base {
public:
	t_generator_java(type_code rsc) : t_generator_base(rsc)
	{
		init_type();
	}

	void init_type()
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
		mtype["std::optional"] = "Optional";
		mtype["std::unique_ptr"] = "long";
	}

	void gen_run(string folder, string file)
	{
		// gen constructr right
		for (t_metadata_base m : from->get_meta()) {
			if (m.tm != tm_class)
				continue;
			
			name_class = m.name;
			bool is_const = false;
			string gen = gen_head();
			
			for (t_metadata_base mf : m.sub) {
				if (mf.tm != tm_fun)
					continue;
				
				cout << "try print fun;\n";
				if (m.name == mf.type) {
					// if (from->cmp_types(m.name, mf.type))
					// 	gen += gen_java_constr_private();
					is_const = true;
				} else {
					gen += gen_fun(mf);
				}
			}
			if (!is_const)
				cout << "need print constructr\n";
			gen += gen_destroy();
			gen += gen_end();
			write_file(folder + m.name + ".java", gen);
		}
	}

	string gen_head()
	{
		// fix native
		string gen = "public class " + name_class + " {\n";
		gen += "\tprivate long " + ptr_class + ";\n\n";
		gen += "\tnative private long _initClass();\n";

		gen += "\tpublic " + name_class + "()\n";
		gen += "\t{\n";
		gen += "\t\t" + ptr_class + " = _initClass();\n";
		gen += "\t}\n\n";;
		return gen;
	}

	string gen_end()
	{
		return "}";
	}

	string gen_fun(t_metadata_base meta)
	{
		string gen = gen_native(meta, false);
		gen += gen_public(meta);
		return gen;
	}

	string gen_native(t_metadata_base meta, bool is_const)
	{
		string gen = "\tnative private ";
		string t = from->get_type_ptr(from->get_type_ref(meta.type));
		gen += !from->is_type_simple(t) ? "long" : t;
		gen += " _" + meta.name + "(";
		string args = gen_args(meta, "");
		if (!is_const) {
			gen += "long _" + ptr_class;
			if (args.size())
				gen += ", ";
		}
		gen += args + ");\n";
		return gen;
	}

	string gen_public(t_metadata_base meta)
	{
		string t = from->get_type_ref(meta.type);
		string gen = "\tpublic " + t + " " + meta.name + "(";
		gen += gen_args(meta, "") + ")\n";
		gen += "\t{\n";
		string str = "_" + meta.name + "(" + ptr_class + 
					(names.size() ? ", " : "") + gen_oper();
		gen += "\t\t";
		if (meta.type != "void")
			gen += "return ";
		if (name_class == t)
			gen += name_class + "(" + str + ")";
		else
			gen += str;
		gen += ");\n";
		gen += "\t}\n\n";
		return gen;
	}

	string gen_constr_private()
	{
		string gen = "\tprivate " + name_class + "(long _" + ptr_class + ")\n";
		gen += "\t{\n";
		gen += "\t\t" + ptr_class + " = _" + ptr_class + ";\n";
		gen += "\t}\n\n";
		return gen;
	}

	string gen_constr(t_metadata_base meta)
	{
		// add args
		string gen = "\tnative private long _initClass();\n";

		gen += "\tpublic " + name_class + "()\n";
		gen += "\t{\n";
		gen += "\t\t" + ptr_class + " = _initClass();\n";
		gen += "\t}\n\n";
	}

	string gen_destroy()
	{
		string gen = "\tnative private void _destroyClass(long _" + ptr_class + ");\n\n";
		auto lambda = [&](string head)
			{
				gen += head;
				gen += "\t{\n";
				gen += "\t\t_destroyClass(" + ptr_class + ");\n";
				gen += "\t\t" + ptr_class + " = 0L;\n";
				gen += "\t}\n\n";
			};

		lambda("\tpublic void destroy()\n");
		lambda("\tprotected void finalize() throws Throwable\n");

		return gen;
	}
};

class t_generator {
	t_generator_java *g_java = nullptr;
	t_generator_jni *g_jni = nullptr;
public:
	t_generator(type_code rsc)
	{
		g_java = new t_generator_java(rsc);
		g_jni = new t_generator_jni(rsc);
	}

	~t_generator()
	{
		delete g_java;
		delete g_jni;
	}

	bool is_OK()
	{
		return g_jni && g_java && g_jni->is_OK() && g_java->is_OK();
	}

	void run(t_rule *rule, string file)
	{
		g_jni->run(rule, file);
		g_java->run(rule, file);
	}

};
