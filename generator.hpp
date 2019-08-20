
class t_generator {
	t_rule *to, *from;
	map<string, string> mtype_jni;
	map<string, string> mtype_java;
	vector<string> names;
	// std::unique_ptr<MetricPlanner> create_metric_planner(*arguments*)
	// return (jlong)create_metric_planner( ... ).release();

	void init_type_jni()
	{
		mtype_jni["bool"] = "jboolean"; // unsigned 8 bits
		mtype_jni["byte"] = "jbyte"; // signed 8 bits
		mtype_jni["char"] = "jchar"; // unsigned 16 bits
		mtype_jni["short"] = "jshort"; // signed 16 bits
		mtype_jni["int"] = "jint"; // signed 32 bits
		mtype_jni["long"] = "jlong"; // unsigned 64 bits
		mtype_jni["float"] = "jfloat"; // 32 bits
		mtype_jni["double"] = "jdouble"; // 64 bits
		mtype_jni["void"] = "void"; // N/A
		mtype_jni["std::string"] = "jstring";
		mtype_java["std::optional"] = "jlong";
		mtype_java["std::unique_ptr"] = "jlong";
	}

	void init_type_java()
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
	}

	string gen_jni_header(string file)
	{
		string gen;
		gen += "#include <jni.h>\n";
		gen += "#include <" + file + ".h>\n\n";
		gen += "#ifndef _Included_" + file + "\n";
		gen += "#define _Included_" + file + "\n";
		gen += "#ifdef __cplusplus\n";
		gen += "extern \"C\" {\n";
		gen += "#endif\n\n";
		return gen;
	}

	string gen_jni_constr(t_metadata_base meta)
	{
		// JNIEXPORT jlong JNICALL Java_MyJavaClass_Function1() {
		//	 MyClass* ptr = new MyClass(...);
		//	 return (long)ptr;
		// }
		string ptr = "ptr";
		string gen;
		gen += "JNIEXPORT jlong JNICALL Java_" + name_class + "__initClass(JNIEnv *, jobject";
		gen += gen_jni_args(meta);
		gen += ")\n";
		gen += "{\n";
		gen += gen_jni_args_convert(meta);
		gen += gen_jni_ptr_new(name_class, ptr) + "(";
		gen += gen_oper();
		gen += ");\n";
		gen += gen_jni_ptr_ret(ptr);
		gen += "}\n\n";
		return gen;
	}

	string gen_jni_destroy()
	{
		// JNIEXPORT jlong JNICALL Java_MyJavaClass_Function1() {
		//	 MyClass* ptr = (MyClass*)lp;
		//	 delete ptr;
		// }
		string ptr = "ptr";
		string gen;
		gen += "JNIEXPORT void JNICALL Java_" + name_class;
		gen += "__destroyClass(JNIEnv *, jobject, jlong " + ptr_class + ")\n";
		gen += "{\n";
		gen += gen_jni_ptr(name_class, ptr, ptr_class);
		// fixed delete all allocated
		gen += "\tdelete " + ptr + ";\n";
		gen += "}\n\n";
		return gen;
	}

	string gen_jni_arg(string type, string name)
	{
		string gen;
		string nm = name.size() ? name : "args";
		names.push_back(nm);
		string t = from->is_type_ref(type) ? from->get_type_ref(type) : type;
		cout << "gen_java_arg: type = " << t << "\n";
		
		if (from->is_type_simple(t))
			gen += mtype_jni[t];
		else if (t == "std::string")
			gen += "jstring";
		else
			gen += "jlong";

		cout << "gen_java_arg: gen = " << gen << "\n";

		gen += " " + nm;
		return gen;
	}

	string gen_jni_args(t_metadata_base meta)
	{
		names.clear();
		string gen;
		for (t_metadata_base val : meta.sub) {
			if (val.tm != tm_val)
				continue;
			gen += ", " + gen_jni_arg(val.type, val.name);
		}
		return gen;
	}

	string gen_oper()
	{
		string gen;
		for (int i = 0; i < names.size(); i++)
			gen += (i ? ", " : "") + names[i];
		return gen;
	}

	string gen_jni_args_convert(t_metadata_base meta)
	{
		string gen;
		int i = 0;
		for (t_metadata_base val : meta.sub) {
			if (val.tm != tm_val || from->is_type_simple(val.type))
				continue;

			string t = from->is_type_ref(val.type) ? from->get_type_ref(val.type) : val.type;
			t = from->is_type_ptr(t) ? from->get_type_ptr(t) : t;
			string ptr = names[i] + "_ptr";

			if (from->cmp_types(val.type, "std::string")) {
				string str = names[i] + "_str";
				gen += "\tconst char *" + ptr + " = env->GetStringUTFChars(" + names[i] + ", NULL);\n";
				gen += "\t" + t + " " + str + ";\n";
				gen += "\tif (NULL != " + ptr + ")\n";
				gen += "\t\t" + str + " = " + t + "(" + ptr + ");\n";
			} else if (!from->is_type_simple(val.type)) {
				// MyClass* ptr = (MyClass*)lp;
				gen += gen_jni_ptr(t, ptr, names[i]);
				// gen += gen_jni_ptr(val.type, ptr, names[i]);
			}
			names[i] = ptr;
			i++;
		}
		return gen;
	}

	string gen_jni_ptr(string type, string ptr, string name)
	{
		return 	"\t" + type + " *" + ptr + " = (" + type + "*)" + name + ";\n";
	}

	string gen_jni_ptr_new(string type, string ptr)
	{
		return 	"\t" + type + " *" + ptr + " = new " + type;
	}

	string gen_jni_ptr_ret(string ptr)
	{
		return "\treturn (jlong)" + ptr + ";\n";
	}

	string gen_jni_fun(t_metadata_base meta)
	{
		// JNIEXPORT jlong JNICALL Java_MyJavaClass_Function1(args) {
		//	 MyClass* ptr = (MyClass*)lp;
		//	 ...
		// }
		string ptr = "ptr";
		string ret_var = "result";
		string gen;
		gen += "JNIEXPORT ";
		gen += !from->is_type_simple(meta.type) ? "jlong" : mtype_jni[meta.type];
		cout << "\n";
		gen += " JNICALL Java_" + name_class + "__" + meta.name +
				"(JNIEnv *, jobject, jlong " + ptr_class;

		gen += gen_jni_args(meta);
		gen += ")\n";
		gen += "{\n";
		gen += gen_jni_args_convert(meta);
		gen += gen_jni_ptr(name_class, ptr, ptr_class);
		// invoke function
		if (!from->is_type_simple(meta.type)) {
			// MyClass* ptr = (MyClass*)lp;
			// add parsing unique_ptr...
			// add args for constructor...
			string t = from->is_type_ref(meta.type) ? from->get_type_ref(meta.type) : meta.type;
			t = from->is_type_ptr(t) ? from->get_type_ptr(t) : t;
			gen += gen_jni_ptr_new(t, ret_var) + ";\n";
			// gen += gen_jni_ptr_new(meta.type, ret_var) + ";\n";
			
			gen += "\t" + ret_var + " = " + ptr + "->" + meta.name + "(";
			gen += gen_oper();
			gen += ");\n";
			gen += gen_jni_ptr_ret(ret_var);
		} else if (meta.type == "void") {
			gen += "\t" + ptr + "->" + meta.name + "(";
			gen += gen_oper();
			gen += ");\n";
		} else {
			gen += "\treturn " + ptr + "->" + meta.name + "(";
			gen += gen_oper();
			gen += ");\n";
		}
		gen += "}\n\n";
		return gen;
	}

	string gen_jni_end()
	{
		string gen;
		gen += "#ifdef __cplusplus\n";
		gen += "}\n";
		gen += "#endif\n";
		gen += "#endif\n\n";
		return gen;
	}

	void write_file(string file, string gen)
	{
		ofstream stream(file);
		if (stream.good())
			stream << gen;
		stream.close();
	}

	void gen_jni(string folder, string file)
	{
		string gen = gen_jni_header(file);

		for (t_metadata_base m : from->get_meta()) {
			if (m.tm != tm_class)
				continue;
			name_class = m.name;
			bool is_const = false;
			for (t_metadata_base mf : m.sub) {
				cout << "sub type = " << 
					(mf.tm == tm_fun ? "fun" :
					 mf.tm == tm_class ? "class" :
					 "val") << ";\n";
				if (mf.tm != tm_fun)
					continue;
				cout << "try print jni fun;\n";
				if (m.name == mf.type && mf.name == "") {
					// is_const = true;
					// gen += gen_jni_constr(m.name, mf);
				} else
					gen += gen_jni_fun(mf);
			}
			if (!is_const)
				gen += gen_jni_constr(t_metadata_base());
			gen += gen_jni_destroy();
		}

		gen += gen_jni_end();
		write_file(folder + "jni_" + file + ".hpp", gen);
	}

	string gen_java_arg(string type, string name)
	{
		string gen;
		string nm = name.size() ? name : "args";
		names.push_back(nm);
		string t = from->is_type_ref(type) ? from->get_type_ref(type) : type;
		cout << "gen_java_arg: type = " << t << "\n";
		
		if (from->is_type_simple(t))
			gen += t;
		else if (t == "std::string")
			gen += "String";
		else
			gen += "long";

		cout << "gen_java_arg: gen = " << gen << "\n";

		gen += " " + nm;
		return gen;
	}

	string gen_java_args(t_metadata_base meta)
	{
		names.clear();
		string gen;
		int i = 0;
		for (t_metadata_base val : meta.sub) {
			if (val.tm != tm_val)
				continue;
			gen += i++ ? ", " : "";
			gen += gen_java_arg(val.type, val.name);
		}
		return gen;
	}

	string gen_java_native(t_metadata_base meta, bool is_const)
	{
		string gen = "\tnative private ";
		string t = from->is_type_ref(meta.type) ? from->get_type_ref(meta.type) : meta.type;
		gen += !from->is_type_simple(t) ? "long" : t;
		gen += " _" + meta.name + "(";
		string args = gen_java_args(meta);
		if (!is_const) {
			gen += "long _" + ptr_class;
			if (args.size())
				gen += ", ";
		}
		gen += args + ");\n";
		return gen;
	}

	string gen_java_public(t_metadata_base meta)
	{
		string t = from->is_type_ref(meta.type) ? from->get_type_ref(meta.type) : meta.type;
		string gen = "\tpublic " + t + " " + meta.name + "(";
		gen += gen_java_args(meta) + ")\n";
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

	string gen_java_constr_private()
	{
		string gen = "\tprivate " + name_class + "(long _" + ptr_class + ")\n";
		gen += "\t{\n";
		gen += "\t\t" + ptr_class + " = _" + ptr_class + ";\n";
		gen += "\t}\n\n";
		return gen;
	}

	string gen_java_constr_public(t_metadata_base meta)
	{
		string gen = "\tpublic " + name_class + "(\n";
		// args
		gen += ")\n";
		gen += "\t{\n";
		// gen += "\t\t" + ptr_class + " = _" + ptr_class + ";\n";
		gen += "\t}\n";
		return gen;
	}

	string gen_java_fun(t_metadata_base meta)
	{
		string gen = gen_java_native(meta, false);
		gen += gen_java_public(meta);
		return gen;
	}

	string gen_java_head()
	{
		// fix native
		string gen = "public class " + name_class + " {\n";
		gen += "\tprivate long " + ptr_class + ";\n\n";
		gen += "\tnative private long _initClass();\n";
		gen += "\tnative private void _destroyClass(long _" + ptr_class + ");\n\n";

		gen += "\tpublic " + name_class + "()\n";
		gen += "\t{\n";
		gen += "\t\t" + ptr_class + " = _initClass();\n";
		gen += "\t}\n\n";

		gen += "\tpublic void destroy()\n";
		gen += "\t{\n";
		gen += "\t\t_destroyClass(" + ptr_class + ");\n";
		gen += "\t\t" + ptr_class + " = 0L;\n";
		gen += "\t}\n\n";

		gen += "\tprotected void finalize() throws Throwable\n";
		gen += "\t{\n";
		gen += "\t\t_destroyClass(" + ptr_class + ");\n";
		gen += "\t\t" + ptr_class + " = 0L;\n";
		gen += "\t}\n\n";

		return gen;
	}

	string gen_java_end()
	{
		return "}";
	}

	void gen_java(string folder, string file)
	{
		// gen constructr right
		for (t_metadata_base m : from->get_meta()) {
			if (m.tm != tm_class)
				continue;
			name_class = m.name;
			string gen = gen_java_head();
			bool is_const = false;
			for (t_metadata_base mf : m.sub) {
				if (mf.tm != tm_fun)
					continue;
				cout << "try print fun;\n";
				if (m.name == mf.type) {
					if (from->cmp_types(m.name, mf.type))
						gen += gen_java_constr_private();
					is_const = true;
				} else {
					gen += gen_java_fun(mf);
				}
			}
			if (!is_const)
				cout << "need print constructr\n";
			gen += gen_java_end();
			write_file(folder + m.name + ".java", gen);
		}
	}

	string name_class;
	string ptr_class = "handle";

public:
	t_generator()
	{
		init_type_jni();
		init_type_java();
		to = nullptr;
	}
	
	t_generator(type_code rsc)
	{
		init_type_jni();
		init_type_java();
		to = t_rule_factory::create(rsc);
	}

	~t_generator()
	{
		delete to;
	}

	bool is_OK()
	{
		return to != nullptr;
	}

	void run(t_rule *rule, string file)
	{
		from = rule;
		int index = file.find(".");
		index = index == string::npos ? file.size() : index;
		string name = string(file, 0, index);
		string folder = "jni_test/";
		gen_jni(folder, name);
		gen_java(folder, name);
	}
};
