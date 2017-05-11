from itertools import tee
from pathselection import joinpathselection
from elasticsearch import Elasticsearch
import csv
import re
import pandas as pd
from main import init_system
from api.apiutils import Relation
from api.reporting import Report
from knowledgerepr import fieldnetwork
from ddapi import API
import os



HOST = 'localhost'
PORT = 9200
global es
es = Elasticsearch([{'host':HOST, 'port':PORT}])
    

cleanliness_directory = "Queries/"
cleaning_query_formatted = cleanliness_directory+'query.in'


def init_component(data_path, model_path):
    api, reporting = init_system(model_path)
    corpus = joinpathselection.Corpus(data_path)
    tables_dfs = read_tables(data_path)
    dirty_cells = read_dirty_cells()
    return (api, reporting, corpus, tables_dfs, dirty_cells)

def read_dirty_cells():
    dirty_cells = []
    error_est = cleanliness_directory + "error_est.in"
    # print(fname)
    with open(error_est, 'r') as f:
        lines = f.readlines()
        for l in lines:
            #print(l)
            tokens = re.compile("\s+").split(l)
            dirty_cells.append(tokens)
    return dirty_cells

def color_cell_red(val):
    color = 'red'
    return 'background-color: {}'.format(color)
    # return 'color: %s' % color

def format_df(df, dirty_cells):
    obj = df.style
    obj.highlight_null(null_color='Yellow')

    for rid, cid in dirty_cells:
        obj = obj.applymap(color_cell_red, subset=pd.IndexSlice[int(rid), df.columns[int(cid)]])
    return (obj)

def format_join_paths(res):
    idx = 0
    f_res = ""
    for path in res.paths():
        # print("PATH: " + str(idx))
        f_res += "PATH: " + str(idx) +"\n"
        idx = idx + 1
        for el in path:
            f_res += el.source_name + " - "+ el.field_name + "\n"
            # print(el.source_name + " - "+ el.field_name)
            # print(" ")
        f_res += "\n"
    return f_res

def joinPaths(api, table1, table2):
    drs_t1 = api.drs_from_table(table1)
    drs_t2 = api.drs_from_table(table2)
    drs_t1.set_table_mode()
    drs_t2.set_table_mode()
    res = api.paths_between(drs_t1, drs_t2, Relation.PKFK, max_hops=1)
    formated_JPs = format_join_paths(res)
    return res, formated_JPs

def get_the_path_of_table(table_name):
    res = es.search(index="profile", body={"from":0, "size":10,"query": {"match": {"sourceName": table_name}}})
    for doc in res['hits']['hits']:
        a = doc['_source']['sourceName'].lower()
        b = table_name.lower()
        c = table_name.lower()+'.csv'
        # print(a)
        if a == b or a == c:
            D_Path = doc['_source']['path']
            return D_Path, doc['_source']['sourceName']
    return [], table_name

def read_tables(data_path):
    tables_names = []
    tables_dfs = []
    for f_name in os.listdir(data_path):
        if f_name.endswith(".csv"):
            tables_names.append(f_name.lower())
            full_f_name = data_path+f_name
            df = load_table(full_f_name)
            tables_dfs.append(df)
    t_names = pd.DataFrame(tables_names)
    tables_dfs.append(t_names)
    return tables_dfs

def load_table_cleanliness(table_name, tables_dfs, dirty_cells):
    df = get_table(table_name, tables_dfs)
    tab_dirty_cells = []
    for tokens in dirty_cells:
        #print(tokens)
        if (tokens[0] == table_name):
            tab_dirty_cells.append([tokens[1], tokens[2]])
    return format_df(df, tab_dirty_cells)

def load_table(table_name):
    tab_content = pd.read_csv(filepath_or_buffer=table_name, delimiter=',', quoting=csv.QUOTE_ALL, doublequote=True)
    return tab_content

def get_table(table_name, tables_dfs):
    names = tables_dfs[len(tables_dfs)-1]
    idx = names[names[0]==table_name.lower()].index.tolist()
    return tables_dfs[idx[0]]

def transform_to_cleaning_input(api, drs, projections):
    def get_attr_index(attr_name, table_name):
        Data_Path = ""
        flag = 0
        res = es.search(index="profile", body={"from":0, "size":10,"query": {"match": {"sourceName": table_name}}})
        for doc in res['hits']['hits']:
            if doc['_source']['sourceName'].lower() == table_name.lower():
                Data_Path = doc['_source']['path']
                break
        thefile = Data_Path + str(table_name)
        with open(thefile, "r") as ins:
            headers = []
            for line in ins:
                headers.extend(line.split(','))
                break
        idx = 0
        for el in headers:
            if el == attr_name:
                return idx
            idx = idx + 1

    def pairwise(iterable):
        a, b = tee(iterable)
        next(b, None)
        return zip(a, b)

    def all_attributes_idx_of(table):
        drs = api.drs_from_table(table)
        attrs_to_project = projections[table]
        indexes = []
        for attr in attrs_to_project:
            attr_idx = get_attr_index(attr, table)
            indexes.append(attr_idx)
        return indexes

    lines = []
    idx = 0
    paths = drs.paths()
    total_paths = len(paths)
    lines.append(total_paths)
    lines.append(" ")
    for path in paths:
        tables_involved = []
        attributes = []
        a_indexes = []
        for el in path:
            dbname = el.db_name
            sourcename = el.source_name
            fieldname = el.field_name
            repeated = False
            for el in tables_involved:
                if el == sourcename:
                    repeated = True
            if repeated:
                continue
            tables_involved.append(sourcename)
            attributes.append(fieldname)
            a_indexes.append(get_attr_index(fieldname, sourcename))
        seen_tables = set()
        lines.append(len(set(tables_involved)))
        for t in tables_involved:
            if t not in seen_tables:
                lines.append(t)
            seen_tables.add(t)
        for a,b in pairwise(a_indexes):
            lines.append(str(a) + " " + str(b))
        first_table = 0
        last_table = len(seen_tables) - 1
        attrs_source = all_attributes_idx_of(tables_involved[0])
        attrs_target = all_attributes_idx_of(tables_involved[-1])
        total_projected_attrs = len(attrs_source) + len(attrs_target)
        lines.append(total_projected_attrs)
        for i in attrs_source:
            lines.append(str(first_table) + " " + str(i))
        for i in attrs_target:
            lines.append(str(last_table) + " " + str(i))
        lines.append(" ")
    #for l in lines:
    #    print(str(l))
    global cleaning_query_formatted
    f = open(cleaning_query_formatted, 'w')
    for l in lines:
        f.write(str(l) + '\n')
    f.close()


def format_paths_with_card_clean(res, Final_RES):
    Paths = res.split('\n')
    formatted_paths = []
    path = Paths[0]
    tokens = re.compile("\t+").split(path)
    record = []
    for ele in tokens:
        record.append(ele)
    formatted_paths.append(record)
    for i in range(len(Final_RES)):
        record = []
        path = Paths[i+1]
        tables_in_path = Final_RES[i]
    # for path in Paths:
        tokens = re.compile("\t+").split(path)
        record.append(tables_in_path)
        for ele in tokens[1:]:
            record.append(ele)
        formatted_paths.append(record)
    formatted_paths  = [x for x in formatted_paths if x != ['']]
    formatted_paths_dfs = pd.DataFrame(formatted_paths[1:], columns=formatted_paths[0])
    return formatted_paths_dfs
    # return formatted_paths

def new_format_join_paths(res):
    idx = 0
    Final_RES = []
    for path in res.paths():
        # print("PATH: " + str(idx))
        f_res = "[" + str(idx) +"]: "
        idx = idx + 1
        for el in path[:1]:
            f_res += el.source_name + " ("+ el.field_name + ") |><| " 
        el = path[len(path)-1]
        f_res += el.source_name + " ("+ el.field_name + ")"
        Final_RES.append(f_res) 
    return Final_RES

def join_path_selection(api, res, projections, corpus):
    global cleaning_query_formatted
    transform_to_cleaning_input(api, res, projections)
    Final_RES = new_format_join_paths(res)
    jps = joinpathselection.JPSelection(corpus, cleanliness_directory + "gt.in", cleanliness_directory + 'error_est.in', cleaning_query_formatted)
    # print(jps.select_join_path())
    JPS_String = jps.select_join_path()
    formatted_JPS = format_paths_with_card_clean(JPS_String, Final_RES)
    return jps, formatted_JPS

def not_empty_record(Rec):
    for el in Rec:
        if el.strip():
            return True
    return False


def clean_with_budget(jps, corpus, budget, jp_idx, dirty_cells):
    # print(jps.budget_clean(budget, jp_idx))
    output = jps.budget_clean(budget, jp_idx)
    outputs = output.split("^")
    records = outputs[1].split('\n')
    data = []
    record_size = 0
    first_record = 1;
    for record in records:
        Rec = record.split('\t')
        found = 0
        if (not_empty_record(Rec)):
            if first_record:
                first_record = 0
                Rec = record.split('\t')
                if (not_empty_record(Rec)):
                    data.append(Rec)
                    continue
            else:
                for i in range(len(dirty_cells)-1):
                    if (str(Rec[0]) == dirty_cells[i][0]) and (str(Rec[1]) == dirty_cells[i][1]) and (str(Rec[2]) == dirty_cells[i][2]):
                        data.append(Rec)
                        found = 1;
                        break
        
    if len(data) > 1:
        df=pd.DataFrame(data[1:], columns=data[0])
        return records, df
    else:
        print("Nothing needs to be cleaned ..")
        return records, []

def clean_with_budget_old(jps, corpus, budget, jp_idx):
    # print(jps.budget_clean(budget, jp_idx))
    output = jps.budget_clean(budget, jp_idx)
    outputs = output.split("^")
    records = outputs[1].split('\n')
    data = []
    record_size = 0
    first_record = 1;
    for record in records:
        Rec = record.split('\t')
        if (not_empty_record(Rec)):
            data.append(Rec)
             
    df=pd.DataFrame(data[1:], columns=data[0])
    return records, df
    
def peek_results(df, tables_dfs):
    B = []
    for el in df[df.columns[0]]:
        if not(el in B):
            B.append(el)
    tables = []
    dfs = []
    for el in B:
        # print(el)
        tab_content = get_table(el, tables_dfs)
        tables.append(tab_content)
        header = []
        for ee in tab_content.columns:
            header.append(ee)
        header = ["recordID"] + header
        data = []
        fetched_rows = []
        for i in range(len(df[df.columns[0]])):
            if (df[df.columns[0]][i] == el):
                row_number = int(df[df.columns[1]][i])
                if not(row_number in fetched_rows):
                    row = tab_content.ix[row_number]
                    record = []
                    for ele in row : 
                        record.append(ele)
                    fetched_rows.append(row_number)
                    new_row = [str(row_number)]+record
                    data.append(new_row)
        new_df = pd.DataFrame(data, columns=header)
        indexed_df = new_df.set_index("recordID")
        dfs.append(indexed_df)
    return dfs

def save_modifications(df, mod_dfs, tables_dfs, data_path, dc):
    B = []
    for el in df[df.columns[0]]:
        if not(el in B):
            B.append(el)
    tables = []
    dfs = []
    output_path = data_path+"modified_files/"
    if not(os.path.exists(output_path)):
        os.makedirs(output_path)
    names = tables_dfs[len(tables_dfs)-1]
    for i in range(len(B)):
        table_name = B[i]
        full_table_name = output_path+table_name
        idx = names[names[0]==table_name.lower()].index.tolist()[0]
        mod_df = mod_dfs[i]
        record_ids = mod_df.index.values
        for j in record_ids:
            for col_name in mod_df.columns:
                new_value = mod_df[col_name][j]
                tables_dfs[idx].ix[int(j), col_name] = new_value
        tables_dfs[idx].to_csv(full_table_name, index=False)
    dc.drop((dc[dc.columns[0]][0].lower() == col_name) and ((dc[dc.columns[1]] == "390")) and ((dc[dc.columns[1]] == '9')))


if __name__ == "__main__":
    print("Running as main...")
    
