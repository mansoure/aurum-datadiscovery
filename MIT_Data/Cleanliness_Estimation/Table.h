/**************************************
 **** 2017-4-23      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/

#ifndef _DATA_TABLE_
#define _DATA_TABLE_
#include "common.h"
class Table {
public:
  int number_of_rows, number_of_cols;
  string table_name;
  vector<string> header;
  doubleVecStr data;
  
  Table() {}
  Table (const string &csv_file_name);
  Table (const string &name, 
          const int rows, 
          const int cols,
          vector<string> header, 
          doubleVecStr data
          );
};
#endif
