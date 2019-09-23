#pragma once
#include "parser.hpp"
#include "generator.hpp"

class t_translator {
public:
	void run(vector<string> files)
	{
		t_parser parser;
		for (string file : files) {
			t_language_cpp language;
			if (!language.run(parser.run(file))) {
				cout << "ERROR: semantic\n";
				continue;
			}
			t_generator_jni gen(language.get_info());
			t_generator_java genj(language.get_info());
			tv_meta meta = language.get_meta();
			gen.run(meta, file);
			genj.run(meta, file);
			t_manager_meta::destroy(meta);
		}
	}
};
