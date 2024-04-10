# PCA Plugin  [![Actions Status](https://github.com/ManiVaultStudio/ParallelCoordinatesPlugin/workflows/ParallelCoordinatesPlugin/badge.svg?branch=master)](https://github.com/ManiVaultStudio/ParallelCoordinatesPlugin/actions)

[Principal components analysis](https://en.wikipedia.org/wiki/Principal_component_analysis) plugin for the [ManiVault](https://github.com/ManiVaultStudio/core) visual analytics framework.

<p align="middle">
  <img src="https://github.com/ManiVaultStudio/PcaPlugin/assets/58806453/91310fdd-641c-44b4-855b-8c3fea814eb1" align="middle" width="20%" />
  <img src="https://github.com/ManiVaultStudio/PcaPlugin/assets/58806453/b22676ca-9e2b-4932-87da-ef2d7f3efc67" align="middle" width="51%" /> </br>
  Settings UI (left) and several PCA components (right: 1&2, 3&4, 5&6)
</p>
<!-- PCA and t-SNE screenshot: 58806453/f9f19920-f1a4-41be-9ce4-ff8239aa6c3b -->

Clone the repo with it's submodule (Eigen 3.4):
```
git clone --recurse-submodule https://github.com/ManiVaultStudio/PcaPlugin.git
```

## Settings
- Preprocessing:
  - By default, the plugin internally centers the data so that each dimension/channel has zero mean.
  - Optional normalization steps before this centering: [Mean normalization](https://en.wikipedia.org/wiki/Feature_scaling#Mean_normalization) and [Rescaling (min-max normalization)](https://en.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)).
- PCA computation algorithms (implemented with [Eigen](https://gitlab.com/libeigen/eigen/)):
  - Explicitly computing the [eigenvectors of the covariance matrix](https://en.wikipedia.org/wiki/Principal_component_analysis#Covariances)
  - [Singular value decomposition](https://en.wikipedia.org/wiki/Principal_component_analysis#Singular_value_decomposition)
- Number of components:
  - Defaults to two

## Testing
You can perform unit tests. Set the cmake variable UNIT_TESTS to build tests. To build the testing project, you'll need to install some further dependencies and create ground truth data; see `test/README.md`.
