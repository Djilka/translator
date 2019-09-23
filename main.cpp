#include "translator.hpp"

int main(int argc, char const *argv[])
{
	if (argc > 1) {
		vector<string> files;
		for (int i = 1; i < argc; i++)
			files.push_back(argv[i]);
		t_translator tr;
		tr.run(files);
	}
	return 0;
}
