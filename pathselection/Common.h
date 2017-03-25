#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>
#include <unordered_map>
using namespace std;
typedef unordered_map<string, int> umpsi;

class Common
{
public:

	static vector<string> get_tokens(string);
	static double jaccard_sim(string, string);
};

#endif // COMMON_H
