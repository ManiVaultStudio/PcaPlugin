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

using Matrix = std::vector<std::vector<float>>;

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

template<class T>
inline bool compEigAndStdMatrix(const Eigen::MatrixXf& eig_mat, const std::vector<T>& std_mat)
{
	const Eigen::MatrixXf std_conv_mat = math::convertStdVectorToEigenMatrix(std_mat, eig_mat.cols());
	return eig_mat == std_conv_mat;
}

template<class T>
inline bool compEigAndStdMatrixAppr(const Eigen::MatrixXf& eig_mat, const std::vector<T>& std_mat, float eps = 0.0001f)
{
	const Eigen::MatrixXf std_conv_mat = math::convertStdVectorToEigenMatrix(std_mat, eig_mat.cols());
	return ((eig_mat.cwiseAbs() - std_conv_mat.cwiseAbs()).norm() <= eps);	// .norm() is the Frobenius norm
}

template<class T>
inline bool compStdAndStdMatrixAppr(const std::vector<T>& mat_a, const std::vector<T>& mat_b, float eps = 0.0001f)
{
	return std::equal(mat_a.begin(), mat_a.end(), mat_b.begin(), [&](T a, T b) { return std::abs(a) - std::abs(b) <= eps; });
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

TEST_CASE("Sklearn example data", "[PCA][COV]") {
	// https://scikit-learn.org/1.1/modules/generated/sklearn.decomposition.PCA.html

	// Sklearn example info
	const size_t num_points = 6;
	const size_t num_dims = 2;


	SECTION("SKLEARN") {
		size_t num_comp = 2;

		const std::string fileName = dataDir.string() + "sklearn_data.bin";
		std::vector<float> data_in;
		readBinaryToStdVector(fileName, data_in);

		// check of data set was loaded correctly
		REQUIRE(data_in.size() == num_points * num_dims);

		Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);

		// Test if loaded data is same as pre-defined
		Eigen::MatrixXf eigen_data(num_points, num_dims);
		eigen_data << -1, -1, -2, -1, -3, -2, 1, 1, 2, 1, 3, 2;
		std::vector<float> eigen_std = math::convertEigenMatrixToStdVector(eigen_data);
		REQUIRE(eigen_data == data);

		// Priciple components
		Eigen::MatrixXf matrixV = math::pcaCovMat(data, num_comp);
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "sklearn_pca.bin", principal_components_reference);
		REQUIRE(compEigAndStdMatrixAppr(matrixV, principal_components_reference));

		// Transformation
		Eigen::MatrixXf trans = math::pcaTransform(data, matrixV);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "sklearn_trans.bin", data_transformed_reference);
		REQUIRE(compEigAndStdMatrixAppr(trans, data_transformed_reference));

		// Single step
		std::vector<float> transCOV, transSVD;
		math::pca(data_in, num_dims, transCOV, num_comp, math::PCA_ALG::COV, math::DATA_NORM::NONE);

		REQUIRE(compStdAndStdMatrixAppr(transCOV, data_transformed_reference));

		math::pca(data_in, num_dims, transSVD, num_comp, math::PCA_ALG::SVD, math::DATA_NORM::NONE);
		REQUIRE(compStdAndStdMatrixAppr(transSVD, data_transformed_reference));

	}

	SECTION("SKLEARN - NORM MINMAX") {
		size_t num_comp = 2;

		// Load data
		const std::string fileName = dataDir.string() + "sklearn_data.bin";
		std::vector<float> data_in;
		readBinaryToStdVector(fileName, data_in);
		//std::vector<float> sklearn_pca_norm_minmax;
		//readBinaryToStdVector(dataDir.string() + "sklearn_pca_norm_minmax.bin", sklearn_pca_norm_minmax);
		std::vector<float> sklearn_trans_norm_minmax;
		readBinaryToStdVector(dataDir.string() + "sklearn_trans_norm_minmax.bin", sklearn_trans_norm_minmax);

		// check of data set was loaded correctly
		REQUIRE(data_in.size() == num_points * num_dims);
		//REQUIRE(sklearn_pca_norm_minmax.size() == num_comp * num_dims);
		REQUIRE(sklearn_trans_norm_minmax.size() == num_points * num_comp);

		// compute pca
		std::vector<float> transCOV, transSVD;

		math::pca(data_in, num_dims, transCOV, num_comp, math::PCA_ALG::COV, math::DATA_NORM::MINMAX);
		REQUIRE(compStdAndStdMatrixAppr(transCOV, sklearn_trans_norm_minmax));

		math::pca(data_in, num_dims, transSVD, num_comp, math::PCA_ALG::SVD, math::DATA_NORM::MINMAX);
		REQUIRE(compStdAndStdMatrixAppr(transSVD, sklearn_trans_norm_minmax));

	}

	SECTION("SKLEARN - NORM MEAN") {
		size_t num_comp = 2;

		// Load data
		const std::string fileName = dataDir.string() + "sklearn_data.bin";
		std::vector<float> data_in;
		readBinaryToStdVector(fileName, data_in);
		//std::vector<float> sklearn_pca_norm_mean;
		//readBinaryToStdVector(dataDir.string() + "sklearn_pca_norm_mean.bin", sklearn_pca_norm_minmax);
		std::vector<float> sklearn_trans_norm_mean;
		readBinaryToStdVector(dataDir.string() + "sklearn_trans_norm_mean.bin", sklearn_trans_norm_mean);

		// check of data set was loaded correctly
		REQUIRE(data_in.size() == num_points * num_dims);
		//REQUIRE(sklearn_pca_norm_mean.size() == num_comp * num_dims);
		REQUIRE(sklearn_trans_norm_mean.size() == num_points * num_comp);

		// compute pca
		std::vector<float> transCOV, transSVD;

		math::pca(data_in, num_dims, transCOV, num_comp, math::PCA_ALG::COV, math::DATA_NORM::MEAN);
		REQUIRE(compStdAndStdMatrixAppr(transCOV, sklearn_trans_norm_mean));

		math::pca(data_in, num_dims, transSVD, num_comp, math::PCA_ALG::SVD, math::DATA_NORM::MEAN);
		REQUIRE(compStdAndStdMatrixAppr(transSVD, sklearn_trans_norm_mean));

	}

}

TEST_CASE("TOY DATA", "[PCA]") {

	SECTION("TEST") {
		const size_t num_pts = 6;
		const size_t num_dim = 3;
		size_t num_comp = 3;

		Matrix data = { {1, 1, 3}, {2, 1, 4}, {-3, 2, 0}, {0.1, 0.5, 0.8}, {2, 1, 1}, {4, 2, 3} };

		Eigen::MatrixXf eigen_data(num_pts, num_dim);
		eigen_data << 1, 1, 3, 2, 1, 4, -3, 2, 0, 0.1, 0.5, 0.8, 2, 1, 1, 4, 2, 3;
		printLine("eigen_data");
		std::cout << eigen_data << std::endl;

		//Eigen::MatrixXf data_norm = math::minMaxNormalization(eigen_data);
		//printLine("data_norm");
		//std::cout << data_norm << std::endl;

		Eigen::MatrixXf matrixV = math::pcaCovMat(eigen_data, num_comp);
		printLine("matrixV");
		std::cout << matrixV << std::endl;

		Eigen::MatrixXf trans = math::pcaTransform(eigen_data, matrixV);
		printLine("trans");
		std::cout << trans << std::endl;

		std::vector<float> trans_std = math::convertEigenMatrixToStdVector(trans);
		printLine("trans_std");
		printVector(trans_std);


		auto data_std = math::convertEigenMatrixToStdVector(eigen_data);
		std::vector<float> transCOV;
		math::pca(data_std, num_dim, transCOV, num_comp, math::PCA_ALG::COV, math::DATA_NORM::MINMAX);

		printLine("transCOV");
		std::cout << math::convertStdVectorToEigenMatrix(transCOV, num_comp) << std::endl;

	}

}

TEST_CASE("PCA SVD MinMaxNorm data", "[PCA][SVD][MinMaxNorm]") {

	const std::string fileName = dataDir.string() + "data.bin";
	std::vector<float> data_in;
	readBinaryToStdVector(fileName, data_in);

	// iris data set info
	const size_t num_points = 150;
	const size_t num_dims = 4;

	// check of data set was loaded correctly
	REQUIRE(data_in.size() == num_points * num_dims);

	SECTION("Do it again") {
		size_t num_comp = num_dims;

		// Read the reference values
		std::vector<float> data_norm_reference;
		readBinaryToStdVector(dataDir.string() + "data_norm_minmax.bin", data_norm_reference);
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_minmax_4.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_minmax_4.bin", data_transformed_reference);

		// check of data set was loaded correctly
		REQUIRE(data_norm_reference.size() == num_points * num_dims);
		REQUIRE(principal_components_reference.size() == num_comp * num_dims);
		REQUIRE(data_transformed_reference.size() == num_points * num_comp);

		Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);

		printLine("data");
		std::cout << data.block(0, 0, 5, 4) << std::endl;

		Eigen::MatrixXf data_norm = math::minMaxNormalization(data);
		printLine("data_norm");
		std::cout << data_norm.block(0, 0, 5, 4) << std::endl;

		Eigen::MatrixXf matrixV = math::pcaCovMat(data_norm, num_comp);
		printLine("matrixV");
		std::cout << matrixV << std::endl;

		Eigen::MatrixXf trans = math::pcaTransform(data_norm, matrixV);
		printLine("trans");
		std::cout << trans.block(0, 0, 5, 4) << std::endl;

	}

	SECTION("All components") {
		size_t num_comp = 2;

		// Read the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_minmax_2.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_minmax_2.bin", data_transformed_reference);

		// convert HsneMatrix to Eigen MatrixXf
		Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);
		printLine("data.block");
		std::cout << data.block(0, 0, 4, 4) << std::endl;

		// compute pcaSVD, get first num_comp components
		Eigen::MatrixXf principal_components = math::pcaSVD(data, num_comp);
		std::vector<float> principal_components_std = math::convertEigenMatrixToStdVector(principal_components);
		//printLine("principal_components.block");
		//std::cout << principal_components.block(0, 0, 4, 2) << std::endl;
		printLine("principal_components");
		std::cout << principal_components << std::endl;
		Eigen::MatrixXf principal_components_reference_mat = math::convertStdVectorToEigenMatrix(principal_components_reference, 4);
		printLine("principal_components_reference_mat");
		std::cout << principal_components_reference_mat << std::endl;

		Eigen::MatrixXf trans = math::pcaTransform(data, principal_components);
		printLine("trans");
		std::cout << trans << std::endl;

		printLine("principal_components_std");
		printVector(principal_components_std);
		printLine("principal_components_reference");
		printVector(data_transformed_reference);
		printLine("principal_components_reference");
		printVector(data_transformed_reference, 10);
		//REQUIRE(principal_components_std == principal_components_reference);

		// project data
		Eigen::MatrixXf data_transformed = math::pcaTransform(data, principal_components);
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
//		std::cout << data.block(0, 0, 4, 4);
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