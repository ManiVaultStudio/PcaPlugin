import numpy as np
from sklearn.datasets import load_digits, load_iris
from sklearn.decomposition import PCA
import os

# check of data directory exists
if not os.path.isdir('data'):
    os.mkdir('data')


def saveMetaInfoAsJson(info, filename):
    import json
    print(f"Save meta data to {filename}.json")
    with open(filename + ".json", "w") as f:
        json.dump(info, f, indent=4)


# define save function
def saveAsBinary(dataToSave, filename, type=np.single):
    print(f"Save data to {filename}.bin")
    dataToSave.astype(type).tofile(filename + ".bin")
    saveMetaInfoAsJson({"Binary file": filename + ".bin", "Data points": dataToSave.shape[0], "Dimensions": dataToSave.shape[1], "dtype": type.__name__}, filename)


# define data
data = load_iris().data.astype(np.single)
num_points, num_dims = data.shape

print(f"Use sklearn digits dataset. \nNumber of points: {num_points} with {num_dims} dimensions each")
print("Save data")
saveAsBinary(data, 'data/data')

# preprocessing: prep
means = np.mean(data, axis=0)
mins = np.min(data, axis=0)
maxs = np.max(data, axis=0)
normFacs = maxs - mins

# preprocessing: mean normalization
data_norm_mean = data.copy()
data_norm_mean -= means

# preprocessing: min-max normalization
data_norm_minmax = data.copy()
data_norm_minmax -= mins

# preprocessing: common
for col in range(num_dims):
    if normFacs[col] < 0.0001:
        continue
    for row in range(num_points):
        data_norm_mean[row, col] /= normFacs[col]
        data_norm_minmax[row, col] /= normFacs[col]

# Save data as binary to disk
print("Save normalized data")
saveAsBinary(data_norm_mean, 'data/data_norm_mean')
saveAsBinary(data_norm_minmax, 'data/data_norm_minmax')

# perform PCA for 2 and for all components
print("Perform PCA and save to disk")

settingsList = [[data, 'data/pca_raw', 'data/trans_raw'],
                [data_norm_minmax, 'data/pca_norm_minmax', 'data/trans_norm_minmax'],
                [data_norm_mean, 'data/pca_norm_mean', 'data/trans_norm_mean']]

for dat, pca_save_path, trans_save_path in settingsList:
    for num_comps in [2, num_dims]:
        print(f"Components: {num_comps}")
        # PCA
        pca = PCA(n_components=num_comps, svd_solver='full')
        pca.fit(dat)

        # Save pca as binary to disk
        saveAsBinary(pca.components_.T, f'{pca_save_path}_{num_comps}')

        # Transform data
        print("Transform...")
        trans = pca.transform(dat)
        saveAsBinary(trans, f'{trans_save_path}_{num_comps}')


#######
# sklearn example
# https://scikit-learn.org/1.1/modules/generated/sklearn.decomposition.PCA.html
#######
print("sklearn example")
X = np.array([[-1, -1], [-2, -1], [-3, -2], [1, 1], [2, 1], [3, 2]], dtype=float)

# preprocessing: prep
X_means = np.mean(X, axis=0)
X_mins = np.min(X, axis=0)
X_maxs = np.max(X, axis=0)
X_normFacs = X_maxs - X_mins

# preprocessing: mean normalization
X_norm_mean = X.copy()
X_norm_mean -= X_means

# preprocessing: min-max normalization
X_norm_minmax = X.copy()
X_norm_minmax -= X_mins

saveAsBinary(X, 'data/sklearn_data')
saveAsBinary(X_norm_mean, 'data/sklearn_data_norm_mean')
saveAsBinary(X_norm_minmax, 'data/sklearn_data_norm_minmax')

pca = PCA(n_components=2)
pca.fit(X)
saveAsBinary(pca.components_.T, f'data/sklearn_pca')
saveAsBinary(pca.transform(X), f'data/sklearn_trans')

pca = PCA(n_components=2)
pca.fit(X_norm_mean)
saveAsBinary(pca.components_.T, f'data/sklearn_pca_norm_mean')
saveAsBinary(pca.transform(X_norm_mean), f'data/sklearn_trans_norm_mean')

pca = PCA(n_components=2)
pca.fit(X_norm_minmax)
saveAsBinary(pca.components_.T, f'data/sklearn_pca_norm_minmax')
saveAsBinary(pca.transform(X_norm_minmax), f'data/sklearn_trans_norm_minmax')

#######
# numpy example
# https://scikit-learn.org/1.1/modules/generated/sklearn.decomposition.PCA.html
#######
# define data
X2 = np.array([[1, 1, 3], [2, 1, 4], [-3, 2, 0], [0.1, 0.5, 0.8], [2, 1, 1], [4, 2, 3]])
# center data
C = X2 - np.mean(X2, axis=0)
# covariance matrix of centered data
V = np.cov(C.T)
# eigendecomposition of covariance matrix
_, evecs = np.linalg.eig(V)
print(evecs)
# project data
P = evecs.T.dot(C.T)
print(P.T)

# numpy vs sklearn check
pca = PCA(n_components=3)
pca.fit(X2)

P2 = pca.transform(X2)

assert np.all(np.abs(P2) - np.abs(P.T) < 0.001)

print("Done.")
