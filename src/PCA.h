#ifndef PCA_H
#define PCA_H

#include <cmath>        // sqrt, sin, cos
#include <numeric>      // accumulate
#include <algorithm>    // for_each, max
#include <execution>
#include <vector>
#include <random>       // random_device, mt19937, uniform_real_distribution
#include <chrono>       // high_resolution_clock, milliseconds
#include <string>
#include <iostream>

#include "Eigen/Dense"

#include "Utils.h"

namespace math {

    /// ////////// ///
    /// CONVERSION ///
    /// ////////// ///

    template<class T>
    inline std::vector<T> convertEigenMatrixToStdVector(Eigen::Matrix<T, -1, -1> mat, bool followStorageOrder = true) {

        const Eigen::StorageOptions StorageOrder = mat.IsRowMajor ? Eigen::RowMajor : Eigen::ColMajor;

        // by default Eigen uses column-major storage order
        if (StorageOrder == Eigen::ColMajor)
        {
            mat.transposeInPlace();
        }

        return { mat.data(), mat.data() + mat.size() };
    }

    Eigen::MatrixXf convertStdVectorToEigenMatrix(const std::vector<float>& data_in, size_t num_dims)
    {
        const size_t num_row = data_in.size() / num_dims;
        const size_t num_col = num_dims;

        // convert HsneMatrix to Eigen MatrixXf
        // each row in MatrixXf corresponds to one data point
        Eigen::MatrixXf data(num_row, num_col);     	// num_rows (data points), num_cols (attributes)

        auto point_range = utils::pyrange(num_row);
        auto dim_range = utils::pyrange(num_col);

        std::for_each(std::execution::par_unseq, point_range.begin(), point_range.end(), [&](const auto point)
            {// loop over data points

                std::for_each(dim_range.begin(), dim_range.end(), [&](const auto dim)
                    {// loop over data point values
                        data(point, dim) = data_in[point* num_dims + dim];
                    }
                );
            }
        );

        // this would be more concise but only works if data_in is not const
        // Also, I didn't test this
        //Eigen::MatrixXf weights = Eigen::Map<Eigen::MatrixXf>(&data_in[0], num_row, num_col);

        return data;
    }


    /// //// ///
    /// UTIL ///
    /// //// ///

    // Returns the indices that would sort an array, like numpy.argsort
    template<class T, typename C = std::less<>>
    std::vector<size_t> argsort(T& data, C cmp = C{})
    {
        std::vector<size_t> idx(data.size());
        std::iota(idx.begin(), idx.end(), 0);

        std::sort(idx.begin(), idx.end(), [&data, &cmp](const size_t& a, const size_t& b)
            { return cmp(data[a], data[b]); });

        return idx;
    }

    /// /// ///
    /// PCA ///
    /// /// ///

    // Number pca components must be in [0, std::min(num_row, num_col)]
    static void checkNumComponents(const size_t num_row, const size_t num_col, size_t& num_comp)
    {
        if (num_comp > std::min(num_row, num_col))
        {
            std::cout << "pca: num_comp must be smaller than min(num_row, num_col). Setting num_comp = min(num_row, num_col)";
            num_comp = std::min(num_row, num_col);
        }
        else if (num_comp <= 0)
        {
            std::cout << "pca: num_comp must larger than 0. Setting num_comp = min(num_row, num_col)";
            num_comp = std::min(num_row, num_col);
        }
    }

    // https://en.wikipedia.org/wiki/Feature_scaling#Mean_normalization
    inline Eigen::MatrixXf meanNormalization(const Eigen::MatrixXf& mat)
    {
        const size_t num_row = mat.rows();
        const size_t num_col = mat.cols();

        // center around mean per attribute
        Eigen::MatrixXf mat_norm = mat.rowwise() - mat.colwise().mean();

        // norm with (max - min) attributes
        Eigen::VectorXf normFacs = mat.colwise().maxCoeff() - mat.colwise().minCoeff();

        // norm with (max - min) attributes:
        // divide all values in mat.col(i) by normFacs[i]
        //return mat_norm.array().rowwise() / normFacs.transpose().array();

        // the previous lines don't seem to work, this does:
        for (size_t col = 0; col < num_col; col++)
        {
            if (normFacs[col] < 0.0001f) continue;
            auto row_range = utils::pyrange(num_row);
            std::for_each(std::execution::par_unseq, row_range.begin(), row_range.end(), [&](const auto row)
                {
                    mat_norm(row, col) /= normFacs[col];
                }
            );
        } // there is probably a more elegant way of doing this

        return mat_norm;
    }

    // map each column to [0,1]
    // https://en.wikipedia.org/wiki/Feature_scaling#Rescaling_(min-max_normalization)
    inline Eigen::MatrixXf minMaxNormalization(const Eigen::MatrixXf& mat)
    {
        const size_t num_row = mat.rows();
        const size_t num_col = mat.cols();

        Eigen::VectorXf minVals = mat.colwise().minCoeff();
        Eigen::VectorXf normFacs = mat.colwise().maxCoeff().transpose() - minVals;

        // shift all values
        Eigen::MatrixXf mat_norm = mat.rowwise() - minVals.transpose();

        // norm with (max - min) attributes:
        // divide all values in mat.col(i) by normFacs[i]
        //return mat_norm.array().rowwise() / normFacs.transpose().array();

         // the previous lines don't seem to work, this does:
        for (size_t col = 0; col < num_col; col++)
        {
            if (normFacs[col] < 0.0001f) continue;
            auto row_range = utils::pyrange(num_row);
            std::for_each(std::execution::par_unseq, row_range.begin(), row_range.end(), [&](const auto row)
                {
                    mat_norm(row, col) /= normFacs[col];
                }
            );
        } // there is probably a more elegant way of doing this

        return mat_norm;
    }

    inline Eigen::MatrixXf pcaSVD(const Eigen::MatrixXf& data, size_t& num_comp)
    {
        const size_t num_row = data.rows();
        const size_t num_col = data.cols();

        checkNumComponents(num_row, num_col, num_comp);

        // compute svd
        Eigen::BDCSVD<Eigen::MatrixXf> svd(data, Eigen::ComputeFullV);
        svd.computeV();

        // this is equivalent to:
        // Eigen::MatrixXf v = svd.matrixV();
        // Eigen::MatrixXf v_comps = v(Eigen::all, Eigen::seq(0, num_comp - 1));
        // return data * v_comps;
        return svd.matrixV()(Eigen::all, Eigen::seq(0, num_comp - 1));

    }

    inline Eigen::MatrixXf pcaCovMat(const Eigen::MatrixXf& data, size_t& num_comp) {
        const size_t num_row = data.rows();
        const size_t num_col = data.cols();

        checkNumComponents(num_row, num_col, num_comp);

        // covaraince matrix
        Eigen::MatrixXf covMat = data.transpose() * data;

        // covariance matrices are symmetric, so use appropriate solver
        Eigen::SelfAdjointEigenSolver <Eigen::MatrixXf> es(covMat);
        Eigen::VectorXf eigenvalues = es.eigenvalues();
        Eigen::MatrixXf eigenvectors = es.eigenvectors();

        // sort eigenvalues and save as Eigen::Vector
        auto eigenvalueOrder = argsort(eigenvalues, std::greater{});
        Eigen::Matrix<size_t, -1, 1> eigenvalueOrderE = Eigen::Map<Eigen::Matrix<size_t, -1, 1>, Eigen::Unaligned>(eigenvalueOrder.data(), eigenvalueOrder.size());

        // permutate eigenvalues and -vectors such that the eigenvectors that correspond
        // with the largest eigenvalues are in the first columns
        Eigen::PermutationMatrix<Eigen::Dynamic, Eigen::Dynamic, size_t> perm(eigenvalueOrderE.transpose());
        eigenvectors = eigenvectors * perm;     // permute columns
        eigenvalues = perm * eigenvalues;

        return eigenvectors(Eigen::all, Eigen::seq(0, num_comp - 1));
    }

    inline Eigen::MatrixXf pcaTransform(const Eigen::MatrixXf& data, const Eigen::MatrixXf& principal_components)
    {
        return data * principal_components;
    }

    enum class PCA_ALG {
        SVD,    // Use singular value decomposition, Eigen::BDCSVD
        COV,    // Compute eigenvalues of covariance matrix of data, Eigen::SelfAdjointEigenSolver
    };

    inline void pca(const std::vector<float>& data_in, const size_t num_dims, std::vector<float>& pca_out, size_t& num_comp, PCA_ALG algorithm = PCA_ALG::SVD)
    {
        // convert HsneMatrix to Eigen MatrixXf
        Eigen::MatrixXf data = convertStdVectorToEigenMatrix(data_in, num_dims);

        assert(data.rows() * data.cols() == data_in.size());
        assert(data.cols() == num_dims);

        // prep data: min-max normalization
        Eigen::MatrixXf data_norm = minMaxNormalization(data);

        // choose which pcaSVD algorithm to use and execute it
        auto pca_alg = [&](const Eigen::MatrixXf& dat) {
            if (algorithm == PCA_ALG::SVD)
                return pcaSVD(dat, num_comp);
            else // algorithm == PCA_ALG::COV
                return pcaCovMat(dat, num_comp);
        };

        // compute pcaSVD, get first num_comp components
        Eigen::MatrixXf principal_components = pca_alg(data_norm);

        // project data
        Eigen::MatrixXf data_transformed = pcaTransform(data_norm, principal_components);

        // compute 2 pca components and convert to std vector with [p0d0, p0d1, ..., p1d0, p1d1, ..., pNd0, pNd1, ..., pNdM]
        pca_out = convertEigenMatrixToStdVector(data_transformed);
    }


}

#endif PCA_H