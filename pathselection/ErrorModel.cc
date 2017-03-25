#include "Common.h"
#include "ErrorModel.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <algorithm>
#define sqr(x) ((x) * (x))
typedef vector<vector<pair<string, pair<int, pair<int, int>>>>> cell_list;
typedef vector<pair<string, pair<int, pair<int, int>>>> cell_list_entry;
using namespace std;

ErrorModel::ErrorModel(Corpus *_corpus) : corpus(_corpus) {matching_pairs.clear();}

vector<pair<string, string>> ErrorModel::get_matching_pairs()
{
	return matching_pairs;
}

bool ErrorModel::find_match(int x)
{
	if (visitedX[x])
		return false;
	visitedX[x] = true;

	for (int y : adj[x])
		if (matchedY[y] < 0 || find_match(matchedY[y]))
		{
			matchedY[y] = x;
			matchedX[x] = y;
			return true;
		}
	return false;
}

double ErrorModel::column_sim(Table *tx, int cx, Table *ty, int cy)
{
	if (tx->stats_info[cx].numeric != ty->stats_info[cy].numeric)
		return 0;

	int num_rowx = tx->num_row;
	int num_rowy = ty->num_row;
	int num_matches = 0;

	//initialize matches
	matchedX.clear(), matchedY.clear();
	matchedX.resize(num_rowx);
	matchedY.resize(num_rowy);
	for (int i = 0; i < num_rowx; i ++)
		matchedX[i] = -1;
	for (int i = 0; i < num_rowy; i ++)
		matchedY[i] = -1;

	//numeric
	if (tx->stats_info[cx].numeric)
	{
		Stats sx = tx->stats_info[cx];
		Stats sy = ty->stats_info[cy];

		//generate match
		vector<pair<double, int>> np1, np2;
		for (int i = 0; i < num_rowx; i ++)
		{
			string cur_cell = tx->cells[i][cx];
			if (cur_cell.find_first_not_of(' ') != std::string::npos)
				np1.emplace_back(atoi(cur_cell.c_str()), i);
		}

		for (int i = 0; i < num_rowy; i ++)
		{
			string cur_cell = ty->cells[i][cy];
			if (cur_cell.find_first_not_of(' ') != std::string::npos)
				np2.emplace_back(atoi(cur_cell.c_str()), i);
		}

		sort(np1.begin(), np1.end());
		sort(np2.begin(), np2.end());

		//use max matching
		for (int i = 0; i < (int) np1.size(); i ++)
		{
			if (i >= (int) np2.size())
				break;

			double diff = fabs(np1[i].first - np2[i].first);
			if (diff / min(sx.range, sy.range) <= diff_threshold)
			{
				num_matches ++;
				matchedX[np1[i].second] = np2[i].second;
				matchedY[np2[i].second] = np1[i].second;
			}
		}
	}
	//string
	else
	{
		//initializing variables
		adj.clear(), removedX.clear(), removedY.clear(), ys.clear();
		adj.resize(num_rowx);
		removedX.resize(num_rowx);
		removedY.resize(num_rowy);

		//find exact matches
		for (int i = 0; i < num_rowy; i ++)
			ys[ty->cells[i][cy]].push_back(i);
		for (int i = 0; i < num_rowx; i ++)
			if (ys.count(tx->cells[i][cx]))
				for (int y : ys[tx->cells[i][cx]])
					if (! removedY[y])
					{
						removedX[i] = true;
						removedY[y] = true;
						matchedX[i] = y;
						matchedY[y] = i;
						num_matches ++;
						break;
					}

		//make edges
		for (int i = 0; i < num_rowx; i ++)
		{
			if (removedX[i])
				continue;
			for (int j = 0; j < num_rowy; j ++)
			{
				if (removedY[i])
					continue;
				if (Common::jaccard_sim(tx->cells[i][cx], ty->cells[j][cy]) >= edge_threshold)
					adj[i].push_back(j);
			}
		}

		//hungarian algorithm
		for (int i = 0; i < num_rowx; i ++)
		{
			if (removedX[i])
				continue;
			visitedX.clear(), visitedX.resize(num_rowx);
			if (find_match(i))
				num_matches ++;
		}
	}

	return (double) num_matches * 2 / (num_rowx + num_rowy);
}

bool ErrorModel::column_sim_exact(Table *tx, int cx, Table *ty, int cy)
{
	if (tx->stats_info[cx].numeric != ty->stats_info[cy].numeric)
		return false;

	int num_rowx = tx->num_row;
	int num_rowy = ty->num_row;
	int intersection = 0;

	//numeric
	if (tx->stats_info[cx].numeric)
	{
		//calculate intersection
		unordered_map<double, int> mp1;
		for (int i = 0; i < num_rowx; i ++)
		{
			string cur_cell = tx->cells[i][cx];
			if (cur_cell.find_first_not_of(' ') != std::string::npos)
				mp1[atoi(cur_cell.c_str())] ++;
		}

		for (int i = 0; i < num_rowy; i ++)
		{
			string cur_cell = ty->cells[i][cy];
			if (cur_cell.find_first_not_of(' ') != std::string::npos)
			{
				double x = atoi(cur_cell.c_str());
				if (mp1.count(x) && mp1[x] > 0)
					intersection ++, mp1[x] --;
			}
		}

		double sim = (double) intersection / (num_rowx + num_rowy - intersection);
		return sim >= numeric_column_sim_threshold;
	}
	//string
	else
	{
		//calculate intersection
		unordered_map<string, int> mp1;
		for (int i = 0; i < num_rowx; i ++)
		{
			string cur_cell = tx->cells[i][cx];
			if (cur_cell.find_first_not_of(' ') != std::string::npos)
				mp1[cur_cell] ++;
		}
		for (int i = 0; i < num_rowy; i ++)
		{
			string cur_cell = ty->cells[i][cy];
			if (cur_cell.find_first_not_of(' ') != std::string::npos)
				if (mp1.count(cur_cell) && mp1[cur_cell] > 0)
					intersection ++, mp1[cur_cell] --;
		}

		double sim = (double) intersection / (num_rowx + num_rowy - intersection);
		return sim >= string_column_sim_threshold;
	}
}

void ErrorModel::print_erroneous_cell(Table *t, int i, int j, cell_list_entry other_values, bool contain_self)
{
	if (t->cells[i][j] == "")
		return ;
	unordered_map<string, int> freq;
	for (auto p : other_values)
		freq[p.first] ++;
	if (! contain_self)
		freq[t->cells[i][j]] ++;
	for (auto word_freq : freq)
		if ((double) word_freq.second / (other_values.size() + 1) >= freq_threshold && word_freq.first != t->cells[i][j])
		{
			cout << "A possible erroneous cell: "
				 << t->name << "   cell(" << i << ", " << j << ")"
				 << " : " << t->cells[i][j] << endl;
			for (auto p : other_values)
				cout << '\t' << corpus->get_table(p.second.first)->name
					 << "   cell(" << p.second.second.first << ", " << p.second.second.second << ")"
					 << " : " << p.first << endl;
		}
}

PKErrorModel::PKErrorModel(Corpus *corpus) : ErrorModel(corpus) {}

void PKErrorModel::find_similar_pks()
{
	similar_pk_pairs.clear();
	matches.clear();

	//calculate matches
	int num_table = corpus->count_table();
	for (int t1 = 0; t1 < num_table; t1 ++)
		for (int t2 = t1 + 1; t2 < num_table; t2 ++)
		{
			Table *tx = corpus->get_table(t1);
			Table *ty = corpus->get_table(t2);
			double sim = column_sim(tx, tx->primary_key_column, ty, ty->primary_key_column);
			double column_sim_threshold = (tx->stats_info[tx->primary_key_column].numeric ? numeric_column_sim_threshold : string_column_sim_threshold);
			if (sim >= column_sim_threshold)
			{
				cout << tx->name << " " << tx->primary_key_column << endl
					 << ty->name << " " << ty->primary_key_column << endl
					 << sim << endl << endl;
				similar_pk_pairs.emplace_back(t1, t2);
				matches.push_back(matchedX);
				similar_pk_pairs.emplace_back(t2, t1);
				matches.push_back(matchedY);
			}
		}
}

void PKErrorModel::find_erroneous_cells()
{
	//call function to find similar pk pairs
	find_similar_pks();

	int num_table = corpus->count_table();
	int num_similar_pk_pairs = (int) similar_pk_pairs.size();

	vector<vector<int>> related_pairs(num_table);
	for (int i = 0; i < num_similar_pk_pairs; i ++)
	{
		auto p = similar_pk_pairs[i];
		related_pairs[p.first].push_back(i);
	}

	for (int t1 = 0; t1 < num_table; t1 ++)
	{
		Table *tx = corpus->get_table(t1);
		int num_rowx = tx->num_row;
		int num_colx = tx->num_col;

		for (int j = 0; j < num_colx; j ++)
		{
			cell_list other_values(num_rowx);
			for (int p : related_pairs[t1])
			{
				int t2 = similar_pk_pairs[p].second;
				Table *ty = corpus->get_table(t2);
				int num_rowy = ty->num_row;
				int num_coly = ty->num_col;
				vector<int> &matchedX = matches[p];
				assert((int) matchedX.size() == num_rowx);

				for (int k = 0; k < num_coly; k ++)
				{
					Stats sx = tx->stats_info[j];
					Stats sy = ty->stats_info[k];

					if (sx.numeric != sy.numeric)
						continue;

					//calculate similarity
					int num_matches = 0;
					double sum1, sum2, sum3;
					sum1 = sum2 = sum3 = 0;
					for (int i = 0; i < num_rowx; i ++)
						if (matchedX[i] >= 0)
						{
							if (sx.numeric)
							{
								string cellx = tx->cells[i][j];
								string celly = ty->cells[matchedX[i]][k];
								if (cellx.find_first_not_of(' ') == std::string::npos || celly.find_first_not_of(' ') == std::string::npos)
									continue;
								double diff = fabs(atoi(cellx.c_str()) - atoi(celly.c_str()));
								double sim = diff / min(sx.range, sy.range);
								if (sim <= diff_threshold)
									num_matches ++;

								double a = atoi(cellx.c_str()) - sx.mean;
								double b = atoi(celly.c_str()) - sy.mean;
								sum1 += a * b;
								sum2 += a * a;
								sum3 += b * b;
							}
							else
							{
								double sim = Common::jaccard_sim(tx->cells[i][j], ty->cells[matchedX[i]][k]);
								if (sim >= edge_threshold)
									num_matches ++;
							}
						}

					double sim = (double) num_matches * 2 / (num_rowx + num_rowy);
					if (sx.numeric)
					{
/*
						sim = (sx.mean * sy.mean + sx.var * sy.var);
						sim /= sqrt(sqr(sx.mean) + sqr(sx.var));
						sim /= sqrt(sqr(sy.mean) + sqr(sy.var));
						sim += 1, sim /= 2.0;

						sim = (sum1 / sqrt(sum2 * sum3) + 1.0) / 2.0;
*/
					}

					double column_sim_threshold = (sx.numeric ? numeric_column_sim_threshold : string_column_sim_threshold);
					if (sim >= column_sim_threshold)
					{
//						cout << tx->name << " " << j << " " << ty->name << " " << k << endl;
						for (int i = 0; i < num_rowx; i ++)
							if (matchedX[i] >= 0 && ty->cells[matchedX[i]][k] != "")
							{
								other_values[i].emplace_back(ty->cells[matchedX[i]][k], make_pair(t2, make_pair(matchedX[i], k)));
								if (! sx.numeric && tx->cells[i][j] != ty->cells[matchedX[i]][k])
								{
									matching_pairs.emplace_back(tx->cells[i][j], ty->cells[matchedX[i]][k]);
//									cout << tx->name << " " << j << " " << ty->name << " " << k << endl;
//									cout << tx->cells[i][j] << endl << ty->cells[matchedX[i]][k] << endl;
								}
							}
					}
				}
			}

			//check if there is any chance to say some cell is not correct
			for (int i = 0; i < num_rowx; i ++)
				print_erroneous_cell(tx, i, j, other_values[i], false);
		}
	}
}

SimErrorModel::SimErrorModel(Corpus *corpus) : ErrorModel(corpus) {}

void SimErrorModel::find_erroneous_cells()
{
	int num_table = corpus->count_table();
	for (int t1 = 0; t1 < num_table; t1 ++)
	{
		Table *tx = corpus->get_table(t1);
		int num_rowx = tx->num_row;
		int num_colx = tx->num_col;
		for (int cx = 0; cx < num_colx; cx ++)
		{
			cout << endl;
			cout << "==========================" << endl;
			cout << tx->name << " " << cx  << " : " << endl;
			cout << "==========================" << endl;
			cout << endl;

			cell_list other_values(num_rowx);
			for (int t2 = t1 + 1; t2 < num_table; t2 ++)
			{
				Table *ty = corpus->get_table(t2);
				int num_coly = ty->num_col;
				for (int cy = 0; cy < num_coly; cy ++)
				{
					double sim = column_sim(tx, cx, ty, cy);
					Stats sx = tx->stats_info[cx];
					Stats sy = ty->stats_info[cy];
					if (sx.numeric != sy.numeric)
						continue;

					double column_sim_threshold = (sx.numeric ? numeric_column_sim_threshold : string_column_sim_threshold);

					if (sim >= column_sim_threshold)
						for (int i = 0; i < num_rowx; i ++)
							if (matchedX[i] >= 0 && ty->cells[matchedX[i]][cy] != "")
							{
								other_values[i].emplace_back(ty->cells[matchedX[i]][cy], make_pair(t2, make_pair(matchedX[i], cy)));
								if (! sx.numeric)
									matching_pairs.emplace_back(tx->cells[i][cx], ty->cells[matchedX[i]][cy]);
							}
				}
			}
			for (int i = 0; i < num_rowx; i ++)
				print_erroneous_cell(tx, i, cx, other_values[i], false);
		}
	}
}

FDErrorModel::FDErrorModel(Corpus *corpus, string fd_file_name) : ErrorModel(corpus)
{
	ifstream fin(fd_file_name.c_str());
	string table_name;
	int c1, c2, num_table = corpus->count_table();
	fds.clear();

	while (fin >> table_name >> c1 >> c2)
	{
		string s;
		for (int i = 0; i < 3; i ++)
			getline(fin, s);

		int table_id = -1;
		for (int i = 0; i < num_table; i ++)
			if (corpus->get_table(i)->name == table_name)
				table_id = i;
		fds.push_back(FD(table_id, c1, c2));
	}

/*
	//add pk fds
	for (int i = 0; i < num_table; i ++)
	{
		Table *cur_table = corpus->get_table(i);
		int pk = cur_table->primary_key_column;
		for (int j = 0; j < cur_table->num_col; j ++)
		{
			if (j == pk)
				continue;
			bool exist = false;
			for (FD fd : fds)
				if (fd.table_id == i && fd.c1 == pk && fd.c2 == j)
				{
					exist = true;
					break;
				}
			if (! exist)
				fds.push_back(FD(i, pk, j));
		}
	}
*/
}

void FDErrorModel::find_erroneous_cells()
{
	int num_table = corpus->count_table();
	//other_values big array
	vector<vector<vector<cell_list_entry>>> other_values;
	vector<vector<vector<bool>>> is_error;

	for (int i = 0; i < num_table; i ++)
	{
		Table *cur_table = corpus->get_table(i);
		other_values.push_back(vector<vector<cell_list_entry>>(cur_table->num_row, vector<cell_list_entry>(cur_table->num_col)));
		is_error.push_back(vector<vector<bool>>(cur_table->num_row, vector<bool>(cur_table->num_col, false)));
	}

	//enumerate FD pairs
	for (int x = 0; x < (int) fds.size(); x ++)
		for (int y = x; y < (int) fds.size(); y ++)
		{
//			cout << x << " " << y << endl;
			Table *tx = corpus->get_table(fds[x].table_id);
			Table *ty = corpus->get_table(fds[y].table_id);
			int x1 = fds[x].c1, y1 = fds[x].c2;
			int x2 = fds[y].c1, y2 = fds[y].c2;
			if (! column_sim_exact(tx, x1, ty, x2) || ! column_sim_exact(tx, y1, ty, y2))
				continue;

			cout << x << " " << y << endl;
			int num_rowx = tx->num_row;
			int num_rowy = ty->num_row;
			unordered_map<string, vector<int>> hash_table;
			for (int j = 0; j < num_rowy; j ++)
			{
				string cur_str = ty->cells[j][x2];
				if (! hash_table.count(cur_str))
					hash_table[cur_str].clear();
				hash_table[cur_str].push_back(j);
			}

			for (int i = 0; i < num_rowx; i ++)
				if (tx->cells[i][x1].find_first_not_of(' ') != std::string::npos)
					for (int j : hash_table[tx->cells[i][x1]])
					{
						other_values[tx->id][i][y1].emplace_back(ty->cells[j][y2], make_pair(ty->id, make_pair(j, y2)));
						other_values[ty->id][j][y2].emplace_back(tx->cells[i][y1], make_pair(tx->id, make_pair(i, y1)));
						if (tx->cells[i][y1] != ty->cells[j][y2])
						{
/*
							cout << "A violation: " << endl;
							cout << "\t" << tx->name << " : (" << i << ", " << x1 << ", " << tx->cells[i][x1] << ") => " << "(" << i << ", " << y1 << ", " << tx->cells[i][y1] << ")." << endl;
							cout << "\t" << ty->name << " : (" << j << ", " << x2 << ", " << ty->cells[j][x2] << ") => " << "(" << j << ", " << y2 << ", " << ty->cells[j][y2] << ")." << endl;
							cout << endl;
 */
							is_error[tx->id][i][x1] = is_error[tx->id][i][y1] = true;
							is_error[ty->id][j][x2] = is_error[ty->id][j][y2] = true;
						}
					}
		}

	for (int i = 0; i < num_table; i ++)
	{
		Table *cur_table = corpus->get_table(i);
		for (int x = 0; x < cur_table->num_row; x ++)
			for (int y = 0; y < cur_table->num_col; y ++)
				if (is_error[i][x][y])
					cout << cur_table->name << " " << x << " " << y << endl;
	}

/*
	//print
	for (int i = 0; i < num_table; i ++)
	{
		Table *cur_table = corpus->get_table(i);
		for (int x = 0; x < cur_table->num_row; x ++)
			for (int y = 0; y < cur_table->num_col; y ++)
				print_erroneous_cell(cur_table, x, y, other_values[i][x][y], true);
	}
*/
}
