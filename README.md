# PCA Plugin  [![Actions Status](https://github.com/ManiVaultStudio/ParallelCoordinatesPlugin/workflows/ParallelCoordinatesPlugin/badge.svg?branch=master)](https://github.com/ManiVaultStudio/ParallelCoordinatesPlugin/actions)

[Principal components analysis](https://en.wikipedia.org/wiki/Principal_component_analysis) plugin for the [ManiVault](https://github.com/ManiVaultStudio/core) visual analytics framework.

By default, the plugin internally centers the data so that each dimension/channel has zero mean.
Other normalization steps before this centering are possible as well: [Mean normalization](https://en.wikipedia.org/wiki/Feature_scaling#Mean_normalization) and [Rescaling (min-max normalization)](https://en.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)).
This plugin implements two PCA computation algorithms: Explicitly computing the eigenvectors of the covariance matrix and singular value decomposition.

Clone the repo with it's submodule (Eigen 3.4):
```
git clone --recurse-submodule https://github.com/ManiVaultStudio/PcaPlugin.git
```

## Testing
You can perform unit tests. Set the cmake variable UNIT_TESTS to build tests. To build the testing project, you'll need to install some further dependencies and create ground truth data; see `test/README.md`.

## TODO
- Use vcpkg to manage dependencies
