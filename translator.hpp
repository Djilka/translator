#include "rule.hpp"
#include "parser.hpp"
#include "generator.hpp"

class t_translator {
	type_code from, to;
public:
	t_translator(type_code src, type_code rsc)
	{
		from = src;
		to = rsc;
	}

	void set(type_code src, type_code rsc)
	{
		from = src;
		to = rsc;
	}

	void run(vector<string> files)
	{
		t_parser parser(from);
		t_generator generator(to);
		if (!parser.is_OK() || !generator.is_OK())
			return;

		for (string file : files)
			generator.run(parser.run(file), file);
	}
};
