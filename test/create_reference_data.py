import numpy as np
from sklearn.datasets import load_iris
from sklearn.decomposition import PCA
import os

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


def proprocessNorm(dat):
    n_points, n_dims = dat.shape
    # preprocessing: prep
    d_means = np.mean(dat, axis=0)
    d_mins = np.min(dat, axis=0)
    d_maxs = np.max(dat, axis=0)
    normFactors = d_maxs - d_mins

    # preprocessing: mean normalization
    dat_norm_mean = dat.copy()
    dat_norm_mean -= d_means

    # preprocessing: min-max normalization
    dat_norm_minmax = dat.copy()
    dat_norm_minmax -= d_mins

    # preprocessing: common
    for col in range(n_dims):
        if normFactors[col] < 0.0001:
            continue
        for row in range(n_points):
            dat_norm_mean[row, col] /= normFactors[col]
            dat_norm_minmax[row, col] /= normFactors[col]

    return dat_norm_mean, dat_norm_minmax


if __name__ == '__main__':
    
    # check of data directory exists
    if not os.path.isdir('data'):
        os.mkdir('data')

    # define data
    data = load_iris().data.astype(np.single)
    num_points, num_dims = data.shape

    print(f"Use sklearn digits dataset. \nNumber of points: {num_points} with {num_dims} dimensions each")
    print("Save iris data")
    saveAsBinary(data, 'data/iris_data')

    # preprocessing: mean normalization and min-max normalization
    data_norm_mean, data_norm_minmax = proprocessNorm(data)

    # Save data as binary to disk
    print("Save normalized data")
    saveAsBinary(data_norm_mean, 'data/iris_data_norm_mean')
    saveAsBinary(data_norm_minmax, 'data/iris_data_norm_minmax')

    # perform PCA for 2 and for all components
    print("Perform PCA and save to disk")

    settingsList = [[data, 'data/iris_pca', 'data/iris_trans'],
                    [data_norm_minmax, 'data/iris_pca_norm_minmax', 'data/iris_trans_norm_minmax'],
                    [data_norm_mean, 'data/iris_pca_norm_mean', 'data/iris_trans_norm_mean']]

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

    # preprocessing: mean normalization and min-max normalization
    X_norm_mean, X_norm_minmax = proprocessNorm(X)
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
