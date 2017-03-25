#ifndef ERRORMODEL__H
#define ERRORMODEL__H

#include "Corpus.h"
#include <vector>
#include <string>
#include <unordered_map>
typedef vector<pair<string, pair<int, pair<int, int>>>> cell_list_entry;
using namespace std;

class ErrorModel
{
public:
	ErrorModel() {}
	ErrorModel(Corpus *);
	vector<pair<string, string>> get_matching_pairs();
	virtual void find_erroneous_cells() = 0;

protected:
	Corpus *corpus;

	//variables for finding similar pks
	vector<vector<int>> adj;
	vector<bool> removedX, removedY, visitedX;
	vector<int> matchedX, matchedY;
	unordered_map<string, vector<int>> ys;
	const double diff_threshold = 0.001;
	const double edge_threshold = 0.5;
	const double string_column_sim_threshold = 0.4;
	const double numeric_column_sim_threshold = 0.7;
	const double freq_threshold = 0.51;

	//variables for transformations
	vector<pair<string, string>> matching_pairs;

	//functions
	double jaccard_sim(string, string);
	double column_sim(Table *, int, Table *, int);
	bool column_sim_exact(Table *, int, Table *, int);
	bool find_match(int);
	void print_erroneous_cell(Table *, int, int, cell_list_entry, bool);
};

class PKErrorModel : public ErrorModel
{
public:
	PKErrorModel() {}
	PKErrorModel(Corpus *);
	void find_erroneous_cells();

private:
	void find_similar_pks();
	vector<pair<int, int>> similar_pk_pairs;
	vector<vector<int>> matches;
};

class SimErrorModel : public ErrorModel
{
public:
	SimErrorModel() {}
	SimErrorModel(Corpus *);
	void find_erroneous_cells();
};

struct FD
{
	int table_id, c1, c2;
	FD() {}
	FD(int tid, int _c1, int _c2) : table_id(tid), c1(_c1), c2(_c2) {}
};

class FDErrorModel : public ErrorModel
{
public:
	FDErrorModel() {}
	FDErrorModel(Corpus *, string);
	void find_erroneous_cells();

private:
	vector<FD> fds;
};

#endif // ERRORMODEL_H
