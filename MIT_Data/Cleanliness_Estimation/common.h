/**************************************
 **** 2017-4-23      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/
  
#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <iomanip>      // std::setprecision
//#include <pthread.h>
#include <numeric>

#ifdef __APPLE__
#include <unordered_map>
#include <unordered_set>
#else
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#endif

using namespace std;
#ifndef __APPLE__
using namespace tr1;
#endif
typedef vector<vector<string> > doubleVecStr;
struct item{
    string name;
    int frequency;
};
bool isNumeric(string);
bool isInteger(string);
unsigned hash_str(const string word);
void remove_elements_in(vector<string> &V);
//void print_vector(vector<double> V);
void trim(string& s);
void print_double_vector(doubleVecStr V, std::fstream& fs);
//void print_double_vector_2(vector<vector<string> > V, const vector<double> &F);
void print_line_of(char c);
void print_line_of(char c, std::fstream& fs);
string check_d_quotation(string str);
void print_vector(const vector<string> &V, std::fstream& fs);
template <typename T>
vector<size_t> sort_descend_indexes(const vector<T> &v) {

  // initialize original index locations
  vector<size_t> idx(v.size());
  std::iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

  return idx;
}

#endif