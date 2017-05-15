/**************************************
 **** 2017-4-23      ******************
 **** Abdulhakim Qahtan ****************
 **** aqahtan@hbku.edu.qa ****************
 ***************************************/


#include "csv_reader.h"
#include "Framework.h"
#include "common.h"

void print_column_data(Table tab, string att){
    int idx = 0;
    string out_f_name = att + ".txt";
    fstream ofs(out_f_name, ios::out);
    if (!ofs.good()){
        cerr << "Problem opening output file .... \n";
        return;
    }
    for (int i = 0; i < tab.number_of_cols; i++){
        if (tab.header[i] == att){
            idx = i;
            break;
        }
    }
    for (int i = 0; i < tab.number_of_rows; i++){
        if (!tab.data[i][idx].empty())
            ofs << tab.data[i][idx] << endl;
    }
    ofs.close();
}



int main(int argc, char ** argv) {
    
    if (argc != 2)
    {
      cout << "Wrong number of arguments .. entered (" << argc << ") \n";
      for (int k = 0; k < argc; k++)
        cerr << argv[k] << endl;
      cout << "Usage (" << argv[0] << "): <data directory>"
              "\n\n";
      return 1;
    }
    string path = string(argv[1]);

    string file_name = "Buildings.csv";
    string full_file_name = path + file_name;
    
    CSV_READER *dataReader = new CSV_READER();  
    Table T = dataReader->read_csv_file(full_file_name);
    cout << "Table name,row number,column number,Cleanliness\n";
    Framework * dvdFramework = new Framework();
    // cout << dvdFramework->get_f_name(T.table_name) << endl;

    vector<string> vios = dvdFramework->detect_value_in_col(T, 6, "0");
    for (int k = 0; k < vios.size(); k++){
        cout << vios[k];
    }
    
    file_name = "Employee_Directory.csv";
    full_file_name = path + file_name;
    Table T1 = dataReader->read_csv_file(full_file_name);
    vios = dvdFramework->detect_value_in_col(T1, 7, "9999999999");
    for (int k = 0; k < vios.size(); k++){
        cout << vios[k];
    }
    vios = dvdFramework->detect_value_in_col(T1, 7, "1111111111");
    for (int k = 0; k < vios.size(); k++){
        cout << vios[k];
    }
    vios = dvdFramework->check_data_type(T1, 7, "Integer");
    for (int k = 0; k < vios.size(); k++){
        cout << vios[k];
    }

    file_name = "Drupal_employee_Directory.csv";
    full_file_name = path + file_name;
    Table T2 = dataReader->read_csv_file(full_file_name);
    vios = dvdFramework->detect_value_in_col(T2, 10, "9999999999");
    for (int k = 0; k < vios.size(); k++){
        cout << vios[k];
    }
    vios = dvdFramework->detect_value_in_col(T2, 10, "1111111111");
    for (int k = 0; k < vios.size(); k++){
        cout << vios[k];
    }
    vios = dvdFramework->check_data_type(T2, 10, "Integer");
    for (int k = 0; k < vios.size(); k++){
        cout << vios[k];
    }

    vios = dvdFramework->check_ALL_FD_violations(T1, T2, 0, 0);
    for (int k = 0; k < vios.size(); k++){
        cout << vios[k];
    }

    
}
