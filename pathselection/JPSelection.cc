#include "Common.h"
#include "JPSelection.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <unordered_set>
typedef vector<Cell> lineage;
using namespace std;

JPSelection::JPSelection(Corpus *corpus, string error_estimation_file, string ground_truth_file, string join_path_file) :
	corpus(corpus)
{
	int num_tables = corpus->count_table();
	for (int i = 0; i < num_tables; i ++)
		tname2tid[corpus->get_table(i)->name] = i;

	//cleanliness
	for (int i = 0; i < num_tables; i ++)
	{
		Table *cur_table = corpus->get_table(i);
		cleanliness_scores.push_back(vector<vector<double>>(cur_table->num_row, vector<double>(cur_table->num_col, 1.0)));
		is_error.push_back(vector<vector<bool>>(cur_table->num_row, vector<bool>(cur_table->num_col, false)));
	}

	ifstream fin(error_estimation_file.c_str());
	string table_name;
	int x, y;
	double cleanliness;
	while (fin >> table_name >> x >> y >> cleanliness)
		if (tname2tid.count(table_name))
		{
			int cur_table_id = tname2tid[table_name];
			if (x >= corpus->get_table(cur_table_id)->num_row || y >= corpus->get_table(cur_table_id)->num_col)
				continue;
			cleanliness_scores[cur_table_id][x][y] = cleanliness;
		}
	fin.close();

	//enlarge tables for scalability test
	enlarge_tables();

	//ground truth
	ifstream fin1(ground_truth_file.c_str());
	while (fin1 >> table_name >> x >> y >> cleanliness)
		if (tname2tid.count(table_name))
		{
			int cur_table_id = tname2tid[table_name];
			is_error[cur_table_id][x][y] = true;
		}
	fin.close();

	//join paths
	ifstream fin2(join_path_file.c_str());
	int n, m;
	fin2 >> n;
	jps.clear();

	for (int i = 0; i < n; i ++)
	{
		fin2 >> m;
		JP cur_jp;
		for (int j = 0; j < m; j ++)
		{
			fin2 >> table_name;
			cur_jp.tables.push_back(tname2tid[table_name]);
		}
		for (int j = 0; j < m - 1; j ++)
		{
			int x, y;
			fin2 >> x >> y;
			cur_jp.jpreds.emplace_back(x, y);
		}
		fin2 >> m;
		for (int j = 0; j < m; j ++)
		{
			int x, y;
			fin2 >> x >> y;
			cur_jp.proj_cols.emplace_back(x, y);
		}
		jps.push_back(cur_jp);
	}
	fin2.close();
}

bool JPSelection::isL1(JP cur_jp, Cell cell)
{
	int n = (int) cur_jp.tables.size();
	for (int i = 0; i < n - 1; i ++)
	{
		auto cp = cur_jp.jpreds[i];
		if (cell.tid == i && cell.y == cp.first)
			return true;
		if (cell.tid == i + 1 && cell.y == cp.second)
			return true;
	}
	return false;
}

vector<double> JPSelection::calculate_num_clean_values(JP cur_jp, vector<vector<lineage>> &lin, vector<vector<double>> &cln,
														  vector<vector<vector<bool>>> &is_error_new, vector<vector<vector<bool>>> &is_error_old)
{
	//calculate number of clean values based on ground truth
	//and calculate the estimations - the accumulate accuracy and average accuracy
	int num_clean = 0;
	int column_count = 0;
	int removed = 0;
	double sum_precision = 0;
	int n = (int) cur_jp.tables.size();
	for (int k = 0; k < n; k ++)
	{
		Table *cur_table = corpus->get_table(cur_jp.tables[k]);
		for (auto cp : cur_jp.proj_cols)
			if (cp.first == k)
			{
				int cur_col = cp.second + column_count;
				for (int i = 0; i < (int) lin.size(); i ++)
				{
					//check whether it's clean
					bool clean = true;
					bool exist = true;
					for (Cell cell : lin[i][cur_col])
					{
						int table_id = corpus->get_table(cur_jp.tables[cell.tid])->id;
						if (is_error_new[table_id][cell.x][cell.y])
							clean = false;
						if (is_error_old[table_id][cell.x][cell.y] && ! is_error_new[table_id][cell.x][cell.y] && isL1(cur_jp, cell))
							exist = false;
					}
					if (! exist)
						removed ++;
					if (clean && exist)
						num_clean ++;

					//accumulate average
					sum_precision += cln[i][cur_col];
				}
			}
		column_count += cur_table->num_col;
	}

	vector<double> ans;
	ans.push_back(num_clean);
	ans.push_back(sum_precision);
	ans.push_back((double) lin.size() * cur_jp.proj_cols.size() - (double) removed);

	return ans;
}


string JPSelection::show_result_table(JP cur_jp, vector<vector<string>> &val, int limit)
{
	//calculate number of clean values based on ground truth
	//and calculate the estimations - the accumulate accuracy and average accuracy
	vector<string> str_rows;
  str_rows.resize(val.size());
	int column_count = 0;
	int n = (int) cur_jp.tables.size();
	for (int k = 0; k < n; k ++)
	{
		Table *cur_table = corpus->get_table(cur_jp.tables[k]);
		for (auto cp : cur_jp.proj_cols)
			if (cp.first == k)
			{
				int cur_col = cp.second + column_count;
				for (int i = 0; i < (int) val.size(); i ++)
          str_rows[i] += val[i][cur_col] + "\t";
			}
    column_count += cur_table->num_col;
	}
  string ans = "";
  int cnt = 0;
  for (auto str : str_rows) {
    if (++cnt> limit) {
      break;
    }
    ans = ans + str;
    ans.pop_back();
    ans += "\n";
  }
	return ans;
}

void JPSelection::enlarge_tables()
{
	int n = corpus->count_table();
	for (int k = 0; k < n; k ++)
	{
		Table *cur_table = corpus->get_table(k);
		int num_row = cur_table->num_row;

		for (int t = 1; t < 30; t ++)
			for (int i = 0; i < num_row; i ++)
			{
				cur_table->cells.push_back(cur_table->cells[i]);
				cleanliness_scores[cur_table->id].push_back(cleanliness_scores[cur_table->id][i]);
			}
	}
}

bool invalidChar (char c) 
{  
  return !(c>=0 && c <128);   
} 
void stripUnicode(string & str) 
{ 
  str.erase(remove_if(str.begin(),str.end(), invalidChar), str.end());  
}

string JPSelection::select_join_path(int K)
{
  stringstream ss;
  int counter = 0;
  //calculate query result
  ss << "Join Path\t Cardinality\t Estimated Cleanliness\t Combined Ranking Score" << endl;
  for (JP cur_jp : jps)
  {
    int n = (int) cur_jp.tables.size();

    int cur = 0;
    int column_count = 0;
    vector<vector<lineage>> lin[2];
    vector<vector<string>> val[2];
    vector<vector<double>> cln[2];

    //initialize table 0
    if (n > 0)
    {
      Table *cur_table = corpus->get_table(cur_jp.tables[0]);
      int num_row = cur_table->num_row * K;
      int num_col = cur_table->num_col;

      lin[cur].clear();
      lin[cur] = vector<vector<lineage>>(num_row, vector<lineage>(num_col));
      val[cur].clear();
      val[cur] = vector<vector<string>>(num_row, vector<string>(num_col));
      cln[cur].clear();
      cln[cur] = vector<vector<double>>(num_row, vector<double>(num_col));

      for (int i = 0; i < num_row; i ++)
        for (int j = 0; j < num_col; j ++)
        {
          lin[cur][i][j].push_back(Cell(0, i, j));
          val[cur][i][j] = cur_table->cells[i][j];
          cln[cur][i][j] = cleanliness_scores[cur_table->id][i][j];
        }
    }

    //hash joins
    for (int k = 1; k < n; k ++)
    {
      cur = 1 - cur;
      lin[cur].clear();
      val[cur].clear();
      cln[cur].clear();

      Table *cur_table = corpus->get_table(cur_jp.tables[k]);
      int num_row = cur_table->num_row * K;
      int num_col = cur_table->num_col;

      //hash join with lin[1 - cur]
      int x = cur_jp.jpreds[k - 1].first + column_count;
      int y = cur_jp.jpreds[k - 1].second;

      unordered_map<string, vector<int>> hash_map;
      for (int i = 0; i < (int) val[1 - cur].size(); i ++)
      {
        string cur_value = val[1 - cur][i][x];
        if (! hash_map.count(cur_value))
          hash_map[cur_value].clear();
        hash_map[cur_value].push_back(i);
      }

      for (int i = 0; i < num_row; i ++)
      {
        string cur_value = cur_table->cells[i][y];
        if (hash_map.count(cur_value))
          for (int j : hash_map[cur_value])
          {
            //concatenate i & j
            int col_size = (int) val[1 - cur][j].size() + num_col;
            int cur_row = (int) val[cur].size();

            lin[cur].push_back(vector<lineage>(col_size));
            val[cur].push_back(vector<string>(col_size));
            cln[cur].push_back(vector<double>(col_size));
            for (int p = 0; p < (int) val[1 - cur][j].size(); p ++)
            {
              //lineage
              lineage &r = lin[cur][cur_row][p];
              r = lin[1 - cur][j][p];
              r.push_back(Cell(k, i, y));
              if (p != x)
                r.insert(r.end(), lin[1 - cur][j][x].begin(), lin[1 - cur][j][x].end());

              //value
              val[cur][cur_row][p] = val[1 - cur][j][p];

              //cleanliness
              cln[cur][cur_row][p] = cln[1 - cur][j][p];
              cln[cur][cur_row][p] *= cleanliness_scores[cur_table->id][i][y];
              if (p != x)
                cln[cur][cur_row][p] *= cln[1 - cur][j][x];
            }
            for (int p = 0; p < num_col; p ++)
            {
              //lineage
              lineage &r = lin[cur][cur_row][(int) val[1 - cur][j].size() + p];
              r.push_back(Cell(k, i, p));
              if (p != y)
                r.push_back(Cell(k, i, y));
              r.insert(r.end(), lin[1 - cur][j][x].begin(), lin[1 - cur][j][x].end());

              //value
              val[cur][cur_row][(int) val[1 - cur][j].size() + p] = cur_table->cells[i][p];

              //cleanliness
              cln[cur][cur_row][(int) val[1 - cur][j].size() + p] = cleanliness_scores[cur_table->id][i][p];
              cln[cur][cur_row][(int) val[1 - cur][j].size() + p] *= cln[1 - cur][j][x];
              if (p != y)
                cln[cur][cur_row][(int) val[1 - cur][j].size() + p] *= cleanliness_scores[cur_table->id][i][y];
            }
          }
      }
      column_count += corpus->get_table(cur_jp.tables[k - 1])->num_col;
    }

    auto num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error, is_error);
    double num_clean = num_clean_value_cp[0];
    double sum_precision = num_clean_value_cp[1];

    /*
       ss << (counter ++) << " : " << endl;
       ss << "Cardinality : " << val[cur].size() << endl;
       ss << "# Clean values: " << num_clean << endl;
       ss << "Accumulated Precision: " << sum_precision << endl;
       ss << "Average Precision : " << sum_precision / val[cur].size() / cur_jp.proj_cols.size() << endl;
       ss << show_result_table(cur_jp, val[cur], 10);
       */


    ss << (counter ++) << "\t\t";
    ss << val[cur].size() << "\t\t";
    ss << sum_precision / val[cur].size() / cur_jp.proj_cols.size() << "\t\t\t";
    ss << val[cur].size() * 1.0 * sum_precision / val[cur].size() / cur_jp.proj_cols.size() << endl;
  }

  //mark projection columns
  //Dong's greedy algorithm
  return ss.str();
}

string JPSelection::budget_clean(int B, int jp_idx)
{
  int K = 1;
	//calculate query result
  stringstream ss;
  JP cur_jp = jps[jp_idx];
  int n = (int) cur_jp.tables.size();

  int cur = 0;
  int column_count = 0;
  vector<vector<lineage>> lin[2];
  vector<vector<string>> val[2];
  vector<vector<double>> cln[2];

  //initialize table 0
  if (n > 0)
  {
    Table *cur_table = corpus->get_table(cur_jp.tables[0]);
    int num_row = cur_table->num_row * K;
    int num_col = cur_table->num_col;

    lin[cur].clear();
    lin[cur] = vector<vector<lineage>>(num_row, vector<lineage>(num_col));
    val[cur].clear();
    val[cur] = vector<vector<string>>(num_row, vector<string>(num_col));
    cln[cur].clear();
    cln[cur] = vector<vector<double>>(num_row, vector<double>(num_col));

    for (int i = 0; i < num_row; i ++)
      for (int j = 0; j < num_col; j ++)
      {
        lin[cur][i][j].push_back(Cell(0, i, j));
        val[cur][i][j] = cur_table->cells[i][j];
        cln[cur][i][j] = cleanliness_scores[cur_table->id][i][j];
      }
  }

  //hash joins
  for (int k = 1; k < n; k ++)
  {
    cur = 1 - cur;
    lin[cur].clear();
    val[cur].clear();
    cln[cur].clear();

    Table *cur_table = corpus->get_table(cur_jp.tables[k]);
    int num_row = cur_table->num_row * K;
    int num_col = cur_table->num_col;

    //hash join with lin[1 - cur]
    int x = cur_jp.jpreds[k - 1].first + column_count;
    int y = cur_jp.jpreds[k - 1].second;

    unordered_map<string, vector<int>> hash_map;
    for (int i = 0; i < (int) val[1 - cur].size(); i ++)
    {
      string cur_value = val[1 - cur][i][x];
      if (! hash_map.count(cur_value))
        hash_map[cur_value].clear();
      hash_map[cur_value].push_back(i);
    }

    for (int i = 0; i < num_row; i ++)
    {
      string cur_value = cur_table->cells[i][y];
      if (hash_map.count(cur_value))
        for (int j : hash_map[cur_value])
        {
          //concatenate i & j
          int col_size = (int) val[1 - cur][j].size() + num_col;
          int cur_row = (int) val[cur].size();

          lin[cur].push_back(vector<lineage>(col_size));
          val[cur].push_back(vector<string>(col_size));
          cln[cur].push_back(vector<double>(col_size));
          for (int p = 0; p < (int) val[1 - cur][j].size(); p ++)
          {
            //lineage
            lineage &r = lin[cur][cur_row][p];
            r = lin[1 - cur][j][p];
            r.push_back(Cell(k, i, y));
            if (p != x)
              r.insert(r.end(), lin[1 - cur][j][x].begin(), lin[1 - cur][j][x].end());

            //value
            val[cur][cur_row][p] = val[1 - cur][j][p];

            //cleanliness
            cln[cur][cur_row][p] = cln[1 - cur][j][p];
            cln[cur][cur_row][p] *= cleanliness_scores[cur_table->id][i][y];
            if (p != x)
              cln[cur][cur_row][p] *= cln[1 - cur][j][x];
          }
          for (int p = 0; p < num_col; p ++)
          {
            //lineage
            lineage &r = lin[cur][cur_row][(int) val[1 - cur][j].size() + p];
            r.push_back(Cell(k, i, p));
            if (p != y)
              r.push_back(Cell(k, i, y));
            r.insert(r.end(), lin[1 - cur][j][x].begin(), lin[1 - cur][j][x].end());

            //value
            val[cur][cur_row][(int) val[1 - cur][j].size() + p] = cur_table->cells[i][p];

            //cleanliness
            cln[cur][cur_row][(int) val[1 - cur][j].size() + p] = cleanliness_scores[cur_table->id][i][p];
            cln[cur][cur_row][(int) val[1 - cur][j].size() + p] *= cln[1 - cur][j][x];
            if (p != y)
              cln[cur][cur_row][(int) val[1 - cur][j].size() + p] *= cleanliness_scores[cur_table->id][i][y];
          }
        }
    }
    column_count += corpus->get_table(cur_jp.tables[k - 1])->num_col;
  }

  auto num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error, is_error);
  double num_clean = num_clean_value_cp[0];
  double sum_precision = num_clean_value_cp[1];

  /*
     ss << (counter ++) << " : " << endl;
     ss << "Cardinality : " << val[cur].size() << endl;
     ss << "# Clean values: " << num_clean << endl;
     ss << "Accumulated Precision: " << sum_precision << endl;
     ss << "Average Precision : " << sum_precision / val[cur].size() / cur_jp.proj_cols.size() << endl;
 */
  ss << show_result_table(cur_jp, val[cur], 1000000);
  ss << "^";


  /*
  ss << (counter ++) << "\t\t";
  ss << val[cur].size() << "\t\t";
  ss << sum_precision / val[cur].size() / cur_jp.proj_cols.size() << "\t\t";
  ss << val[cur].size() * 1.0 * sum_precision / val[cur].size() / cur_jp.proj_cols.size() << endl;
  combine_score.push_back(val[cur].size() * 1.0 * sum_precision / val[cur].size() / cur_jp.proj_cols.size());
  */

  //mark projection columns
  unordered_set<int> proj_cols;
  column_count = 0;
  for (int k = 0; k < n; k ++)
  {
    Table *cur_table = corpus->get_table(cur_jp.tables[k]);
    for (auto cp : cur_jp.proj_cols)
      if (cp.first == k)
        proj_cols.insert(cp.second + column_count);
    column_count += cur_table->num_col;
  }

  //Dong's greedy algorithm
  vector<pair<double, Cell>> sort_array;
  unordered_map<Cell, vector<pair<int, int>>> inv_list;
  double sum_cln = 0;
  for (int x = 0; x < (int) lin[cur].size(); x ++)
    for (int y = 0; y < (int) lin[cur][0].size(); y ++)
      if (proj_cols.count(y))
      {
        for (Cell lin_cell : lin[cur][x][y])
        {
          if (! inv_list.count(lin_cell))
            inv_list[lin_cell].clear();
          inv_list[lin_cell].emplace_back(x, y);
        }
        sum_cln += cln[cur][x][y];
      }

  for (int k = 0; k < n; k ++)
  {
    Table *cur_table = corpus->get_table(cur_jp.tables[k]);
    for (int i = 0; i < cur_table->num_row; i ++)
      for (int j = 0; j < cur_table->num_col; j ++)
      {
        Cell cur_cell = Cell(k, i, j);
        //size
        double e_size = (double) val[cur].size() * (double) proj_cols.size();
        if (isL1(cur_jp, cur_cell) && inv_list.count(cur_cell))
          e_size -= (double) inv_list[cur_cell].size() * (1 - cleanliness_scores[cur_table->id][i][j]);

        //average cleanliness
        double avg_cln = sum_cln;
        if (! isL1(cur_jp, cur_cell) && inv_list.count(cur_cell))
          for (auto cp : inv_list[cur_cell])
          {
            int x = cp.first;
            int y = cp.second;

            avg_cln -= cln[cur][x][y];
            double new_cln = 1.0;
            for (Cell lin_cell : lin[cur][x][y])
              if (! (lin_cell == cur_cell))
                new_cln *= cleanliness_scores[corpus->get_table(cur_jp.tables[lin_cell.tid])->id][lin_cell.x][lin_cell.y];
            avg_cln += new_cln;
          }

        avg_cln /= e_size;
        sort_array.emplace_back(- avg_cln, cur_cell);
      }
  }

  sort(sort_array.begin(), sort_array.end(), [](const pair<double, Cell> &p1, pair<double, Cell> &p2) {
      return p1.first < p2.first; });

  vector<vector<vector<bool>>> is_error_new = is_error;
  ss << "Table\tRow\tCol\tImpact\t Value" << endl;
  for (int k = 0; k < min(B, (int) sort_array.size()); k ++)
  {
    Cell cur_cell = sort_array[k].second;
    is_error_new[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] = false;
    Table * cur_table = corpus->get_table(cur_jp.tables[cur_cell.tid]);
    ss  <<  cur_table->name <<  "\t"<<  cur_cell.x << "\t" << cur_cell.y << "\t"
      // << cleanliness_scores[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] << " "
      << -1 * sort_array[k].first << "\t"
      << cur_table->cells[cur_cell.x][cur_cell.y] << endl;
  }
  num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error_new, is_error);
  // ss << "Algo 0: # New clean values and precision: " << num_clean_value_cp[0] - num_clean << " " << num_clean_value_cp[0] / num_clean_value_cp[2] << endl;


  //First method - B * R
  /*
     unordered_map<Cell, int> ctb;
     for (int i = 0; i < (int) lin[cur].size(); i ++)
     for (int j = 0; j < (int) lin[cur][0].size(); j ++)
     if (proj_cols.count(j))
     for (Cell cell : lin[cur][i][j])
     ctb[cell] ++;

     sort_array.clear();
     for (auto cp : ctb)
     sort_array.emplace_back(cp.second, cp.first);
     for (auto &cp : sort_array)
     {
     Cell cur_cell = cp.second;
     double cleanliness = cleanliness_scores[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y];
     cp.first *= -(1 - cleanliness);
     }

     sort(sort_array.begin(), sort_array.end(), [](const pair<double, Cell> &p1, pair<double, Cell> &p2) {
     return p1.first < p2.first; });
     is_error_new = is_error;
     for (int k = 0; k < min(B, (int) sort_array.size()); k ++)
     {
     Cell cur_cell = sort_array[k].second;
     is_error_new[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] = false;
     }
     num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error_new, is_error);
  // ss << "Algo 1: # New clean values and precision: " << num_clean_value_cp[0] - num_clean << " " << num_clean_value_cp[0] / num_clean_value_cp[2] << endl;

  //Second method - select B dirtiest cells in source
  sort_array.clear();
  for (int k = 0; k < n; k ++)
  {
  Table *cur_table = corpus->get_table(cur_jp.tables[k]);
  for (int i = 0; i < cur_table->num_row; i ++)
  for (int j = 0; j < cur_table->num_col; j ++)
  {
  Cell cur_cell(k, i, j);
  if (cleanliness_scores[cur_table->id][i][j] < 1)
  sort_array.emplace_back(cleanliness_scores[cur_table->id][i][j], cur_cell);
  }
  }
  sort(sort_array.begin(), sort_array.end(), [](const pair<double, Cell> &p1, pair<double, Cell> &p2) {
  return p1.first < p2.first; });
  is_error_new = is_error;
  for (int k = 0; k < B; k ++)
  {
  Cell cur_cell = sort_array[k].second;
  is_error_new[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] = false;
  }
  num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error_new, is_error);
  // ss << "Algo 2: # New clean values and precision: " << num_clean_value_cp[0] - num_clean << " " << num_clean_value_cp[0] / num_clean_value_cp[2] << endl;

  //Third method - select randomly B cells in source
  is_error_new = is_error;
  srand((unsigned) time(NULL));
  for (int k = 0; k < B; k ++)
  {
  Cell cur_cell;
  cur_cell.tid = rand() % n;
  Table *cur_table = corpus->get_table(cur_jp.tables[cur_cell.tid]);
  cur_cell.x = rand() % cur_table->num_row;
  cur_cell.y = rand() % cur_table->num_col;
  is_error_new[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] = false;
  }
  num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error_new, is_error);
  // ss << "Algo 3: # New clean values and precision: " << num_clean_value_cp[0] - num_clean << " " << num_clean_value_cp[0] / num_clean_value_cp[2] << endl;
  */

  string out = ss.str();
  stripUnicode(out);
  return out;
}



/* bakup

string JPSelection::select_join_path(int B, int K)
{
  stringstream ss;
	int counter = 0;
  vector<string> cells_to_clean;
  vector<double> combine_score;
	//calculate query result

  ss << "Join Path\t Cardinality\t Estimated Cleanliness\t Combined Ranking Score" << endl;
	for (JP cur_jp : jps)
	{
		int n = (int) cur_jp.tables.size();

		int cur = 0;
		int column_count = 0;
		vector<vector<lineage>> lin[2];
		vector<vector<string>> val[2];
		vector<vector<double>> cln[2];

		//initialize table 0
		if (n > 0)
		{
			Table *cur_table = corpus->get_table(cur_jp.tables[0]);
			int num_row = cur_table->num_row * K;
			int num_col = cur_table->num_col;

			lin[cur].clear();
			lin[cur] = vector<vector<lineage>>(num_row, vector<lineage>(num_col));
			val[cur].clear();
			val[cur] = vector<vector<string>>(num_row, vector<string>(num_col));
			cln[cur].clear();
			cln[cur] = vector<vector<double>>(num_row, vector<double>(num_col));

			for (int i = 0; i < num_row; i ++)
				for (int j = 0; j < num_col; j ++)
				{
					lin[cur][i][j].push_back(Cell(0, i, j));
					val[cur][i][j] = cur_table->cells[i][j];
					cln[cur][i][j] = cleanliness_scores[cur_table->id][i][j];
				}
		}

		//hash joins
		for (int k = 1; k < n; k ++)
		{
			cur = 1 - cur;
			lin[cur].clear();
			val[cur].clear();
			cln[cur].clear();

			Table *cur_table = corpus->get_table(cur_jp.tables[k]);
			int num_row = cur_table->num_row * K;
			int num_col = cur_table->num_col;

			//hash join with lin[1 - cur]
			int x = cur_jp.jpreds[k - 1].first + column_count;
			int y = cur_jp.jpreds[k - 1].second;

			unordered_map<string, vector<int>> hash_map;
			for (int i = 0; i < (int) val[1 - cur].size(); i ++)
			{
				string cur_value = val[1 - cur][i][x];
				if (! hash_map.count(cur_value))
					hash_map[cur_value].clear();
				hash_map[cur_value].push_back(i);
			}

			for (int i = 0; i < num_row; i ++)
			{
				string cur_value = cur_table->cells[i][y];
				if (hash_map.count(cur_value))
					for (int j : hash_map[cur_value])
					{
						//concatenate i & j
						int col_size = (int) val[1 - cur][j].size() + num_col;
						int cur_row = (int) val[cur].size();

						lin[cur].push_back(vector<lineage>(col_size));
						val[cur].push_back(vector<string>(col_size));
						cln[cur].push_back(vector<double>(col_size));
						for (int p = 0; p < (int) val[1 - cur][j].size(); p ++)
						{
							//lineage
							lineage &r = lin[cur][cur_row][p];
							r = lin[1 - cur][j][p];
							r.push_back(Cell(k, i, y));
							if (p != x)
								r.insert(r.end(), lin[1 - cur][j][x].begin(), lin[1 - cur][j][x].end());

							//value
							val[cur][cur_row][p] = val[1 - cur][j][p];

							//cleanliness
							cln[cur][cur_row][p] = cln[1 - cur][j][p];
							cln[cur][cur_row][p] *= cleanliness_scores[cur_table->id][i][y];
							if (p != x)
								cln[cur][cur_row][p] *= cln[1 - cur][j][x];
						}
						for (int p = 0; p < num_col; p ++)
						{
							//lineage
							lineage &r = lin[cur][cur_row][(int) val[1 - cur][j].size() + p];
							r.push_back(Cell(k, i, p));
							if (p != y)
								r.push_back(Cell(k, i, y));
							r.insert(r.end(), lin[1 - cur][j][x].begin(), lin[1 - cur][j][x].end());

							//value
							val[cur][cur_row][(int) val[1 - cur][j].size() + p] = cur_table->cells[i][p];

							//cleanliness
							cln[cur][cur_row][(int) val[1 - cur][j].size() + p] = cleanliness_scores[cur_table->id][i][p];
							cln[cur][cur_row][(int) val[1 - cur][j].size() + p] *= cln[1 - cur][j][x];
							if (p != y)
								cln[cur][cur_row][(int) val[1 - cur][j].size() + p] *= cleanliness_scores[cur_table->id][i][y];
						}
					}
			}
			column_count += corpus->get_table(cur_jp.tables[k - 1])->num_col;
		}

		auto num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error, is_error);
		double num_clean = num_clean_value_cp[0];
		double sum_precision = num_clean_value_cp[1];

		ss << (counter ++) << " : " << endl;
		ss << "Cardinality : " << val[cur].size() << endl;
		ss << "# Clean values: " << num_clean << endl;
		ss << "Accumulated Precision: " << sum_precision << endl;
		ss << "Average Precision : " << sum_precision / val[cur].size() / cur_jp.proj_cols.size() << endl;
    ss << show_result_table(cur_jp, val[cur], 10);


		ss << (counter ++) << "\t\t";
		ss << val[cur].size() << "\t\t";
		ss << sum_precision / val[cur].size() / cur_jp.proj_cols.size() << "\t\t";
    ss << val[cur].size() * 1.0 * sum_precision / val[cur].size() / cur_jp.proj_cols.size() << endl;
    combine_score.push_back(val[cur].size() * 1.0 * sum_precision / val[cur].size() / cur_jp.proj_cols.size());
    stringstream cell_to_clean;

		//mark projection columns
		unordered_set<int> proj_cols;
		column_count = 0;
		for (int k = 0; k < n; k ++)
		{
			Table *cur_table = corpus->get_table(cur_jp.tables[k]);
			for (auto cp : cur_jp.proj_cols)
				if (cp.first == k)
					proj_cols.insert(cp.second + column_count);
			column_count += cur_table->num_col;
		}

		//Dong's greedy algorithm
		vector<pair<double, Cell>> sort_array;
		unordered_map<Cell, vector<pair<int, int>>> inv_list;
		double sum_cln = 0;
		for (int x = 0; x < (int) lin[cur].size(); x ++)
			for (int y = 0; y < (int) lin[cur][0].size(); y ++)
			if (proj_cols.count(y))
			{
				for (Cell lin_cell : lin[cur][x][y])
				{
					if (! inv_list.count(lin_cell))
						inv_list[lin_cell].clear();
					inv_list[lin_cell].emplace_back(x, y);
				}
				sum_cln += cln[cur][x][y];
			}

		for (int k = 0; k < n; k ++)
		{
			Table *cur_table = corpus->get_table(cur_jp.tables[k]);
			for (int i = 0; i < cur_table->num_row; i ++)
				for (int j = 0; j < cur_table->num_col; j ++)
				{
					Cell cur_cell = Cell(k, i, j);
					//size
					double e_size = (double) val[cur].size() * (double) proj_cols.size();
					if (isL1(cur_jp, cur_cell) && inv_list.count(cur_cell))
						e_size -= (double) inv_list[cur_cell].size() * (1 - cleanliness_scores[cur_table->id][i][j]);

					//average cleanliness
					double avg_cln = sum_cln;
					if (! isL1(cur_jp, cur_cell) && inv_list.count(cur_cell))
						for (auto cp : inv_list[cur_cell])
						{
							int x = cp.first;
							int y = cp.second;

							avg_cln -= cln[cur][x][y];
							double new_cln = 1.0;
							for (Cell lin_cell : lin[cur][x][y])
								if (! (lin_cell == cur_cell))
									new_cln *= cleanliness_scores[corpus->get_table(cur_jp.tables[lin_cell.tid])->id][lin_cell.x][lin_cell.y];
							avg_cln += new_cln;
						}

					avg_cln /= e_size;
					sort_array.emplace_back(- avg_cln, cur_cell);
				}
		}

		sort(sort_array.begin(), sort_array.end(), [](const pair<double, Cell> &p1, pair<double, Cell> &p2) {
         return p1.first < p2.first; });

		vector<vector<vector<bool>>> is_error_new = is_error;
		for (int k = 0; k < min(B, (int) sort_array.size()); k ++)
		{
			Cell cur_cell = sort_array[k].second;
			is_error_new[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] = false;
      Table * cur_table = corpus->get_table(cur_jp.tables[cur_cell.tid]);
      cell_to_clean << cur_table->cells[cur_cell.x][cur_cell.y] << "\t\t" << cur_cell.x << " " << cur_cell.y << " "
                    << cleanliness_scores[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] << endl;

		}
    cells_to_clean.push_back(cell_to_clean.str());
		num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error_new, is_error);
		// ss << "Algo 0: # New clean values and precision: " << num_clean_value_cp[0] - num_clean << " " << num_clean_value_cp[0] / num_clean_value_cp[2] << endl;


		//First method - B * R
		unordered_map<Cell, int> ctb;
		for (int i = 0; i < (int) lin[cur].size(); i ++)
			for (int j = 0; j < (int) lin[cur][0].size(); j ++)
				if (proj_cols.count(j))
					for (Cell cell : lin[cur][i][j])
						ctb[cell] ++;

		sort_array.clear();
		for (auto cp : ctb)
			sort_array.emplace_back(cp.second, cp.first);
		for (auto &cp : sort_array)
		{
			Cell cur_cell = cp.second;
			double cleanliness = cleanliness_scores[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y];
			cp.first *= -(1 - cleanliness);
		}

		sort(sort_array.begin(), sort_array.end(), [](const pair<double, Cell> &p1, pair<double, Cell> &p2) {
         return p1.first < p2.first; });
		is_error_new = is_error;
		for (int k = 0; k < min(B, (int) sort_array.size()); k ++)
		{
			Cell cur_cell = sort_array[k].second;
			is_error_new[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] = false;
		}
		num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error_new, is_error);
		// ss << "Algo 1: # New clean values and precision: " << num_clean_value_cp[0] - num_clean << " " << num_clean_value_cp[0] / num_clean_value_cp[2] << endl;

		//Second method - select B dirtiest cells in source
		sort_array.clear();
		for (int k = 0; k < n; k ++)
		{
			Table *cur_table = corpus->get_table(cur_jp.tables[k]);
			for (int i = 0; i < cur_table->num_row; i ++)
				for (int j = 0; j < cur_table->num_col; j ++)
				{
					Cell cur_cell(k, i, j);
					if (cleanliness_scores[cur_table->id][i][j] < 1)
						sort_array.emplace_back(cleanliness_scores[cur_table->id][i][j], cur_cell);
				}
		}
		sort(sort_array.begin(), sort_array.end(), [](const pair<double, Cell> &p1, pair<double, Cell> &p2) {
         return p1.first < p2.first; });
		is_error_new = is_error;
		for (int k = 0; k < B; k ++)
		{
			Cell cur_cell = sort_array[k].second;
			is_error_new[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] = false;
		}
		num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error_new, is_error);
		// ss << "Algo 2: # New clean values and precision: " << num_clean_value_cp[0] - num_clean << " " << num_clean_value_cp[0] / num_clean_value_cp[2] << endl;

		//Third method - select randomly B cells in source
		is_error_new = is_error;
		srand((unsigned) time(NULL));
		for (int k = 0; k < B; k ++)
		{
			Cell cur_cell;
			cur_cell.tid = rand() % n;
			Table *cur_table = corpus->get_table(cur_jp.tables[cur_cell.tid]);
			cur_cell.x = rand() % cur_table->num_row;
			cur_cell.y = rand() % cur_table->num_col;
			is_error_new[corpus->get_table(cur_jp.tables[cur_cell.tid])->id][cur_cell.x][cur_cell.y] = false;
		}
		num_clean_value_cp = calculate_num_clean_values(cur_jp, lin[cur], cln[cur], is_error_new, is_error);
		// ss << "Algo 3: # New clean values and precision: " << num_clean_value_cp[0] - num_clean << " " << num_clean_value_cp[0] / num_clean_value_cp[2] << endl;
	}
  int max_idx = 0;
  double max_score = 0;
  for (int idx = 0; idx < combine_score.size(); idx++) {
    if (combine_score[idx] > max_score) {
      max_score = combine_score[idx];
      max_idx = idx;
    }
  }
  ss << cells_to_clean[max_idx] << endl;

  string out = ss.str();
  stripUnicode(out);
  return out;
}
*/
