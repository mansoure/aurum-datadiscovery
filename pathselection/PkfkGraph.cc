#include "PkfkGraph.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
using namespace std;

PkfkGraph::PkfkGraph(string file_name)
{
	ifstream fin(file_name.c_str(), ios::in);
	unordered_map<string, int> s;
	vector<unordered_set<int>> edge;
	num_columns = 0;
	s.clear();
	ids.clear();
	adj.clear();
	edge.clear();

	int t1, c1, t2, c2;
	double sim;
	while (fin >> t1 >> t2 >> c1 >> c2 >> sim)
	{
		string s1 = to_string(t1) + "_" + to_string(c1);
		string s2 = to_string(t2) + "_" + to_string(c2);
		if (! s.count(s1))
		{
			ids.emplace_back(t1, c1);
			s[s1] = num_columns ++;
			adj.push_back(vector<pair<int, int>>());
			edge.push_back(unordered_set<int>());
		}
		if (! s.count(s2))
		{
			ids.emplace_back(t2, c2);
			s[s2] = num_columns ++;
			adj.push_back(vector<pair<int, int>>());
			edge.push_back(unordered_set<int>());
		}

		if (t1 != t2)
		{
			int x = s[s1], y = s[s2];
			if (! edge[x].count(y))
				adj[x].emplace_back(y, 1), edge[x].insert(y);
			if (! edge[y].count(x))
				adj[y].emplace_back(x, 1), edge[y].insert(x);
		}
	}
	fin.close();

	for (int i = 0; i < num_columns; i ++)
		for (int j = 0; j <= ids[i].second; j ++)
		{
			string ss = to_string(ids[i].first) + "_" + to_string(j);
			if (! s.count(ss))
			{
				ids.emplace_back(ids[i].first, j);
				s[ss] = num_columns ++;
				adj.push_back(vector<pair<int, int>>());
				edge.push_back(unordered_set<int>());
			}
		}

	//add edges between columns in the same table
	for (int i = 0; i < num_columns; i ++)
		for (int j = i + 1; j < num_columns; j ++)
			if (ids[i].first == ids[j].first)
			{
				adj[i].emplace_back(j, 0);
				if (i != j)
					adj[j].emplace_back(i, 0);
			}

	int sum_edge = 0;
	for (int i = 0; i < num_columns; i ++)
		sum_edge += (int) adj[i].size();
	cout << sum_edge << endl;
}

void PkfkGraph::print_join_paths()
{
	vector<vector<int>> global_num_paths;
	global_num_paths.resize(num_columns);
	for (int i = 0; i < num_columns; i ++)
		global_num_paths[i].resize(num_columns);

	for (int s = 0; s < num_columns; s ++)
	{
		vector<vector<int>> num_paths;
		num_paths.resize(num_columns);
		for (int i = 0; i < num_columns; i ++)
			num_paths[i].resize(MAX_HOP + 1);

		num_paths[s][0] = 1;
		for (int hop = 0; hop < MAX_HOP; hop ++)
			for (int i = 0; i < num_columns; i ++)
				if (num_paths[i][hop])
					for (auto p : adj[i])
						if (p.second != hop % 2)
							num_paths[p.first][hop + 1] += num_paths[i][hop];

		for (int hop = 1; hop <= MAX_HOP; hop ++)
			for (int i = 0; i < num_columns; i ++)
				global_num_paths[s][i] += num_paths[i][hop];

		num_paths.clear();
		num_paths.resize(num_columns);
		for (int i = 0; i < num_columns; i ++)
			num_paths[i].resize(MAX_HOP + 1);

		num_paths[s][1] = 1;
		for (int hop = 0; hop < MAX_HOP; hop ++)
			for (int i = 0; i < num_columns; i ++)
				if (num_paths[i][hop])
					for (auto p : adj[i])
						if (p.second != hop % 2)
							num_paths[p.first][hop + 1] += num_paths[i][hop];

		for (int hop = 1; hop <= MAX_HOP; hop ++)
			for (int i = 0; i < num_columns; i ++)
				global_num_paths[s][i] += num_paths[i][hop];
	}

	ofstream fout("goodpairs.txt", ios::out);
	for (int i = 0; i < num_columns; i ++)
		for (int j = i + 1; j < num_columns; j ++)
			if (global_num_paths[i][j] > 1 && global_num_paths[i][j] < 5)
				fout << ids[i].first << " " << ids[i].second << endl << ids[j].first << " " << ids[j].second << endl << global_num_paths[i][j] << endl << endl;

	int start_table = 9;
	int start_column = 0;
	int end_table = 30;
	int end_column = 9;
	int start_node = -1;
	int end_node = -1;
	for (int i = 0; i < num_columns; i ++)
		if (start_table == ids[i].first && start_column == ids[i].second)
			start_node = i;
		else if (end_table == ids[i].first && end_column == ids[i].second)
			end_node = i;

	path_nodes.clear();
	path_nodes.push_back(start_node);
	tot = 0;
	dfs(start_node, end_node, 0, -1);
	cout << tot << endl;
	fout.close();
}

void PkfkGraph::dfs(int x, int goal, int step, int last)
{
	if (x == goal)
	{
		for (int x : path_nodes)
			cout << ids[x].first << " " << ids[x].second << endl;
		cout << endl;
		tot ++;
		return ;
	}

	if (step >= MAX_HOP)
		return ;

	for (auto p : adj[x])
	{
		if (p.second != last)
		{
			path_nodes.push_back(p.first);
			dfs(p.first, goal, step + 1, p.second);
			path_nodes.pop_back();
		}
	}
}
