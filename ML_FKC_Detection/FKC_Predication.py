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
    print("Calculating the features")
    for JP in edges_list:
        print("... ... ", end="")
        # print(eID, JP)
        # Process = Popen([command, str(JP), str(eID), str(filename)], stdout=DEVNULL, stderr=STDOUT)
        Process = Popen([command, str(JP), str(eID), str(filename)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        Process.wait()
        # print(Process)
        eID = eID + 1

    print("")

    range1 = [i for i in range(2,9)]
    names = ['uniquenessRatioN1', 'uniquenessRatioN2', 'colNameSimilarityRatio','isSubsetString', 'coverageRatio', 'averageLenDiffRatio', 'tableSizeRatio']
    testData = pandas.read_csv(filename, header=0, usecols=range1)
    outData = pandas.read_csv(filename, header=0, usecols=[1])
    loaddedRF = joblib.load('/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/aurum-datadiscovery/ML_FKC_Detection/FKCModel_RF_StratifiedKFold.pkl')
    prediction_proba = loaddedRF.predict_proba(testData)
    prediction = loaddedRF.predict(testData)

    iD = 1
    res = []
    print("{0:95} {1:10} {2:12}".format("Join Path", "Class", "Propablity"))
    # print ("Join Path                                   ,   Class,      Propablity")
    for i,j,k in zip(outData._values,prediction, prediction_proba):
        jp = str(i[0])
        if j == 1:
            predication = "Real PKFK"
        elif j==2:
            predication = "Recommended"
        elif j==3:
            predication = "Not Useful"
        else:
            predication = "NA"

        prob = str(k)
        output = "{0:95} {1:10} {2:12}".format(jp, predication, prob)
        res.append([jp, predication, prob])
        print(output)
        iD=iD+1
    # return res



# if __name__ == "__main__":
#     classifyJoinPaths()

