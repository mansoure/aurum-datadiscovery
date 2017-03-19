import pandas
from subprocess import call
from sklearn.externals import joblib

url = "/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/fkc_labeled_data/chembl_edges_test.csv"
names = ['uniquenessRatioN1', 'uniquenessRatioN2', 'colNameSimilarityRatio','isSubsetString', 'coverageRatio', 'averageLenDiffRatio', 'tableSize']
onames = ['Table1',	'Table2',	'Column1',	'Column2']
# testData = pandas.read_csv(url, header=0, usecols=names)
#
# outData = pandas.read_csv(url, header=0, usecols=onames)
#
# loaddedRF = joblib.load('FKCModel_RF_StratifiedKFold.pkl')
#
# prediction_proba = loaddedRF.predict_proba(testData)
# prediction = loaddedRF.predict(testData)
# # print(prediction_proba)

filename = 'temp.csv'
try:
    os.remove(filename)
except OSError:
    pass
f1 = open(filename, 'a')
f1.close()


edges_list = ['compound_records.csv@doc_id;docs.csv@doc_id','binding_sites.csv@tid;target_dictionary.csv@tid']
eID = 1

for el in edges_list:
    print(eID, el)
    call()
    eID = eID + 1




# call()


# for el in prediction:
#     print(el)
#
# for row in outData._values :
#     print(row)

iD = 1

# for i,j in zip(outData._values,prediction):
#     print(iD, i, j)
#     iD=iD+1



