/**************************************
 **** 2017-3-6      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/

#include <algorithm>    // std::sort
#include "Framework.h"
#include "common.h"


// ========================================================================
string Framework::get_f_name(string complete_file_name){
    size_t pos = 0;
    std::string delimiter = "/";
    std::string token;
    while ((pos = complete_file_name.find(delimiter)) != std::string::npos) {
        token = complete_file_name.substr(0, pos);
        // std::cout << token << std::endl;
        complete_file_name.erase(0, pos + delimiter.length());
    }
    return complete_file_name;
}

// ========================================================================

bool Framework::search_in_vector(vector<string> violations, string violating_string){
    for (int i = 0; i < violations.size(); i ++)
        if (violating_string == violations[i])
            return true;
    return false;
}

// ========================================================================

vector<string> Framework::detect_value_in_col(Table T, int idx, string value){
    string violation;
    vector<string> violations;
    string f_name = get_f_name(T.table_name);
    for (int i = 0; i < T.data.size(); i++){
        if (T.data[i][idx] == value){
            violation = f_name+","+std::to_string(i)+","+std::to_string(idx)+","+"0\n";
            violations.push_back(violation);
        }
    }
    return violations;
}

vector<string> Framework::check_data_type(Table T, int col, string type){
    string violation;
    vector<string> violations;
    string f_name = get_f_name(T.table_name);
    for (int i = 0; i < T.data.size(); i++){
        if (!isInteger(T.data[i][col])){
            violation = f_name+","+std::to_string(i)+","+std::to_string(col)+","+"0\n";
            violations.push_back(violation);
        }
    }
    return violations;
}



vector<string> Framework::check_FD_violations(Table T, int idx1, int idx2){
    string violation, violating_string;
    bool found = false;
    bool is_violation;
    vector<string> violating_strings;
    vector<string> violations;
    string f_name = get_f_name(T.table_name);
    for (int i = 0; i < T.data.size(); i++){
        // cerr << i << endl;
        violating_string = T.data[i][idx1];
        // violation = f_name+"\t"+std::to_string(i)+"\t"+std::to_string(idx1)+"\t"+"0\n";
        found = search_in_vector(violating_strings, violating_string);
        // cerr << found << endl;
        is_violation = false;
        if (!found){
            for (int j = i+1; j < T.data.size(); j ++){
                if ((T.data[i][idx1] == T.data[j][idx1])&&(T.data[i][idx2] != T.data[j][idx2])){
                    violating_strings.push_back(violating_string);
                    for (int k = 0; k < T.data.size(); k ++){
                        if (T.data[k][idx1] == violating_string){
                            violation = f_name+","+std::to_string(k)+","+std::to_string(idx1)+","+"0\n";
                            violations.push_back(violation);
                            violation = f_name+","+std::to_string(k)+","+std::to_string(idx2)+","+"0\n";
                            violations.push_back(violation);
                        }
                    }
                    break;
                }
            }
        }
    }
    // for (int kk = 0; kk < violating_strings.size(); kk++)
    //     cerr << violating_strings[kk] << endl;
    return violations;
}




vector<string> Framework::check_FD_violations(Table T1, Table T2, int idx1, 
                int idx2, int idx3, int idx4){
    string violation, violating_string;
    bool found = false;
    bool is_violation;
    vector<string> violating_strings;
    vector<string> violations;
    string f_name1 = get_f_name(T1.table_name);
    string f_name2 = get_f_name(T2.table_name);
    for (int i = 0; i < T1.data.size(); i++){
        // cerr << i << endl;
        violating_string = T1.data[i][idx1];
        // violation = f_name1+"\t"+std::to_string(i)+"\t"+std::to_string(idx1)+"\t"+"0\n";
        found = search_in_vector(violating_strings, violating_string);
        // cerr << found << endl;
        is_violation = false;
        if (!found){
            for (int j = 0; j < T2.data.size(); j ++){
                if ((T1.data[i][idx1] == T2.data[j][idx3])&&(T1.data[i][idx2] != T2.data[j][idx4])){
                    violating_strings.push_back(violating_string);
                    for (int k1 = 0; k1 < T1.data.size(); k1 ++){
                        if (T1.data[k1][idx1] == violating_string){
                            violation = f_name1+","+std::to_string(k1)+","+std::to_string(idx1)+","+"0\n";
                            violations.push_back(violation);
                            violation = f_name1+","+std::to_string(k1)+","+std::to_string(idx2)+","+"0\n";
                            violations.push_back(violation);
                        }
                    }
                    for (int k2 = 0; k2 < T2.data.size(); k2 ++){
                        if (T2.data[k2][idx3] == violating_string){
                            violation = f_name2+","+std::to_string(k2)+","+std::to_string(idx3)+","+"0\n";
                            violations.push_back(violation);
                            violation = f_name2+","+std::to_string(k2)+","+std::to_string(idx4)+","+"0\n";
                            violations.push_back(violation);
                        }
                    }
                    break;
                }
            }
        }
    }
    // for (int kk = 0; kk < violating_strings.size(); kk++)
    //     cerr << violating_strings[kk] << endl;
    return violations;
}

vector<string> Framework::check_ALL_FD_violations(Table T1, Table T2, int PK1, int PK2){
    std::vector<string> violations;
    std::vector<string> col_violations;
    int idx1, idx2;
    string f_name1 = get_f_name(T1.table_name);
    string f_name2 = get_f_name(T2.table_name);
    for (int i = 0; i < T1.header.size(); i++)
    {
        if (i == PK1)   continue;
        for (int j = 0; j < T2.header.size(); j++){
            if (j == PK2)   continue;
            if (T1.header[i] == T2.header[j])
            {    idx1 = i;       idx2 = j;     break; }
        }
        
        col_violations = check_FD_violations(T1, T2, PK1, idx1, PK2, idx2);
        for (int k = 0; k < col_violations.size(); k++){
            violations.push_back(col_violations[k]);
        }
        remove_elements_in(col_violations);
    }
    return violations;
}