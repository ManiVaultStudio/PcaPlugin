# PCA Plugin

Performs a [principal components analysis](https://en.wikipedia.org/wiki/Principal_component_analysis).

By default, the plugin internally centers the data so that each dimension/channel has zero mean.
Other normalization steps before this centering are possible as well: [Mean normalization](https://en.wikipedia.org/wiki/Feature_scaling#Mean_normalization) and [Rescaling (min-max normalization)](https://en.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)).
This plugin implements two PCA computation algorithms: Explicitly computing the eigenvectors of the covariance matrix and singular value decomposition.

Make sure to fetch all submodules (Eigen 3.4):
```git clone --recurse-submodule https://github.com/hdps/PcaPlugin.git```

## Testing
You can perform unit tests. Set the cmake variable UNIT_TESTS to build tests. To build the testing project, you'll need to install some further dependencies and create ground truth data; see `test/README.md`.

## TODO
- Use vcpkg to manage dependencies