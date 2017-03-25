#include "ER.h"
#include "Corpus.h"
#include "ErrorModel.h"
#include "Transformation.h"
#include "PkfkGraph.h"
#include "JPSelection.h"
#include <ctime>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include <unordered_map>
#include <unordered_set>
using namespace std;

int main()
{
/*
//	Corpus *corpus = new MITDWH("../../mitdata/");
//	Corpus *corpus = new MITDWH("../../debug/");
//	cout << corpus->count_table() << endl;
//	cout << corpus->count_multi_column_table() << endl;

	Corpus *corpus = new Corpus("../../datasets/citation_exp/");
	ErrorModel *error_model = new PKErrorModel(corpus);
	error_model->find_erroneous_cells();

	Transformation *trans = new Transformation("example.txt");
	trans->generate_rules();

	ER *er = new ER("textfiles/dept_names.txt");
	er->run_ER();
*/

/*	Corpus *corpus = new Corpus("../../datasets/citation/");
	ofstream fout("fastjoin_input.txt");
	int num_table = corpus->count_table();
	for (int i = 0; i < num_table; i ++)
	{
		Table *cur_table = corpus->get_table(i);
		if (cur_table->name != "citations.csv")
			continue;

		vector<vector<string>> &cells = cur_table->cells;
		cout << cells.size() << " " << cells[0].size() << endl;
		for (int x = 0; x < (int) cells.size(); x ++)
			for (int y = 0; y < (int) cells[0].size(); y ++)
				fout << cells[x][y] << endl;
	}
*/

/*
	PkfkGraph *graph = new PkfkGraph("Data_mit_pkfks.txt");
	graph->print_join_paths();
*/

/*
	//transform the fd file from dong deng
	Corpus *corpus = new Corpus("/Users/wenbo/Documents/wenbo/MIT/DataCivilizer/mitdwh_exp/");
	ifstream fin("fd_bad.txt");
	ofstream fout("fd_good.txt");
	string s;
	while (getline(fin, s))
	{
		string table_name, fd_str, c1_str, c2_str;
		int tab_pos = (int) s.find("\t");
		table_name = s.substr(0, tab_pos);
		s.erase(0, tab_pos + 1);

		tab_pos = (int) s.find("\t");
		fd_str = s.substr(tab_pos + 1);

		int arrow_pos = (int) fd_str.find("=>");
		c1_str = fd_str.substr(0, arrow_pos);
		c2_str = fd_str.substr(arrow_pos + 2);

		for (int i = 0; i < (int) c1_str.size(); i ++)
			if (isupper(c1_str[i]))
				c1_str[i] += 32;
		for (int i = 0; i < (int) c2_str.size(); i ++)
			if (isupper(c2_str[i]))
				c2_str[i] += 32;

		//search for table in corpus
		int num_table = corpus->count_table();
		for (int i = 0; i < num_table; i ++)
		{
			Table *cur_table = corpus->get_table(i);
			if (cur_table->name != table_name)
				continue;

			int c1 = -1, c2 = -1;
			for (int i = 0; i < cur_table->num_col; i ++)
			{
				if (cur_table->headers[i] == c1_str)
					c1 = i;
				if (cur_table->headers[i] == c2_str)
					c2 = i;
			}
			fout << table_name << " " << c1 << " " << c2 << endl << c1_str << " => " << c2_str << endl << endl;
		}
	}
	fin.close();
	fout.close();
*/

/*
	Corpus *corpus = new Corpus("../../datasets/mitdwh_exp/");

	ErrorModel *error_model = new FDErrorModel(corpus, "fd_good_more.txt");
	error_model->find_erroneous_cells();
*/
	//running sigmod exps
	string corpus_paths[] = {"../../datasets/mitdwh_q1/", "../../datasets/mitdwh_q2/", "../../datasets/mitdwh_q3/", "../../datasets/citation_exp/"};
	string est_paths[] = {"../../datasets/gt.in", "error_est.in", "gt.in" ,"celltruth.csv"};
	string gt_paths[] = {"../../datasets/error_est.in", "gt.in", "gt.in", "celltruth.csv"};
	string jp_paths[] = {"../../datasets/q1.in", "q2.in", "q3.in", "q4.in"};
	int Bs[] = {1000, 20, 10, 40};

	for (int i = 0; i < 4; i ++)
	{
		cout << endl << endl << "Query " << i << " : " << endl;
		Corpus *corpus = new Corpus(corpus_paths[i]);
		JPSelection *jps = new JPSelection(corpus, est_paths[i], gt_paths[i], jp_paths[i]);

		for (int k = 1; k <= 1; k ++)
		{
			struct timeval t1, t2;
			gettimeofday(&t1, NULL);
			cout << jps->select_join_path() << endl;
      cout << jps->budget_clean(Bs[i], 0) << endl;
			gettimeofday(&t2, NULL);
			double totalTime = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) / 1000000.0;

			cout << "Query " << i << " " << k << " times larger : " << totalTime << "s." << endl;
		}
		delete jps;
	}
/*
	//output all department names
	umpsi dept_names;
	Corpus *corpus = new Corpus("../../datasets/mitdata/");
	int num_table = corpus->count_table();
	for (int k = 0; k < num_table; k ++)
	{
		Table *cur_table = corpus->get_table(k);
		for (int j = 0; j < cur_table->num_col; j ++)
		{
			string header = cur_table->headers[j];
			if (header.find("name") == string::npos && header.find("title") == string::npos)
				continue;
			if (header.find("org") == string::npos && header.find("department") == string::npos)
				continue;
			for (int i = 0; i < cur_table->num_row; i ++)
				dept_names[cur_table->cells[i][j]] ++;
		}
	}

	for (auto cp : dept_names)
		if (cp.first != "")
			cout << cp.second << endl;
*/
/*
	//output all course names
	umpsi course_names;
	Corpus *corpus = new Corpus("../../datasets/mitdata/");
	int num_table = corpus->count_table();
	for (int k = 0; k < num_table; k ++)
	{
		Table *cur_table = corpus->get_table(k);
		for (int j = 0; j < cur_table->num_col; j ++)
		{
			string header = cur_table->headers[j];
			if (header.find("name") == string::npos && header.find("title") == string::npos)
				continue;
			if (header.find("course") == string::npos && header.find("subject") == string::npos)
				continue;
			for (int i = 0; i < cur_table->num_row; i ++)
				course_names[cur_table->cells[i][j]] ++;
		}
	}

	for (auto cp : course_names)
		if (cp.first != "")
			cout << cp.second << endl;
	//process data.gov data
	Corpus *corpus = new Corpus("/data/dongdeng/govdata/saved/");
	int num_table = corpus->count_table();
	unordered_map<string, int> area_names;
	for (int k = 0; k < num_table; k ++)
	{
		Table *cur_table = corpus->get_table(k);
		for (int j = 0; j < cur_table->num_col; j ++)
		{
			bool has_value = false;
			for (int i = 0; i < cur_table->num_row; i ++)
				if (cur_table->cells[i][j] == "sprngfld gdns" || cur_table->cells[i][j] == "springfield gardens")
				{
					has_value = true;
					break;
				}
			if (has_value)
				for (int i = 0; i < cur_table->num_row; i ++)
					area_names[cur_table->cells[i][j]] ++;
		}
	}

	ofstream fout1("area_names.txt");
	ofstream fout2("area_names_weights.txt");
	for (auto cp : area_names)
		if (cp.first != "")
			fout1 << cp.first << endl, fout2 << cp.second << endl;
  */

	return 0;
}
