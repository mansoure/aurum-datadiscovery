#include "Corpus.h"
#include "Reader.h"
#include <dirent.h>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#define sqr(x) ((x) * (x))
using namespace std;

Table::Table(int _id, string _name, vector<string> _headers, vector<vector<string>> _cells):
	id(_id), name(_name), headers(_headers), cells(_cells)
{
	num_col = (int) headers.size();
	num_row = (int) cells.size();
/*
	//find primary key - the left most column that has unique values
	primary_key_column = -1;
	int smallest_num_duplicates = num_row;
	for (int j = 0; j < num_col; j ++)
	{
		unordered_set<string> values;
		int num_duplicates = 0;
		for (int i = 0; i < num_row; i ++)
			if (values.count(cells[i][j]))
				num_duplicates ++;
			else
				values.insert(cells[i][j]);
		if (num_duplicates == 0)
		{
			smallest_num_duplicates = 0;
			primary_key_column = j;
			break;
		}
		if (num_duplicates < smallest_num_duplicates)
		{
			smallest_num_duplicates = num_duplicates;
			primary_key_column = j;
		}
	}
	if (smallest_num_duplicates)
		cerr << name << " dose not have a primary key." << endl;

	//determine for each column whether it's numeric or string
	stats_info.clear(), stats_info.resize(num_col);
	for (int j = 0; j < num_col; j ++)
	{
		Stats &s = stats_info[j];
		s.mean = 0;
		s.var = 0;
		s.numeric = true;
		s.minn = 1e30;
		s.maxx = -1e30;
		s.num = 0;
		for (int i = 0; i < num_row; i ++)
		{
			for (int k = 0; k < (int) cells[i][j].size(); k ++)
				if (! isdigit(cells[i][j][k]) && cells[i][j][k] != '.' && cells[i][j][k] != '-')
				{
					s.numeric = false;
					break;
				}
			if (! s.numeric)
				break;
			if (cells[i][j].find_first_not_of(' ') != std::string::npos)
			{
				double cur_number = atoi(cells[i][j].c_str());
				s.num ++;
				s.mean += cur_number;
				s.var += sqr(cur_number);
				s.minn = min(s.minn, cur_number);
				s.maxx = max(s.maxx, cur_number);
			}
		}
		if (s.num == 0)
			s.numeric = false;
		if (s.numeric)
		{
			s.mean /= s.num;;
			s.var = s.var / s.num - sqr(s.mean);
			s.range = s.maxx - s.minn;
		}
	}
*/
}

Corpus::Corpus()
{
	tables.clear();
}

Corpus::Corpus(string directory_name)
{
	Reader *reader = new CSVReader();
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(directory_name.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			Table table = reader->read_a_table(directory_name, ent->d_name);
			if (table.id < 0)
				continue;
			table.id = (int) tables.size();
			tables.push_back(table);
			cerr << tables.size() << endl;
		}

		closedir(dir);
		delete reader;
	}
	else
		perror("could not open catalog directory for indexing");
}

int Corpus::count_multi_column_table()
{
	int ans = 0;
	for (Table t : tables)
		if (t.num_col > 6)
			ans ++;
	return ans;
}

int Corpus::count_table()
{
	return (int) tables.size();
}

Table *Corpus::get_table(int id)
{
	return &tables[id];
}
