# Load libraries
import pandas
from pandas.tools.plotting import scatter_matrix
import matplotlib.pyplot as plt
from sklearn import model_selection
from sklearn.metrics import classification_report
from sklearn.metrics import confusion_matrix
from sklearn.metrics import accuracy_score
from sklearn.model_selection import StratifiedShuffleSplit
from sklearn.model_selection import StratifiedKFold
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import RandomForestClassifier
from sklearn.tree import DecisionTreeClassifier
from sklearn.neighbors import KNeighborsClassifier
from sklearn.discriminant_analysis import LinearDiscriminantAnalysis
from sklearn.ensemble import GradientBoostingRegressor
from sklearn.naive_bayes import GaussianNB
from sklearn.svm import SVC

from sklearn.externals import joblib



# Load dataset
url = "/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/fkc_labeled_data/chembl_all_edges_features.csv"
names = ['uniquenessRatioN1', 'uniquenessRatioN2', 'colNameSimilarityRatio','isSubsetString', 'coverageRatio', 'averageLenDiffRatio', 'tableSize', 'class']

dataset = pandas.read_csv(url, header=0, usecols=names)

# shape
# print(dataset.shape)

# head
# print(dataset.head(5))

# descriptions
# print(dataset.describe())

# class distribution
print(dataset.groupby('class').size())

# box and whisker plots
dataset.plot(kind='box', subplots=True, layout=(7,7), sharex=False, sharey=False)
# plt.show()

# histograms
dataset.hist()
# plt.show()

# scatter plot matrix
scatter_matrix(dataset)
# plt.show()

# Split-out validation dataset
array = dataset.values
X = array[:,0:7]
Y = array[:,7]
print(X)
validation_size = 0.15
seed = 1234
# X_train, X_validation, Y_train, Y_validation = model_selection.train_test_split(X, Y, test_size=validation_size, random_state=seed)

# skf = StratifiedShuffleSplit(n_splits=7, test_size=0.9, random_state=seed)
# skf.get_n_splits(X, Y)
#
# # print(sss)
#
# for train_index, test_index in skf.split(X, Y):
#    # print("TRAIN:", train_index, "TEST:", test_index)
#    X_train, X_validation = X[train_index], X[test_index]
#    Y_train, Y_validation = Y[train_index], Y[test_index]


skf = StratifiedKFold(n_splits=7, random_state=seed)
skf.get_n_splits(X, Y)

print(skf)

for train_index, test_index in skf.split(X, Y):
   # print("TRAIN:", train_index, "TEST:", test_index)
   X_train, X_validation = X[train_index], X[test_index]
   Y_train, Y_validation = Y[train_index], Y[test_index]


# Test options and evaluation metric
seed = 1234
scoring = 'accuracy'


# Set-up the test harness to use 10-fold cross validation
# Spot Check Algorithms
models = []
models.append(('GBM', GradientBoostingRegressor()))
models.append(('NB', GaussianNB()))
models.append(('SVM', SVC()))
models.append(('LDA', LinearDiscriminantAnalysis()))
models.append(('LR', LogisticRegression()))
models.append(('KNN', KNeighborsClassifier()))
models.append(('CART', DecisionTreeClassifier()))
models.append(('RF', RandomForestClassifier()))
# evaluate each model in turn
results = []
names = []
for name, model in models:
	# kfold = model_selection.KFold(n_splits=10, random_state=seed)
	# cv_results = model_selection.cross_val_score(model, X_train, Y_train, cv=skf, scoring=scoring)
	cv_results = model_selection.cross_val_score(model, X_train, Y_train, cv=skf)
	results.append(cv_results)
	names.append(name)
	msg = "%s: %f (%f)" % (name, cv_results.mean(), cv_results.std())
	print(msg)

# Make predictions on validation dataset
knn = KNeighborsClassifier()
knn.fit(X_train, Y_train)
predictions = knn.predict(X_validation)
print(accuracy_score(Y_validation, predictions))
print(confusion_matrix(Y_validation, predictions))
print(classification_report(Y_validation, predictions))

# Make predictions on validation dataset
CART = DecisionTreeClassifier()
CART.fit(X_train, Y_train)

#save(CART,file="abc")
predictions = CART.predict(X_validation)
print(accuracy_score(Y_validation, predictions))
print(confusion_matrix(Y_validation, predictions))
print(classification_report(Y_validation, predictions))

# Make predictions on validation dataset
RF = RandomForestClassifier()
RF.fit(X_train, Y_train)

joblib.dump(RF, 'FKCModel_RF_StratifiedKFold.pkl')

loaddedRF = joblib.load('FKCModel_RF_StratifiedKFold.pkl')

predictions = loaddedRF.predict(X_validation)
print(accuracy_score(Y_validation, predictions))
print(confusion_matrix(Y_validation, predictions))
print(classification_report(Y_validation, predictions))


url = "/Users/emansour/elab/DAGroup/DataCivilizer/Aurum-GitHub/fkc_labeled_data/chembl_edges_test.csv"
names = ['uniquenessRatioN1', 'uniquenessRatioN2', 'colNameSimilarityRatio','isSubsetString', 'coverageRatio', 'averageLenDiffRatio', 'tableSize']

testData = pandas.read_csv(url, header=0, usecols=names)

prediction_proba = loaddedRF.predict_proba(testData)
print(prediction_proba)




#load(CART,file="abc")

