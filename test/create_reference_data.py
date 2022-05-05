import numpy as np
from sklearn.datasets import load_digits
from sklearn.decomposition import PCA
import os

# check of data directory exists
if not os.path.isdir('data'):
    os.mkdir('data')

# define data
data = load_digits().data
num_points, num_dims = data.shape

print(f"Use sklearn digits dataset. \nNumber of points: {num_points} with {num_dims} dimensions each")
print("Save data")
with open('data/data', 'wb') as f:
    np.save(f, data)

# preprocessing: prep
means = np.mean(data, axis=0)
mins = np.max(data, axis=0)
maxs = np.min(data, axis=0)
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
with open('data/data_norm_minmax', 'wb') as f:
    np.save(f, data_norm_mean)
with open('data/data_norm_mean', 'wb') as f:
    np.save(f, data_norm_minmax)

# perform PCA for 2 and for all components
print("Perform PCA and save to disk")
for num_comps in [2, num_dims]:
    print(f"Components: {num_comps}")
    # PCA
    pca_norm_mean = PCA(n_components=num_comps)
    pca_norm_mean.fit(data_norm_mean)

    pca_norm_minmax = PCA(n_components=num_comps)
    pca_norm_minmax.fit(data_norm_minmax)

    # Save pca as binary to disk
    with open(f'data/pca_norm_minmax_{num_comps}', 'wb') as f:
        np.save(f, pca_norm_mean.components_)
    with open(f'data/pca_norm_mean_{num_comps}', 'wb') as f:
        np.save(f, pca_norm_minmax.components_)

    # PCA on raw data
    pca_raw = PCA(n_components=num_comps)
    pca_raw.fit(data)

    # Save pca as binary to disk
    with open(f'data/pca_raw_{num_comps}', 'wb') as f:
        np.save(f, pca_raw.components_)

    # Transform data
    print("Transform...")
    trans_norm_mean = np.matmul(data_norm_mean, pca_norm_mean.components_.T)
    trans_norm_minmax = np.matmul(data_norm_minmax, pca_norm_minmax.components_.T)
    trans_raw = np.matmul(data, pca_raw.components_.T)

    with open(f'data/trans_norm_mean_{num_comps}', 'wb') as f:
        np.save(f, trans_norm_mean)
    with open(f'data/trans_norm_minmax_{num_comps}', 'wb') as f:
        np.save(f, trans_norm_minmax)
    with open(f'data/trans_raw_{num_comps}', 'wb') as f:
        np.save(f, trans_raw)

print("Done.")
