#ifndef TRANS_H
#define TRANS_H
#include "Common.h"
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
using namespace std;

struct Rule
{
	Rule() {map1.clear(), map2.clear(), str = "";}
	Rule(string);

	umpsi map1, map2;
	string str;
};

class Transformation
{
public:
	//constructors
	Transformation() {}
	Transformation(vector<pair<string, string>>);
	Transformation(string);

	//main functions
	vector<Rule> generate_rules(int num_rules = DEFAULT_NUM_RULES,
								int max_num_token = DEFAULT_MAX_NUM_TOKEN,
								bool print = false);

	static vector<string> gen_derived_strings(string, vector<Rule>);

	//constants
	const static int DEFAULT_NUM_RULES;
	const static int DEFAULT_MAX_NUM_TOKEN;
	const static int MAX_CELL_LEN;
	const static string NONE;

protected:

	//functions
	vector<string> gen_patterns(umpsi, int);
	void remove_zero_elements_in_map(umpsi &);
	int get_token_count(string s);
	bool next_combination(vector<int> &, int);

	//input matching pairs
	vector<pair<string, string>> matching_pairs;

	//token sets
	vector<pair<umpsi, umpsi>> token_set_pairs;

	//output rules
	vector<string> rules;
};

#endif // TRANS_H
