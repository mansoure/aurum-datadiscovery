#ifndef CORPUS__H
#define CORPUS__H

#include <map>
#include <vector>
#include <string>
using namespace std;

struct Stats
{
	bool numeric;
	double mean, var, minn, maxx, range;
	int num;
};

struct Table
{
	Table() {cells.clear(); id = -1;}
	Table(int, string, vector<string>, vector<vector<string>>);

	int id;
	string name;
	vector<string> headers;
	vector<vector<string>> cells;
	int num_row, num_col, primary_key_column;
	vector<Stats> stats_info;

	int operator < (const Table &o) const
	{
		return id < o.id;
	}
};

class Corpus
{
public:
	Corpus();
    Corpus(string);
	int count_multi_column_table();

	//public auxiliary functions
	int count_table();
	Table *get_table(int);		//by table id

protected:
	vector<Table> tables;
};


#endif
