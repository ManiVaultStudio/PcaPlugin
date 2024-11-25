#pragma once

#include <fstream>
#include <iostream>

#include "Eigen/Dense"

namespace testing {

	/// /////// ///
	/// LOGGING ///
	/// /////// ///

	template<class T>
	void printVector(const std::vector<T>& vec)
	{
		for (const auto& val : vec)
			std::cout << val << " ";
		std::cout << std::endl;
	}

	template<class T>
	void printVector(const std::vector<T>& vec, size_t until)
	{
		if (until >= vec.size()) until = vec.size();

		for (size_t i = 0; i < until; i++)
			std::cout << vec[i] << " ";
		std::cout << std::endl;
	}

	void printLine(const std::string& line)
	{
		std::cout << line << std::endl;
	}

	/// /////////// ///
	/// COMPARISONS ///
	/// /////////// ///

	template<class T>
	inline bool compEigAndStdMatrix(const Eigen::MatrixXf& eig_mat, const std::vector<T>& std_mat)
	{
		const Eigen::MatrixXf std_conv_mat = math::convertStdVectorToEigenMatrix(std_mat, eig_mat.cols());
		return eig_mat == std_conv_mat;
	}

	template<class T>
	inline bool compEigAndStdMatrixAbsAppr(const Eigen::MatrixXf& eig_mat, const std::vector<T>& std_mat, float eps = 0.0001f)
	{
		const Eigen::MatrixXf std_conv_mat = math::convertStdVectorToEigenMatrix(std_mat, eig_mat.cols());
		return ((eig_mat.cwiseAbs() - std_conv_mat.cwiseAbs()).norm() <= eps);	// .norm() is the Frobenius norm
	}

	template<class T>
	inline bool compStdAndStdMatrixAbsAppr(const std::vector<T>& mat_a, const std::vector<T>& mat_b, float eps = 0.0001f)
	{
		return std::equal(mat_a.begin(), mat_a.end(), mat_b.begin(), [&](T a, T b) { return std::abs(a) - std::abs(b) <= eps; });
	}

	inline bool compEigAndEigMatrixAppr(const Eigen::MatrixXf& mat_a, const Eigen::MatrixXf& mat_b, float eps = 0.0001f)
	{
		return (mat_a.cwiseAbs() - mat_b.cwiseAbs()).isZero(eps);
	}

	template<class T>
	inline bool compEigAndStdMatrixAppr(const Eigen::MatrixXf& eig_mat, const std::vector<T>& std_mat, float eps = 0.0001f)
	{
		const Eigen::MatrixXf mat = math::convertStdVectorToEigenMatrix(std_mat, eig_mat.cols());

		Eigen::MatrixXf mat_a = math::standardOrientation(eig_mat);
		Eigen::MatrixXf mat_b = math::standardOrientation(mat);

		return compEigAndEigMatrixAppr(mat_a, mat_b, eps);
	}

	template<class T>
	inline bool compStdAndStdMatrixAppr(const std::vector<T>& std_mat_a, const std::vector<T>& std_mat_b, size_t num_cols, float eps = 0.0001f)
	{
		const Eigen::MatrixXf eig_mat_a = math::convertStdVectorToEigenMatrix(std_mat_a, num_cols);
		const Eigen::MatrixXf eig_mat_b = math::convertStdVectorToEigenMatrix(std_mat_b, num_cols);

		Eigen::MatrixXf mat_a = math::standardOrientation(eig_mat_a);
		Eigen::MatrixXf mat_b = math::standardOrientation(eig_mat_b);

		return compEigAndEigMatrixAppr(mat_a, mat_b, eps);
	}

	/// /////// ///
	/// FILE IO ///
	/// /////// ///

	[[nodiscard]] bool readBinaryToStdVector(const std::string& fileName, std::vector<float>& data)
	{
		std::ifstream fin(fileName, std::ios::in | std::ios::binary);
	
		// check if files exists
		if (!fin.is_open()) {
			std::cout << "Unable to load file: " << fileName << std::endl;
			return false;
		}

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

		return true;
	}

}
