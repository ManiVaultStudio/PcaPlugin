# PCA Plugin

Performs a [principal components analysis](https://en.wikipedia.org/wiki/Principal_component_analysis).

## Dependencies

- Eigen 3.4: Download the header-only library [Eigen](https://gitlab.com/libeigen/eigen) and pass the top level directory of your local copy as `EIGEN3_INCLUDE_DIR` to cmake.

## Testing
You can perform unit tests, see `test/README.md` (there are more dependencies). Set the cmake variable UNIT_TESTS to build tests.