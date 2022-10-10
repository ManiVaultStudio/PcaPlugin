# PCA Plugin Test
Run `create_reference_data.py` to create reference data sets and reference PCA transformations before performing the tests.

The reference values are created in python using [scikit-learn](https://scikit-learn.org) and [numpy](https://numpy.org/).

## Dependencies:
- [Catch2](https://github.com/catchorg/Catch2) for unit testing
- [nlohmann_json](https://github.com/nlohmann/json) for reading meta data about the test data from json files in cpp
- Eigen 3.4: Setup [Eigen](https://gitlab.com/libeigen/eigen) as described in their [documentation](https://eigen.tuxfamily.org/dox/TopicCMakeGuide.html).
