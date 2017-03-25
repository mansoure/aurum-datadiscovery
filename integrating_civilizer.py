from itertools import tee
from pathselection import joinpathselection
from integrating_civilizer import *
import csv

def transform_to_cleaning_input(api, drs, projections):
    def get_attr_index(attr_name, table_name):
        thefile = "/Users/emansour/elab/DAGroup/DataCivilizer/data/cleaningTest/" + str(table_name)
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
        print(path)
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
    f = open('/Users/emansour/elab/DAGroup/DataCivilizer/Demo/cidr-demo/datasets/q1.in', 'w')
    for l in lines:
        f.write(str(l) + '\n')
    f.close()

def format_join_paths(res):
    idx = 0
    for path in res.paths():
        print("PATH: " + str(idx))
        idx = idx + 1
        for el in path:
            print(el.source_name + " - "+ el.field_name)
            print(" ")

def format_join_pathsOneLine(res):
    idx = 0
    for path in res.paths():
        # myPath = "PATH " + str(idx)
        myPath = ""
        idx = idx + 1
        flag = 0
        for el in path:
            if flag == 1:
                myPath = myPath + "; "
            myPath = myPath + el.source_name + "@"+ el.field_name
            flag = 1
        print(myPath)

def join_path_selection(api, res, projections):
    transform_to_cleaning_input(api, res, projections)
    corpus = joinpathselection.Corpus("/Users/emansour/elab/DAGroup/DataCivilizer/data/cleaningTest/")
    jps = joinpathselection.JPSelection(corpus, "/Users/emansour/elab/DAGroup/DataCivilizer/Demo/cidr-demo/datasets/gt.in", "/Users/emansour/elab/DAGroup/DataCivilizer/Demo/cidr-demo/datasets/error_est.in", "/Users/emansour/elab/DAGroup/DataCivilizer/Demo/cidr-demo/datasets/q1.in")
    print(jps.select_join_path())
    return (corpus, jps)

def clean_with_budget(jps, corpus, budget, jp_idx):
    # print(jps.budget_clean(budget, jp_idx))
    output = jps.budget_clean(budget, jp_idx)
    outputs = output.split("^")
    print(outputs[1])
    return outputs[0]

def peek_results(limit, handle):
    rows = handle.split("\n")
    cnt = 0
    print("Full Name\tOffice Phone\tPrimary Title\tDepartment Name\tIs Degree Granting\tSchool Name")
    for line in rows:
      if cnt > limit:
        break
      cnt = cnt + 1
      print(line)


if __name__ == "__main__":
    print("Running as main...")

