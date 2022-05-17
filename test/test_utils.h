#pragma once

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

template<class T>
inline bool compEigAndStdMatrixAppr(const Eigen::MatrixXf& eig_mat, const std::vector<T>& std_mat, float eps = 0.0001f)
{
	const Eigen::MatrixXf std_conv_mat = math::convertStdVectorToEigenMatrix(std_mat, eig_mat.cols());

	//printLine("eig_mat");
	//std::cout << eig_mat << std::endl;
	//printLine("std_conv_mat");
	//std::cout << std_conv_mat << std::endl;

	Eigen::MatrixXf m1 = math::standardOrientation(std_conv_mat);
	Eigen::MatrixXf m2 = math::standardOrientation(eig_mat);

	//printLine("m1");
	//std::cout << m1 << std::endl;
	//printLine("m2");
	//std::cout << m2 << std::endl;

	return (m1.cwiseAbs() - m2.cwiseAbs()).isZero(eps);
}

template<class T>
inline bool compStdAndStdMatrixAppr(const std::vector<T>& mat_a, const std::vector<T>& mat_b, size_t num_cols, float eps = 0.0001f)
{
	const Eigen::MatrixXf mat_a_eig = math::convertStdVectorToEigenMatrix(mat_a, num_cols);
	const Eigen::MatrixXf mat_b_eig = math::convertStdVectorToEigenMatrix(mat_b, num_cols);

	Eigen::MatrixXf m1 = math::standardOrientation(mat_a_eig);
	Eigen::MatrixXf m2 = math::standardOrientation(mat_b_eig);

	return (m1.cwiseAbs() - m2.cwiseAbs()).isZero(eps);
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
