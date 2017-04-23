import sys
import time
from ontomatch import glove_api
from ontomatch import matcher_lib as matcherlib
from ontomatch.matcher_lib import MatchingType
from ontomatch import ss_api as api


def get_recall_ratio(pos_matchings, neg_matchings, gt_fkc_file):
    total_fkc = 0
    number_found_pkfk = 0
    number_found_pkfk_in_pos = 0
    number_found_pkfk_in_neg = 0
    with open(gt_fkc_file) as f:
        for line in f.readlines():
            total_fkc +=1
            tokens = line.split(';')
            left = tokens[0]
            right = tokens[1]
            reverse_line = right+';'+left
            found_in_pos = False
            found_in_neg = False
            if line in pos_matchings or reverse_line in pos_matchings:
                number_found_pkfk_in_pos += 1
                found_in_pos = True
            if line in neg_matchings or reverse_line in neg_matchings:
                number_found_pkfk_in_neg += 1
                found_in_neg = True
            if found_in_pos or found_in_neg:
                number_found_pkfk += 1
            # else:
            #     print(line)

    print('total_fkc is ' + str(total_fkc))
    print('number_found_pkfk is ' + str(number_found_pkfk))
    print('number_found_pkfk_in_pos is ' + str(number_found_pkfk_in_pos))
    print('number_found_pkfk_in_neg is ' + str(number_found_pkfk_in_neg))


def curate_pkfk_edges(file_name):
    pos_matchings = []
    neg_matchings = []
    with open(file_name) as f:
        for line in f.readlines():
            # each line is table1, col1, table2, col2
            tokens = line.split(',')
            table1 = tokens[0]
            attribute1 = tokens[1]
            table2 = tokens[2]
            attribute2 = tokens[3]
            match = table1 + '@' + attribute1 + ';' + table2 + '@' + attribute2
            semantic_sim, neg_signal = matcherlib.find_negative_sem_signal_attr_sch_sch(attribute1, attribute2)
            if neg_signal and semantic_sim < 0.4:
                neg_matchings.append(match)
            else:
                pos_matchings.append(match)

    print('positive PKFK edges are ' + str(len(pos_matchings)))
    # for el in pos_matchings:
    #     print(el)

    print('negative PKFK edges are ' + str(len(neg_matchings)))
    for el in neg_matchings:
        print(el)
    return pos_matchings, neg_matchings


if __name__ == "__main__":

    # Load glove model
    print("Loading language model...")
    path_to_glove_model = "../glove/glove.6B.100d.txt"
    glove_api.load_model(path_to_glove_model)
    print("Loading language model...OK")

    s = time.time()
    # file will PKFK edges
    # pos_matchings, neg_matchings = curate_pkfk_edges("/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/eva_ssproject/chembl_22_PKFK674_397.csv")
    # pos_matchings, neg_matchings = curate_pkfk_edges("/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/eva_ssproject/chembl_22_PKFK674_298.csv")
    pos_matchings, neg_matchings = curate_pkfk_edges("/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/eva_ssproject/COMA_Correspondences_193_03.csv")
    # pos_matchings, neg_matchings = curate_pkfk_edges("/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/eva_ssproject/chembl_22_PKFK674.csv")
    # get_recall_ratio(pos_matchings, neg_matchings, "/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/eva_ssproject/chembl_FKC_Edges_all.txt")
    e = time.time()
    print("filtering FP took: " + str(e - s))
    exit()





