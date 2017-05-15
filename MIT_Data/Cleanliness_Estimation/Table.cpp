/**************************************
 **** 2017-4-23      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/

#include "Table.h"
#include <stdio.h>
Table::Table(const string& name) {
  table_name = name;
}

Table::Table(const string &name,
             const int rows, 
             const int cols,
             vector<string> __header,
             doubleVecStr __data
            )
{
  table_name = name;
  number_of_rows = rows;
  number_of_cols = cols;
  header = __header; 
  data = __data;
}

