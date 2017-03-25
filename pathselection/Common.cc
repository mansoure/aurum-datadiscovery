#include "Common.h"
#include <cctype>
using namespace std;

vector<string> Common::get_tokens(string s)
{
	s += ' ';
	vector<string> ans;
	string ss = "";
	for (int i = 0; i < (int) s.size(); i ++)
		if (! isalpha(s[i]) && ! isdigit(s[i]))
		{
			if (ss.size())
				ans.push_back(ss);
			ss = "";
		}
		else
			ss += s[i];
	return ans;
}

double Common::jaccard_sim(string x, string y)
{
	if (x.find_first_not_of(' ') == std::string::npos)
		return 0;
	if (y.find_first_not_of(' ') == std::string::npos)
		return 0;

	vector<string> tokens_x = Common::get_tokens(x);
	vector<string> tokens_y = Common::get_tokens(y);

	//calculate intersection
	umpsi token_map_x;
	for (string token : tokens_x)
		token_map_x[token] ++;

	int num_intersection = 0;
	for (string token : tokens_y)
		if (token_map_x.count(token) && token_map_x[token] > 0)
			num_intersection ++, token_map_x[token] --;

	return (double) num_intersection / (tokens_x.size() + tokens_y.size() - num_intersection);
}
