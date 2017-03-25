#include "Transformation.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
using namespace std;

Rule::Rule(string rule)
{
	//make maps
	int pos = (int) rule.find("_");
	vector<string> tokens1 = Common::get_tokens(rule.substr(0, pos));
	vector<string> tokens2 = Common::get_tokens(rule.substr(pos + 1));
	for (string t : tokens1)
		if (t != Transformation::NONE)
			map1[t] ++;
	for (string t : tokens2)
		if (t != Transformation::NONE)
			map2[t] ++;

	//make str
	str = "";
	for (int i = 0; i < (int) rule.size(); i ++)
		if (rule[i] == '_')
			str += " -> ";
		else
			str += rule[i];
}

const int Transformation::DEFAULT_NUM_RULES = 50;
const int Transformation::DEFAULT_MAX_NUM_TOKEN = 1;
const int Transformation::MAX_CELL_LEN = 10;
const string Transformation::NONE = "NONE";

vector<string> Transformation::gen_derived_strings(string s, vector<Rule> rules)
{
	//answer set
	unordered_set<string> ans_set;
	vector<string> ans;

	//token map
	vector<string> tokens = Common::get_tokens(s);
	umpsi token_map;
	for (string token : tokens)
		token_map[token] ++;

	//find applicable rules
	vector<int> applicable_rules;
	for (int i = 0; i < (int) rules.size(); i ++)
	{
		Rule &rule = rules[i];
		bool applicable = true;
		for (auto it : rule.map1)
			if (! token_map.count(it.first) || token_map[it.first] < it.second)
			{
				applicable = false;
				break;
			}
		if (applicable)
			applicable_rules.push_back(i);
	}

	//derive strings
	int M = (1 << (int) (applicable_rules.size()));
	for (int mask = 0; mask < M; mask ++)
	{
		umpsi cur_map = token_map;
		for (int i = 0; i < (int) applicable_rules.size(); i ++)
		{
			if(! (mask & (1 << i)))
				continue;

			Rule &rule = rules[applicable_rules[i]];
			bool applicable = true;
			for (auto it : rule.map1)
				if (! cur_map.count(it.first) || cur_map[it.first] < it.second)
				{
					applicable = false;
					break;
				}
			if (applicable)
			{
				for (auto it : rule.map1)
					cur_map[it.first] -= it.second;
				for (auto it : rule.map2)
					cur_map[it.first] += it.second;
			}
		}

		string cur_string = "";
		for (auto it : cur_map)
			for (int i = 0; i < it.second; i ++)
			{
				if (cur_string.size())
					cur_string += " ";
				cur_string += it.first;
			}
		ans_set.insert(cur_string);
	}

	ans = vector<string>(ans_set.begin(), ans_set.end());
	return ans;
}

Transformation::Transformation(vector<pair<string, string>> _matching_pairs)
{
	matching_pairs.clear();
	for (auto p : _matching_pairs)
	{
		if ((int) Common::get_tokens(p.first).size() > MAX_CELL_LEN ||
			(int) Common::get_tokens(p.second).size() > MAX_CELL_LEN)
			continue;
		matching_pairs.emplace_back(p.first, p.second);
	}
}

Transformation::Transformation(string file_name)
{
	ifstream fin(file_name.c_str());
	matching_pairs.clear();
	string s, s1, s2;
	while (getline(fin, s))
	{
		getline(fin, s1);
		getline(fin, s2);

		if ((int) Common::get_tokens(s1).size() > MAX_CELL_LEN ||
			(int) Common::get_tokens(s2).size() > MAX_CELL_LEN)
			continue;

		matching_pairs.emplace_back(s1, s2);
	}
	fin.close();
}

void Transformation::remove_zero_elements_in_map(umpsi &mp)
{
	for (auto it = mp.begin(); it != mp.end(); )
		if (it->second == 0)
			it = mp.erase(it);
		else
			it ++;
}

vector<string> Transformation::gen_patterns(umpsi mp, int max_num_token)
{
	//sort all tokens
	vector<string> tokens;
	for (auto it : mp)
		for (int i = 1; i <= it.second; i ++)
			tokens.push_back(it.first);
	sort(tokens.begin(), tokens.end());

	//brute-force using next_permutation()
	vector<string> ans;
	for (int len = 1; len <= max_num_token; len ++)
	{
		if (len > (int) tokens.size())
			break;
		//initialize permutation
		vector<int> comb;
		for (int i = 1; i <= len; i ++)
			comb.push_back(i - 1);

		while (1)
		{
			string cur = "";
			for (int i = 0; i < len; i ++)
			{
				if (i)
					cur += " ";
				cur += tokens[comb[i]];
			}
			ans.push_back(cur);
			if (! next_combination(comb, (int) tokens.size() - 1))
				break;
		}
	}
	return ans;
}

int Transformation::get_token_count(string s)
{
	int num_token = 0;
	for (int i = 0; i < (int) s.size(); i ++)
		if (s[i] == '_' || s[i] == ' ')
			num_token ++;
	if (s.find(NONE) != string::npos)
		num_token --;

	return num_token + 1;
}

bool Transformation::next_combination(vector<int> &comb, int lim)
{
	int first_good_guy = -1;
	int num_token = (int) comb.size();
	for (int i = num_token - 1; i >= 0; i --)
		if (comb[i] < lim - (num_token - i - 1))
		{
			first_good_guy = i;
			break;
		}
	if (first_good_guy < 0)
		return false;
	comb[first_good_guy] ++;
	for (int i = first_good_guy + 1; i < num_token; i ++)
		comb[i] = comb[i - 1] + 1;

	return true;
}

vector<Rule> Transformation::generate_rules(int num_rules, int max_num_token, bool print)
{
	int num_pairs = (int) matching_pairs.size();

	//make token sets pairs
	for (int i = 0; i < num_pairs; i ++)
	{
		vector<string> token_set;
		umpsi mp1, mp2;

		token_set = Common::get_tokens(matching_pairs[i].first);
		mp1.clear();
		for (string token : token_set)
			mp1[token] ++;

		token_set = Common::get_tokens(matching_pairs[i].second);
		mp2.clear();
		for (string token : token_set)
			mp2[token] ++;

		token_set_pairs.emplace_back(mp1, mp2);
	}

	//identity rules
	for (int i = 0; i < num_pairs; i ++)
	{
		umpsi &mp1 = token_set_pairs[i].first;
		umpsi &mp2 = token_set_pairs[i].second;
		for (auto &p : mp1)
			while (mp1[p.first] && mp2.count(p.first) && mp2[p.first])
			{
				mp1[p.first] --;
				mp2[p.first] --;
			}
		remove_zero_elements_in_map(mp1);
		remove_zero_elements_in_map(mp2);
	}

	//greedy algorithm
	vector<Rule> rules;
	for (int k = 1; k <= num_rules; k ++)
	{
		//calculating the hashmap
		umpsi support_map;
		for (int i = 0; i < num_pairs; i ++)
		{
			vector<string> pattern1 = gen_patterns(token_set_pairs[i].first, max_num_token);
			vector<string> pattern2 = gen_patterns(token_set_pairs[i].second, max_num_token);
/*			for (string p : pattern1)
				support_map[p + '_' + NONE] ++;
			for (string p : pattern2)
				support_map[NONE + '_' + p] ++;
*/			for (string p1 : pattern1)
				for (string p2 : pattern2)
					support_map[p1 + '_' + p2] ++;
		}

		//find the maximum one
		string best_pattern = "";
		int support_best_pattern = -1;
		for (auto p : support_map)
			if (p.second > support_best_pattern)
			{
				support_best_pattern = p.second;
				best_pattern = p.first;
			}
			else if (p.second == support_best_pattern)
				if (get_token_count(p.first) > get_token_count(best_pattern))
					best_pattern = p.first;

		//get rule, print it
		Rule rule(best_pattern);
		rules.push_back(rule);
		if (print)
		{
			cout << rule.str << endl;
			cout << " : support = " << support_best_pattern << endl;
		}

		//update and output
		for (int i = 0; i < num_pairs; i ++)
		{
			//check whether can apply the best_pattern to this pair
			bool can_apply = true;
			for (auto it : rule.map1)
				if (! token_set_pairs[i].first.count(it.first) || it.second > token_set_pairs[i].first[it.first])
				{
					can_apply = false;
					break;
				}
			for (auto it : rule.map2)
				if (! token_set_pairs[i].second.count(it.first) || it.second > token_set_pairs[i].second[it.first])
				{
					can_apply = false;
					break;
				}

			if (! can_apply)
				continue;

			//print
			if (print)
			{
				cout << endl;
				cout << '\t' << matching_pairs[i].first << endl;
				cout << '\t' << matching_pairs[i].second << endl;
				cout << endl;
			}

			//update token_set_pairs
			for (auto it : rule.map1)
				token_set_pairs[i].first[it.first] -= it.second;
			for (auto it : rule.map2)
				token_set_pairs[i].second[it.first] -= it.second;
		}
	}

	return rules;
}
