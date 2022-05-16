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

TEST_CASE("Sklearn example data", "[PCA][COV][SVD][NONORM][MinMaxNorm][MeanNorm]") {
	// https://scikit-learn.org/1.1/modules/generated/sklearn.decomposition.PCA.html
	// Sklearn example info
	const size_t num_points = 6;
	const size_t num_dims = 2;

	SECTION("SKLEARN") {
		printLine("Sklearn example data: no norm");

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
		printLine("Sklearn example data: norm minmax");

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
		printLine("Sklearn example data: norm mean");

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

TEST_CASE("Toy data", "[PCA][SVD][COV]") {

	SECTION("TOY") {
		printLine("Toy data: no norm");

		const size_t num_pts = 6;
		const size_t num_dim = 3;
		size_t num_comp = 3;

		// define data
		Matrix data = { {1, 1, 3}, {2, 1, 4}, {-3, 2, 0}, {0.1, 0.5, 0.8}, {2, 1, 1}, {4, 2, 3} };
		Eigen::MatrixXf eigen_data(num_pts, num_dim);
		eigen_data << 1, 1, 3, 2, 1, 4, -3, 2, 0, 0.1, 0.5, 0.8, 2, 1, 1, 4, 2, 3;
		
		const std::vector<float> reference_components{ 0.86774688, -0.49679373,  0.01453782, -0.02012211, -0.00589032,  0.99978018,  0.49659889,  0.86784866, 0.01510785 };
		const std::vector<float> reference_transform{  0.50372027,  0.90652942, -0.2345759,   1.86806603,  1.27758436, -0.20493023, -4.47718603,  0.28426804, 0.66172945,
													  -1.35970842, -0.55267811, -0.78078729,  0.37826936, -1.32596162, -0.25025377,  3.08683879, -0.58974208, 0.80881774 };

		// center the data
		eigen_data = eigen_data.rowwise() - eigen_data.colwise().mean();

		// compute svd
		Eigen::MatrixXf matrixV = math::pcaSVD(eigen_data, num_comp);
		REQUIRE(compEigAndStdMatrixAppr(matrixV, reference_components));

		// project centered data
		Eigen::MatrixXf trans = math::pcaTransform(eigen_data, matrixV);
		REQUIRE(compEigAndStdMatrixAppr(trans, reference_transform));

		// convert from eigen to std vector
		std::vector<float> trans_std = math::convertEigenMatrixToStdVector(trans);
		REQUIRE(compStdAndStdMatrixAppr(trans_std, reference_transform));

		// use combined function instead of calling each step and use covariance instead of svd function
		auto data_std = math::convertEigenMatrixToStdVector(eigen_data);
		std::vector<float> transCOV;
		math::pca(data_std, num_dim, transCOV, num_comp, math::PCA_ALG::COV, math::DATA_NORM::NONE);

		REQUIRE(compStdAndStdMatrixAppr(transCOV, reference_transform));
	}

}

TEST_CASE("Iris SVD MinMaxNorm data", "[PCA][SVD][MinMaxNorm]") {

	const std::string fileName = dataDir.string() + "data.bin";
	std::vector<float> data_in;
	readBinaryToStdVector(fileName, data_in);

	// iris data set info
	const size_t num_points = 150;
	const size_t num_dims = 4;

	// check of data set was loaded correctly
	REQUIRE(data_in.size() == num_points * num_dims);

	std::vector<float> principal_components_std;
	std::vector<float> data_transformed_std;

	auto individualSteps = [&](std::vector<float>& pcs_std, std::vector<float>& trans_std, size_t num_comp) -> void {
		// convert HsneMatrix to Eigen MatrixXf
		Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);

		// min max norm
		Eigen::MatrixXf data_normed = math::minMaxNormalization(data);

		// center the data
		data_normed = math::colwiseZeroMean(data_normed);

		// compute pcaSVD, get first num_comp components
		Eigen::MatrixXf principal_components = math::pcaSVD(data_normed, num_comp);
		pcs_std = math::convertEigenMatrixToStdVector(principal_components);

		// project data
		Eigen::MatrixXf data_transformed = math::pcaTransform(data_normed, principal_components);
		trans_std = math::convertEigenMatrixToStdVector(data_transformed);
	};

	SECTION("Two components individual steps") {
		printLine("Iris data: 2 comp individual steps, SVD MinMaxNorm");
		size_t num_comp = 2;

		// Load the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_minmax_2.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_minmax_2.bin", data_transformed_reference);

		individualSteps(principal_components_std, data_transformed_std, num_comp);

		
		REQUIRE(compStdAndStdMatrixAppr(data_transformed_std, data_transformed_reference));

	}

	SECTION("All components individual steps") {
		printLine("Iris data: 4 comp individual steps, SVD MinMaxNorm");
		size_t num_comp = num_dims;

		// Load the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_minmax_4.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_minmax_4.bin", data_transformed_reference);

		individualSteps(principal_components_std, data_transformed_std, num_comp);

		REQUIRE(compStdAndStdMatrixAppr(principal_components_std, principal_components_reference));
		REQUIRE(compStdAndStdMatrixAppr(data_transformed_std, data_transformed_reference));

	}

	SECTION("Two components single step") {
		printLine("Iris data: 2 comp single step, SVD MinMaxNorm");
		size_t num_comp = 2;

		// Load the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_minmax_2.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_minmax_2.bin", data_transformed_reference);

		std::vector<float> transSVD;
		math::pca(data_in, num_dims, transSVD, num_comp, math::PCA_ALG::SVD, math::DATA_NORM::MINMAX);

		REQUIRE(compStdAndStdMatrixAppr(transSVD, data_transformed_reference));
	}

	SECTION("All components single step") {
		printLine("Iris data: 4 comp single step, SVD MinMaxNorm");
		size_t num_comp = num_dims;

		// Load the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_minmax_4.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_minmax_4.bin", data_transformed_reference);

		std::vector<float> transSVD;
		math::pca(data_in, num_dims, transSVD, num_comp, math::PCA_ALG::SVD, math::DATA_NORM::MINMAX);

		REQUIRE(compStdAndStdMatrixAppr(transSVD, data_transformed_reference));
	}

}

TEST_CASE("Iris COV MeanNorm data", "[PCA][COV][MinMaxNorm]") {

	const std::string fileName = dataDir.string() + "data.bin";
	std::vector<float> data_in;
	readBinaryToStdVector(fileName, data_in);

	// iris data set info
	const size_t num_points = 150;
	const size_t num_dims = 4;

	// check of data set was loaded correctly
	REQUIRE(data_in.size() == num_points * num_dims);

	std::vector<float> principal_components_std;
	std::vector<float> data_transformed_std;

	auto individualSteps = [&](std::vector<float>& pcs_std, std::vector<float>& trans_std, size_t num_comp) -> void {
		// convert HsneMatrix to Eigen MatrixXf
		Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);

		// mean norm
		Eigen::MatrixXf data_normed = math::meanNormalization(data);

		// center the data
		data_normed = math::colwiseZeroMean(data_normed);

		// compute pcaSVD, get first num_comp components
		Eigen::MatrixXf principal_components = math::pcaCovMat(data_normed, num_comp);
		pcs_std = math::convertEigenMatrixToStdVector(principal_components);

		// project data
		Eigen::MatrixXf data_transformed = math::pcaTransform(data_normed, principal_components);
		trans_std = math::convertEigenMatrixToStdVector(data_transformed);
	};

	SECTION("Two components individual steps") {
		printLine("Iris data: 2 comp individual steps, COV MeanNorm");
		size_t num_comp = 2;

		// Load the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_mean_2.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_mean_2.bin", data_transformed_reference);

		individualSteps(principal_components_std, data_transformed_std, num_comp);

		REQUIRE(compStdAndStdMatrixAppr(principal_components_std, principal_components_reference));
		REQUIRE(compStdAndStdMatrixAppr(data_transformed_std, data_transformed_reference));

	}

	SECTION("All components individual steps") {
		printLine("Iris data: 4 comp individual steps, COV MeanNorm");
		size_t num_comp = num_dims;

		// Load the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_mean_4.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_mean_4.bin", data_transformed_reference);

		individualSteps(principal_components_std, data_transformed_std, num_comp);

		REQUIRE(compStdAndStdMatrixAppr(principal_components_std, principal_components_reference));
		REQUIRE(compStdAndStdMatrixAppr(data_transformed_std, data_transformed_reference));

	}

	SECTION("Two components single step") {
		printLine("Iris data: 2 comp single step, COV MeanNorm");
		size_t num_comp = 2;

		// Load the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_mean_2.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_mean_2.bin", data_transformed_reference);

		std::vector<float> transSVD;
		math::pca(data_in, num_dims, transSVD, num_comp, math::PCA_ALG::COV, math::DATA_NORM::MEAN);

		REQUIRE(compStdAndStdMatrixAppr(transSVD, data_transformed_reference));
	}

	SECTION("All components single step") {
		printLine("Iris data: 4 comp single step, COV MeanNorm");
		size_t num_comp = num_dims;

		// Load the reference values
		std::vector<float> principal_components_reference;
		readBinaryToStdVector(dataDir.string() + "pca_norm_mean_4.bin", principal_components_reference);
		std::vector<float> data_transformed_reference;
		readBinaryToStdVector(dataDir.string() + "trans_norm_mean_4.bin", data_transformed_reference);

		std::vector<float> transSVD;
		math::pca(data_in, num_dims, transSVD, num_comp, math::PCA_ALG::COV, math::DATA_NORM::MEAN);

		REQUIRE(compStdAndStdMatrixAppr(transSVD, data_transformed_reference));
	}

}

TEST_CASE("Iris data normalization", "[MeanNorm][MinMaxNorm]") {

	const std::string fileName = dataDir.string() + "data.bin";
	std::vector<float> data_in;
	readBinaryToStdVector(fileName, data_in);

	// iris data set info
	const size_t num_points = 150;
	const size_t num_dims = 4;

	// check of data set was loaded correctly
	REQUIRE(data_in.size() == num_points * num_dims);

	// convert HsneMatrix to Eigen MatrixXf
	Eigen::MatrixXf data = math::convertStdVectorToEigenMatrix(data_in, num_dims);


	SECTION("Mean Norm") {
		printLine("Iris data: Mean Norm");

		std::vector<float> data_normed_reference;
		readBinaryToStdVector(dataDir.string() + "data_norm_mean.bin", data_normed_reference);

		// mean norm
		Eigen::MatrixXf data_normed = math::meanNormalization(data);

		REQUIRE(compEigAndStdMatrixAppr(data_normed, data_normed_reference));

	}

	SECTION("MinMax Norm") {
		printLine("Iris data: MinMax Norm");

		std::vector<float> data_normed_reference;
		readBinaryToStdVector(dataDir.string() + "data_norm_minmax.bin", data_normed_reference);

		// mean norm
		Eigen::MatrixXf data_normed = math::minMaxNormalization(data);

		REQUIRE(compEigAndStdMatrixAppr(data_normed, data_normed_reference));

	}


}
