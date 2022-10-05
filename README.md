# PCA Plugin

Performs a [principal components analysis](https://en.wikipedia.org/wiki/Principal_component_analysis).

By default, the plugin internally centers the data so that each dimension/channel has zero mean.
Other normalization steps before this centering are possible as well: [Mean normalization](https://en.wikipedia.org/wiki/Feature_scaling#Mean_normalization) and [Rescaling (min-max normalization)](https://en.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)).
This plugin implements two PCA computation algorithms: Explicitly computing the eigenvectors of the covariance matrix and singular value decomposition.

## Dependencies

- Eigen 3.4: Setup [Eigen](https://gitlab.com/libeigen/eigen) as described in their [documentation](https://eigen.tuxfamily.org/dox/TopicCMakeGuide.html).

## Testing
You can perform unit tests, see `test/README.md` (there are more dependencies). Set the cmake variable UNIT_TESTS to build tests.

## TODO
- Use vcpkg to manage dependencies