#ifndef PKFKGRAPH__H
#define PKFKGRAPH__H

#include <string>
#include <vector>
#include <utility>
#include <cstring>
using namespace std;

class PkfkGraph
{
public:
	PkfkGraph() {}
	PkfkGraph(string);
	void print_join_paths();

private:
	const int MAX_HOP = 4;
	int num_columns;
	vector<vector<pair<int, int>>> adj;
	vector<pair<int, int>> ids;
	vector<int> path_nodes;
	int tot;

	//functions
	void dfs(int, int, int, int);
};

#endif
