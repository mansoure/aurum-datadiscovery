/**************************************
 **** 2017-4-23      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/

#include "common.h"

unsigned hash_str(const string word){
    #define A 54059 /* a prime */
    #define B 76963 /* another prime */
    #define C 86969 /* yet another prime */
    #define FIRSTH 37 /* also prime */

    unsigned h = FIRSTH;
    for(int i = 0; i < word.length(); i++) {
      h = (h * A) ^ (word[i] * B);
    }
    return h; // or return h % C;
}
string check_d_quotation(string str){
    std::size_t found = str.find(",");
    if ((found!=std::string::npos) || (str.empty()))
        return "\""+str+"\"";
    return str;
}

void print_vector(const vector<string> &V, std::fstream& fs){
//    cout << "[ " ;
    for (int i = 0; i < V.size() - 1; i++)
         fs << check_d_quotation(V[i]) << ',';
    fs << check_d_quotation(V[V.size()-1]) << "\n";
}

void trim(string& s)
{
	size_t p = s.find_first_not_of(" \t");
	s.erase(0, p);
	
	p = s.find_last_not_of(" \t");
	if (string::npos != p)
		s.erase(p+1);
}

void remove_elements_in(vector<string> &V){
    while(!V.empty()){
        V.pop_back();
    }
}

void print_double_vector(doubleVecStr V, std::fstream& fs){
//    print_vector(V[0]);
    for (int i = 0; i < V.size(); i++){
//      cout << "[ ";
//      for (int j = 0; j < V[i].size()-1; j ++){
//          cout << check_d_quotation(V[i][j]) << ", ";
//      }
//      cout << check_d_quotation(V[i][V[i].size()-1]) << endl;
        print_vector(V[i], fs);
  }
}


void print_line_of(char c){
    for (int i = 0; i < 30; i++)
        cout << c;
    cout << endl;
}
void print_line_of(char c, std::fstream& fs){
    for (int i = 0; i < 30; i++)
        fs << c;
    fs << endl;
}



bool isInteger(string st) {
    int len = st.length();
    for (int i = 0; i < len; i++) {
        if (int(st[i])<48 || int(st[i]) > 57) {
            return false;
        }
    }
    return true;   
}


bool isNumeric(string st) {
    int len = st.length(), ascii_code, decimal_count = -1, negative_count = -1;
    for (int i = 0; i < len; i++) {
        ascii_code = int(st[i]);
        switch (ascii_code) {
            case 44: // Allow commas.
                // This will allow them anywhere, so ",,," would return true.
                // Write code here to require commas be every 3 decimal places.
                break;
            case 45: // Allow a negative sign.
                negative_count++;
                if (negative_count || i != 0) {
                    return false;
                }
                break;
            case 46: // Allow a decimal point.
                decimal_count++;
                if (decimal_count) {
                    return false;
                }
                break;
            default:
                if (ascii_code < 48 || ascii_code > 57) {
                    return false;
                }
                break;
        }
    }
    return true;
}


