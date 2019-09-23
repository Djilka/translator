#include "gen.hpp"
#include "parser_b.hpp"

void test_value()
{
	cout << "test_value\n";
	vector<string> v = {"int", "name", ";"};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_function0()
{
	cout << "test_function0\n";
	vector<string> v = {"int", "name", "(", ")", ";"};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_function1()
{
	cout << "test_function1\n";
	vector<string> v = {"int", "name", "(", "int", ")", ";"};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_function2()
{
	cout << "test_function2\n";
	vector<string> v = {"int", "name", "(", "int", "name", ")", ";"};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_class0()
{
	cout << "test_class0\n";
	vector<string> v = {"class", "name", "{", "}", ";"};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_class1()
{
	cout << "test_class1\n";
	vector<string> v = {
		"class", "name", ":", "public", "int",
		"{", "}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_class2()
{
	cout << "test_class2\n";
	vector<string> v = {
		"class", "name", "{",
		"int", "name", ";",
		"}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_class3()
{
	cout << "test_class3\n";
	vector<string> v = {
		"class", "name", ":", "public", "int",
		"{",
		"int", "name", "(", ")", ";",
		"}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_class4()
{
	cout << "test_class4\n";
	vector<string> v = {
		"class", "name", ":", "public", "int",
		"{",
		"int", "name", "(", ")", ";",
		"using", "name", "=", "int", ";",
		"}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_class5()
{
	cout << "test_class5\n";
	vector<string> v = {
		"class", "name", ":", "public", "int",
		"{",
		"static", "float", "name", "(", "int", "fff", ")", ";",
		"using", "name", "=", "int", ";",
		"}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_class6()
{
	cout << "test_class6\n";
	vector<string> v = {
		"class", "name", ":", "public", "int",
		"{",
		"static", "const", "float", "name", "(", "int", "fff", ")", ";",
		"using", "name", "=", "int", ";",
		"}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_manager_meta::destroy(meta);
}

void test_gen0()
{
	cout << "test_gen0\n";
	vector<string> v = {
		"class", "Test",
		"{",
		"Test", "fun", "(", "int", "fff", ")", ";",
		"}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_generator_jni gen(language.get_info());
	gen.run(meta, "test.hpp");
	t_manager_meta::destroy(meta);
}

void test_gen1()
{
	cout << "test_gen1\n";
	vector<string> v = {
		"class", "Test",
		"{",
		"float&", "fun", "(", "int", "fff", ")", ";",
		"void", "pointer_fun", "(", "int*", "yyy", ")", ";",
		"void", "ref_fun", "(", "std::string&", "str", ")", ";",
		"Test", "move_fun", "(", "Test&&", "iptr", ")", ";",
		"}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_generator_jni gen(language.get_info());
	gen.run(meta, "test.hpp");
	t_manager_meta::destroy(meta);
}

void test_gen3()
{
	cout << "test_gen3\n";
	vector<string> v = {
		"class", "Test",
		"{",
		"void", "fun", "(", "std::unique_ptr<int>", "fff", ")", ";",
		"}", ";"
	};

	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_generator_jni gen(language.get_info());
	gen.run(meta, "test.hpp");
	t_manager_meta::destroy(meta);
}

int main()
{
	bool all_test = false;
	if (all_test) {
		test_value();
		test_function0();
		test_function1();
		test_function2();
		test_class0();
		test_class1();
		test_class2();
		test_class3();
		test_class4();
		test_class5();
		test_class6();
	}
	// test_gen0();
	// test_gen1();
	// bad
	// test_gen3();
	t_parser parser;
	vector<string> v = parser.run("example.h");
	// for (string str : v)
	// 	cout << "str = " << str << "\n";
	t_language_cpp language;
	bool fl = language.run(v);
	cout << "parse = " << fl << "\n";
	tv_meta meta = language.get_meta();
	cout << "meta = " << meta.size() << "\n";
	t_generator_jni gen(language.get_info());
	t_generator_java genj(language.get_info());
	gen.run(meta, "test.hpp");
	genj.run(meta, "test.hpp");
	t_manager_meta::destroy(meta);
}