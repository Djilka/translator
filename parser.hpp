#pragma once
#include "info.hpp"

class t_parser {
	t_info info;
private:
	vector<string> split(string str)
	{
		vector<string> res;
		int ps = 0, pc = 0;
		int count = 0;
		while (pc < str.size()) {
			if (str[pc] == '<' || str[pc] == '>') {
				count += str[pc] == '<' ? 1 : -1;
			} else if (!count) {
				if (str[pc] == ':' && pc + 1 < str.size() && str[pc+1] == ':') {
					pc += 2;
					continue;
				}
				string s(str, pc, 1);
				if (!isalnum(str[pc]) && info.is_sep(s)) {
					if (ps != pc)
						res.push_back(string(str, ps, pc - ps));
					res.push_back(s);
					ps = pc + 1;
				}
			}
			pc++;
		}
		if (ps != pc)
			res.push_back(string(str, ps, pc - ps + 1));
		return res;
	}

	vector<string> read(string name)
	{
		vector<string> v;
		ifstream stream(name);
		string tmp;

		while (stream.good()) {
			tmp.clear();
			stream >> tmp;
			if (tmp == "//" || tmp[0] == '#') {
				char tmp_c[256];
				stream.getline(tmp_c, 256, '\n');
				continue;
			}
			if (tmp.size() == 1)
				v.push_back(tmp);
			else if (tmp.size()) {
				vector<string> v_tmp = split(tmp);
				v.insert(v.end(), v_tmp.begin(), v_tmp.end());
			}
		}

		stream.close();
		return v;
	}
public:
	vector<string> run(string name)
	{
		return read(name);
	}

	
};
