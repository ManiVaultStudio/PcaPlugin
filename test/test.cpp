#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <filesystem>

#include "PCA.h"
#include "Utils.h"

// see https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md#test-cases-and-sections

const std::filesystem::path dataDir = std::filesystem::current_path() / "data" / "";

// digits data set info
const size_t num_points = 1797;
const size_t num_dims = 64;

void readBinaryToStdVector(const std::string fileName, std::vector<float>& data)
{
	std::ifstream fin(fileName, std::ios::in | std::ios::binary);
	if (!fin.is_open()) {
		std::cout << "Unable to load file: " << fileName << std::endl;
	}
	else {
		// number of data points
		fin.seekg(0, std::ios::end);
		auto fileSize = fin.tellg();
		auto numDataPoints = fileSize / sizeof(float);
		fin.seekg(0, std::ios::beg);

		// read data
		data.clear();
		data.resize(numDataPoints);
		fin.read(reinterpret_cast<char*>(data.data()), fileSize);
		fin.close();
	}
}

TEST_CASE("PCA SVD raw data", "[PCA][SVD]") {

    const std::string fileName = dataDir.string() +  "data";
    std::vector<float> data_in;
    readBinaryToStdVector(fileName, data_in);

    SECTION("2 components") {
		size_t num_comp = 2;

		// Read the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_raw_2", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_raw_2", data_transformed_reference);

        // convert HsneMatrix to Eigen MatrixXf
        Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);

        // compute pcaSVD, get first num_comp components
        Eigen::MatrixXf principal_components = math::pcaSVD(data, num_comp);
		std::vector<float> principal_components_std = math::convertEigenMatrixToStdVector(principal_components);
		REQUIRE(principal_components_std == principal_components_reference);

        // project data
        Eigen::MatrixXf data_transformed = math::pcaTransform(data, principal_components);
		std::vector<float> data_transformed_std = math::convertEigenMatrixToStdVector(data_transformed);
		REQUIRE(data_transformed_std == data_transformed_reference);

	}
    SECTION("All components") {

	}
}