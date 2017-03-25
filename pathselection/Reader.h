#include "Corpus.h"
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

#define MAX_CSV_FILE_SIZE 102400000

class Reader {
private:
//	const char chars[31] = "-.\t'&,~!@#$%^&*()_=+<>?:;\"[]{}";
	const char chars[33] = "\t'&,~!@#$%^&*()_=+<>?:;\"[]{}-/\\ ";

public:
	void strNormalize(string &s); // also for the use of query normalization
	virtual Table read_a_table(string, string) = 0;
    virtual ~Reader() {}
};

class CSVReader : public Reader
{
private:
	const char field_delimiter = ',';
	const int max_num_row = 100000000;
	const int max_num_col = 100;
	long long filesize(const char* filename);
	bool ends_with(std::string const & value, std::string const & ending);
	void csv_read_row(std::istream &in, std::vector<std::string> &row);
	bool get_table(const string &filepath, vector<string> &headers, vector<vector<string>> &rows);
public:
	Table read_a_table(string, string);
};
