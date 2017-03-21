import pandas
from subprocess import call
import subprocess
from subprocess import DEVNULL, STDOUT, check_call
from subprocess import Popen
import os
from sklearn.externals import joblib

def format_join_pathsOneLine(res):
    JPlist = []
    idx = 0
    for path in res.paths():
        # myPath = "PATH " + str(idx)
        myPath = ""
        idx = idx + 1
        flag = 0
        for el in path:
            if flag == 1:
                myPath = myPath + ";"
            myPath = myPath + el.source_name + "@"+ el.field_name
            flag = 1
        JPlist.append(myPath)
        # print(myPath)
    return JPlist

def classifyJoinPaths(res):
# def classifyJoinPaths():
    filename = '/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/aurum-datadiscovery/ML_FKC_Detection/temp.csv'
    command = '/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/aurum-datadiscovery/ML_FKC_Detection/RHEEM_QuDS'
    # edges_list = ['research_companies.csv@res_stem_id;research_stem.csv@res_stem_id','binding_sites.csv@tid;target_dictionary.csv@tid']
    edges_list = format_join_pathsOneLine(res)

    try:
        os.remove(filename)
    except OSError:
        pass
    f1 = open(filename, 'a')
    f1.close()

    eID = 1
    for JP in edges_list:
        # print(eID, JP)
        Process = Popen([command, str(JP), str(eID), str(filename)], stdout=DEVNULL, stderr=STDOUT)
        Process.wait()
        # print(Process)
        eID = eID + 1

    range1 = [i for i in range(2,9)]
    names = ['uniquenessRatioN1', 'uniquenessRatioN2', 'colNameSimilarityRatio','isSubsetString', 'coverageRatio', 'averageLenDiffRatio', 'tableSizeRatio']
    testData = pandas.read_csv(filename, header=0, usecols=range1)
    outData = pandas.read_csv(filename, header=0, usecols=[1])
    loaddedRF = joblib.load('/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/aurum-datadiscovery/ML_FKC_Detection/FKCModel_RF_StratifiedKFold.pkl')
    prediction_proba = loaddedRF.predict_proba(testData)
    prediction = loaddedRF.predict(testData)

    iD = 1
    for i,j,k in zip(outData._values,prediction, prediction_proba):
        print("-", i, j, k)
        iD=iD+1



# if __name__ == "__main__":
#     classifyJoinPaths()

