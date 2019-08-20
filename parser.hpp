
class t_parser {
private:
	t_rule *rule;

	vector<string> read(string name)
	{
		vector<string> v;
		ifstream stream(name);
		string tmp;
		cout << "t_parser::read\n";

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
				vector<string> v_tmp = rule->split(tmp);
				v.insert(v.end(), v_tmp.begin(), v_tmp.end());
			}
		}

		stream.close();
		return v;
	}
public:
	t_parser(type_code type)
	{
		rule = t_rule_factory::create(type);
	}

	~t_parser()
	{
		delete rule;
	}

	bool is_OK()
	{
		return rule != nullptr;
	}

	void meta(vector<string> v)
	{
		cout << "vector:\n";
		for (string t : v)
			cout << "v_tmp = " << t << "\n";
		cout << "\n";

		int i = 0, tmp = 0;
		while (i < v.size()) {
			cout << "metas name class: " << v[i+1] << "\n";
			if (rule->try_val(v, i) || rule->get_st_size("val")) {
				cout << "find val\n";
			} else if (rule->try_fun(v, i) || rule->get_st_size("fun")) {
				cout << "find fun\n";
			} else if (rule->try_class(v, i) || rule->get_st_size("class")) {
				cout << "find class\n";
			} else {
				cout << "didn't find type\n";
				i++;
			}
			cout << "\n";
		}
	}

	t_rule* run(string name)
	{
		cout << "t_parser::run1 name = " << name << "\n";
		meta(read(name));
		return rule;
	}
};
