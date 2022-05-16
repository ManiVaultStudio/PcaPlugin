#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <filesystem>
#include <source_location>
#include <iostream>

#include "PCA.h"
#include "Utils.h"

// see https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md#test-cases-and-sections

// TODO:
// try out the test data set with only a couple of points
// then, for the digits, try out only one data set (some norm) in the settings list
// try out different sklearn data sets

const std::filesystem::path current_file_path = std::source_location::current().file_name();
const std::filesystem::path dataDir = current_file_path.parent_path() / "data" / "";

// digits data set info
//const size_t num_points = 1797;
//const size_t num_dims = 64;

// iris data set info
const size_t num_points = 150;
const size_t num_dims = 4;

template<class T>
void printVector(std::vector<T>& vec)
{
	for (auto& val : vec)
		std::cout << val << " ";
	std::cout << std::endl;
}

template<class T>
void printVector(std::vector<T>& vec, size_t until)
{
	if (until >= vec.size()) until = vec.size();

	for (size_t i = 0; i < until; i++)
		std::cout << vec[i] << " ";
	std::cout << std::endl;
}

void printLine(std::string line)
{
	std::cout << line << std::endl;
}

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

TEST_CASE("PCA SVD MinMaxNorm data", "[PCA][SVD][MinMaxNorm]") {

	const std::string fileName = dataDir.string() + "data.bin";
	std::vector<float> data_in;
	readBinaryToStdVector(fileName, data_in);

	// check of digits data set was loaded correctly
	REQUIRE(data_in.size() == num_points * num_dims);

	SECTION("All components") {
		size_t num_comp = 2;

		// Read the reference values
		std::vector<float> data_norm_reference;
		readBinaryToStdVector(dataDir.string() + "data_norm_minmax.bin", data_norm_reference);
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_minmax_2.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_minmax_2.bin", data_transformed_reference);

		// convert HsneMatrix to Eigen MatrixXf
		Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);
		printLine("data.block");
		std::cout << data.block(0, 0, 10, 10) << std::endl;

		Eigen::MatrixXf data_norm = math::minMaxNormalization(data);
		printLine("data_norm.block");
		std::cout << data_norm.block(0, 0, 10, 10) << std::endl;
		printLine("data_norm_reference");
		printVector(data_norm_reference, 20);
		REQUIRE(math::convertEigenMatrixToStdVector(data_norm) == data_norm_reference);

		// compute pcaSVD, get first num_comp components
		Eigen::MatrixXf principal_components = math::pcaSVD(data_norm, num_comp);
		std::vector<float> principal_components_std = math::convertEigenMatrixToStdVector(principal_components);
		printLine("principal_components.block");
		std::cout << principal_components.block(0, 0, 10, 2) << std::endl;
		Eigen::MatrixXf principal_components_reference_mat = math::convertStdVectorToEigenMatrix(principal_components_reference, 2);
		printLine("principal_components_reference_mat");
		std::cout << principal_components_reference_mat << std::endl;
		printLine("principal_components_std");
		printVector(principal_components_std, 20);
		printLine("principal_components_reference");
		printVector(principal_components_reference);
		//REQUIRE(principal_components_std == principal_components_reference);

		// project data
		Eigen::MatrixXf data_transformed = math::pcaTransform(data_norm, principal_components);
		std::vector<float> data_transformed_std = math::convertEigenMatrixToStdVector(data_transformed);
		//REQUIRE(data_transformed_std == data_transformed_reference);

	}
	//   SECTION("All components") {

	   //}
}

//TEST_CASE("PCA SVD raw data", "[PCA][SVD]") {
//
//    const std::string fileName = dataDir.string() +  "data.bin";
//    std::vector<float> data_in;
//    readBinaryToStdVector(fileName, data_in);
//
//	// check of digits data set was loaded correctly
//	REQUIRE(data_in.size() == num_points * num_dims);
//
//    SECTION("2 components") {
//		size_t num_comp = 2;
//
//		// Read the reference values
//		std::vector<float> principal_components_reference;
//		readBinaryToStdVector(dataDir.string() + "pca_raw_2.bin", principal_components_reference);
//		std::vector<float> data_transformed_reference;
//		readBinaryToStdVector(dataDir.string() + "trans_raw_2.bin", data_transformed_reference);
//
//        // convert HsneMatrix to Eigen MatrixXf
//        Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);
//
//		std::cout << data.block(0, 0, 10, 10);
//
//        // compute pcaSVD, get first num_comp components
//        Eigen::MatrixXf principal_components = math::pcaSVD(data, num_comp);
//
//		std::cout << principal_components;
//
//		std::vector<float> principal_components_std = math::convertEigenMatrixToStdVector(principal_components);
//		REQUIRE(principal_components_std == principal_components_reference);
//
//        // project data
//        Eigen::MatrixXf data_transformed = math::pcaTransform(data, principal_components);
//		std::vector<float> data_transformed_std = math::convertEigenMatrixToStdVector(data_transformed);
//		REQUIRE(data_transformed_std == data_transformed_reference);
//
//	}
// //   SECTION("All components") {
//
//	//}
//}