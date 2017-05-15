/**************************************
 **** 2017-3-6      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/


#ifndef _Framework_
#define _Framework_



#include "Table.h"
#include <map>
#include <string.h>
#include <stdio.h>
#include <istream>
#include <sstream>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include "csv_reader.h"

class Framework {
private:
    // int max_num_terms_per_att = 10;
public:
    Framework(){ }
    bool search_in_vector(vector<string> violations, string str);
    vector<string> detect_value_in_col(Table, int, string);
    vector<string> check_data_type(Table, int, string);
    vector<string> check_FD_violations(Table, int, int);
    vector<string> check_FD_violations(Table, Table, int, int, int, int);
    vector<string> check_ALL_FD_violations(Table, Table, int, int);
    string get_f_name(string);
};
#endif