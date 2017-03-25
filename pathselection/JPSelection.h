#ifndef JPSELECTION_H
#define JPSELECTION_H

#include "Corpus.h"
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

struct JP
{
	vector<int> tables;
	vector<pair<int, int>> jpreds, proj_cols;
};

struct Cell
{
	int tid, x, y;

	Cell() {}
	Cell(int _tid, int _x, int _y) :
		tid(_tid), x(_x), y(_y)
	{}

	bool operator == (const Cell &o) const
	{
		return tid == o.tid && x == o.x && y == o.y;
	}

	bool operator < (const Cell &o) const
	{
		return tid < o.tid || (tid == o.tid &&  x == o.x)
			|| (tid == o.tid && x == o.x && y < o.y);
	}
};

typedef vector<Cell> lineage;

namespace std {

	template <>
	struct hash<Cell>
	{
		std::size_t operator()(const Cell& k) const
		{
			using std::size_t;
			using std::hash;
			using std::string;

			return ((hash<int>()(k.tid)
					 ^ (hash<int>()(k.x) << 1)) >> 1)
			^ (hash<int>()(k.y) << 1);
		}
	};

}


class JPSelection
{
public:
	JPSelection() {}
	JPSelection(Corpus *, string, string, string);
	string select_join_path(int K = 1);
  string budget_clean(int B, int jp_idx);

private:
	Corpus *corpus;
	unordered_map<string, int> tname2tid;
	vector<vector<vector<double>>> cleanliness_scores;
	vector<vector<vector<bool>>> is_error;
	vector<JP> jps;
	int b;

	//functions
	vector<double> calculate_num_clean_values(JP, vector<vector<lineage>> &, vector<vector<double>> &, vector<vector<vector<bool>>> &, vector<vector<vector<bool>>> &);
  string show_result_table(JP cur_jp, vector<vector<string>> &val, int limit);
	bool isL1(JP, Cell);
	void enlarge_tables();
};

#endif
