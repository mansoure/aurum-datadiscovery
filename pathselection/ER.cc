#include "ER.h"
#include "Reader.h"
#include "Common.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <unordered_set>

using namespace std;

const double ER::JAC_THRESHOLD = 0.7;

ER::ER(string table_name)
{
	//read original tables
	ifstream fin(table_name.c_str());
	string s;
	str.clear();
	while (getline(fin, s))
		str.push_back(s);
	n = (int) str.size();
}

void ER::run_ER() //currently specifically written for the apt-buy dataset
{
	vector<Rule> rules;
	int iter_id = 1;
	int num_matches_prev = 0;

	while (1)
	{
		cout << "Iteration " << iter_id ++ << " : " << endl;
		vector<pair<int, int>> matching_pairs;
		matching_pairs.clear();
		for (int i = 0; i < n; i ++)
			for (int j = i + 1; j < n; j ++)
			{
				string cell1 = str[i];
				string cell2 = str[j];

				vector<string> gen_1 = Transformation::gen_derived_strings(cell1, rules);
				double max_sim = 0;
				for (string s1 : gen_1)
						max_sim = max(max_sim, Common::jaccard_sim(s1, cell2));
				if (max_sim >= JAC_THRESHOLD)
					matching_pairs.emplace_back(i, j);
			}

		//print number of matchings
		cout << "\t# newly added matchings : " << (int) matching_pairs.size() - num_matches_prev << endl;
		cout << "\t# total matchings : " << matching_pairs.size() << endl;
		if ((int) matching_pairs.size() <= num_matches_prev)
		{
			cout << "No improvement. Quit..." << endl;
			break;
		}
		num_matches_prev = (int) matching_pairs.size();

		//make examples
		vector<pair<string, string>> examples;
		for (int i = 0; i < (int) matching_pairs.size(); i ++)
		{
			int x = matching_pairs[i].first;
			int y = matching_pairs[i].second;
			examples.emplace_back(str[x], str[y]);
		}

		Transformation trans(examples);
		rules = trans.generate_rules(iter_id, 1, false);
		cout << "\tCurrent rules : " << endl;
		for (Rule rule : rules)
			cout << "\t\t" << rule.str << endl;
	}
}
