#include "header.hpp"
// #include "code_gen.hpp"
// #include "parser.hpp"
#include "translator.hpp"

int main(int argc, char const *argv[])
{
	if (argc > 1) {
		// get name file -> parce -> create jni
		// t_parser parser;
		// for (int i = 1; i < argc; i++)
		// 	parser.run(argv[i]);
		vector<string> files;
		for (int i = 1; i < argc; i++)
			files.push_back(argv[i]);
		t_translator tr(tc_cpp, tc_java);
		tr.run(files);
	}
	return 0;
}
