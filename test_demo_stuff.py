from main import init_system
from ML_FKC_Detection import FKC_Predication
# from pathselection import joinpathselection
# from integrating_civilizer import *
# from ML_FKC_Detection import FKC_Predication
from api.apiutils import Relation
api, reporting = init_system("/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/aurum-datadiscovery/models/mitdw/")



# t1 = "research_companies.csv"
# t2 = "research_stem.csv"

def joinPaths(table1, table2):
    drs_t1 = api.drs_from_table(table1)
    drs_t2 = api.drs_from_table(table2)
    drs_t1.set_table_mode()
    drs_t2.set_table_mode()
    res = api.paths_between(drs_t1, drs_t2, Relation.PKFK, max_hops=1)
    # res = api.paths_between(drs_t1, drs_t2, Relation.CONTENT_SIM, max_hops=1)
    return res

if __name__ == "__main__":
    # reporting.print_content_sim_relations()
    # PKFK = reporting.print_pkfk_relations()
    # res = joinPaths(t1, t2)
    # FKC_Predication.classifyJoinPaths(res)
    # t1 = "Employee_directory.csv"
    # t2 = "Sis_department.csv"
    # res = joinPaths(t1, t2)
    # # format_join_paths(res)

    t1 = "Drupal_employee_directory.csv"
    t2 = "Employee_directory.csv"
    res = joinPaths(t1, t2)
    FKC_Predication.classifyJoinPaths(res)

