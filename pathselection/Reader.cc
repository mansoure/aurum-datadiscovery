#include "Reader.h"
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iostream>
#include <unordered_set>
#include <dirent.h>
#include <sys/stat.h>
using namespace std;

void Reader::strNormalize(string &s)
{
  /*
	for (int i = 0; i < (int) s.size(); i ++)
  		s[i] = tolower(s[i]);
  */

	//bad chars
	unordered_set<char> bad_chars;
	for (int i = 0; i < (int) strlen(chars); i ++)
		bad_chars.insert(chars[i]);
	string ss = "";
	for (int i = 0; i < (int) s.size(); i ++)
		if (! bad_chars.count(s[i]))
			ss += s[i];
		else if (i > 0 && ! bad_chars.count(s[i - 1]))
			ss += " ";
	s = ss;
}

long long CSVReader::filesize(const char* filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return (long long) in.tellg();
}

bool CSVReader::ends_with(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void CSVReader::csv_read_row(std::istream &in, std::vector<std::string> &row)
{
	std::stringstream ss;
	bool inquotes = false;
	while(in.good())
	{
		char c = in.get();
		if (!inquotes && c == '"') //beginquotechar
			inquotes = true;
		else if (inquotes && c == '"') //quotechar
		{
			if (in.peek() == '"') //2 consecutive quotes resolve to 1
				ss << (char)in.get();
			else //endquotechar
				inquotes = false;
		}
		else if (!inquotes && c == field_delimiter) //end of field
		{
			string temp_str = ss.str();
			strNormalize(temp_str);
			row.push_back(temp_str);
			ss.str("");
		}
		else if (!inquotes && (c == '\r' || c == '\n'))
		{
			if (in.peek() == '\n')  in.get();
			string temp_str = ss.str();
			strNormalize(temp_str);
			row.push_back(temp_str);
			return;
		}
		else
			ss << c;
	}
}

bool CSVReader::get_table(const string &filepath, vector<string> &headers, vector<vector<string>> &rows)
{
	ifstream in(filepath, ios::in);
	if (in.fail()) return (cout << "File not found: " + filepath << endl) && false;
	csv_read_row(in, headers);

	while (in.good())
	{
		vector<string> tuple;
		tuple.reserve(headers.size());
		csv_read_row(in, tuple);
		if (tuple.size() != headers.size()) continue;
		rows.push_back(tuple);
	}
	in.close();
	return true;
}

Table CSVReader::read_a_table(string directory_name, string file_name)
{
	Table bad_table = Table();

	if (! ends_with(file_name, ".csv"))
	{
		cerr << "WARNING: Skipped non-csv file " << file_name << endl;
		return bad_table;
	}

	if (filesize((directory_name + file_name).c_str()) > (long long) MAX_CSV_FILE_SIZE)
	{
		cerr << "WARNING: Skipped a large file " << file_name << endl;
		return bad_table;
	}

	vector<string> data_headers;
	vector<vector<string>> data_rows;

	if (! get_table(directory_name + file_name, data_headers, data_rows))
		return bad_table;
/*
	if ((int) data_headers.size() > max_num_col || (int) data_rows.size() > max_num_row)
	{
		cerr << "WARNING: Skipped a large file " << file_name << endl;
		return bad_table;
	}
*/
	return Table(0, file_name, data_headers, data_rows);
}
