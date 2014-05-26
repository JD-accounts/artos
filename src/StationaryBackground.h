#ifndef ARTOS_STATIONARYBACKGROUND_H
#define ARTOS_STATIONARYBACKGROUND_H

#include <string>
#include <Eigen/Core>

namespace ARTOS
{

/**
* Class for reading and writing stationary background statistics, i. e. general mean and covariance, from/to a binary file.
*
* Beyond that, this class is also capable of reconstructing a covariance matrix of a specific size from the stationary
* autocorrelation function read before.  
* The great deal behind this: Each covariance between two cells \f$x_{i,j}\f$ and \f$x_{m,n}\f$ only depends on the offset \f$(m-i, n-j)\f$.
* Since there are only N offsets given a total of N cells, the autocorrelation function only has to come up with
* \f$N * num\_features^2\f$ instead of \f$N^2 * num\_features^2\f$, which the full covariance matrix needs.  
* Therefore, N, i. e. the number of spatial offsets used for learning the statistics, is the upper bound for the number
* of rows and columns of reconstructed covariance matrices.
*
* @author Bjoern Barz <bjoern.barz@uni-jena.de>
*/
class StationaryBackground
{

public:

    /**
    * Mean feature vector for a single cell.
    */
    typedef Eigen::Array<float, Eigen::Dynamic, 1> MeanVector;
    
    /**
    * Covariance matrix between features for a specific offset.
    */
    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> CovMatrix;
    
    /**
    * Vector of feature covariance matrices.
    */
    typedef Eigen::Array<CovMatrix, Eigen::Dynamic, 1> CovMatrixArray;
    
    /**
    * Matrix of feature covariance matrices (used in reconstruction process).
    */
    typedef Eigen::Matrix<CovMatrix, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> CovMatrixMatrix;
    
    /**
    * Array of spatial offsets. Each row is a (dx, dy) tuple.
    */
    typedef Eigen::Array<int, Eigen::Dynamic, 2, Eigen::RowMajor> OffsetArray;
    
    /**
    * A generic 2-D matrix of floats.
    */
    typedef Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Matrix;
    
    
    /**
    * Constructs uninitialized background statistics, which can be read later using `readFromFile`.
    */
    StationaryBackground() : mean(), cov(), offsets(), cellSize(0) { };
    
    /**
    * Constructs empty background statistics with given dimensions.
    *
    * @param[in] numOffsets Number of spatial offsets.
    *
    * @param[in] numFeatures Number of features.
    *
    * @param[in] cellSize Number of pixels each cell spans in both dimensions.
    */
    StationaryBackground(const unsigned int numOffsets, const unsigned int numFeatures = 31, const unsigned int cellSize = 8);
    
    /**
    * Constructs a new background statistics object and reads the statistics from a given file.
    *
    * @param[in] backgroundFile Path of the binary background statistics file.
    */
    StationaryBackground(const std::string & backgroundFile) : mean(), cov(), offsets(), cellSize(0)
    { this->readFromFile(backgroundFile); };
    
    /**
    * Copy constructor.
    */
    StationaryBackground(const StationaryBackground & other)
    : mean(other.mean), cov(other.cov), offsets(other.offsets), cellSize(other.cellSize) { };
    
    /**
    * Reads background statistics from a file.
    *
    * @param[in] filename Path of the binary background statistics file.
    *
    * @return True if the file could be read successfully, false if it is inaccessible or invalid.
    * empty() may also be used to check the status of this operation after it has finished.
    */
    bool readFromFile(const std::string & filename);
    
    /**
    * Writes the background statistics hold by this object to a file.
    *
    * @param[in] filename Path to write the file to.
    *
    * @return Returns true if these statistics aren't empty and the file could be written successfully, otherwise false.
    */
    bool writeToFile(const std::string & filename);

    /**
    * Resets this object to it's initial state by clearing all statistics and resizing all matrices and vectors to zero size.
    */
    void clear();

    /**
    * Determines if statistics are present or not.
    *
    * @return True if the matrices and vectors holding the statistics have zero size.
    */
    bool empty() const { return (this->mean.size() == 0); };
    
    /**
    * @return Number of features each cell has.
    */
    unsigned int getNumFeatures() const { return this->mean.size(); };
    
    /**
    * @return Number of spatial offsets available.
    */
    unsigned int getNumOffsets() const { return this->cov.size(); };
    
    /**
    * @return Maximum available offset in x or y direction or -1 if the offset array is empty.
    */
    int getMaxOffset() const { return (this->offsets.rows() > 0) ? this->offsets.abs().maxCoeff() : -1; };
    
    /**
    * Reconstructs a covariance matrix from the spatial autocorrelation function for a specific number of
    * rows and columns. The resulting 4-D matrix is of the form `cov(xy1, xy2)(feature1, feature2)`, while
    * linearisation of a spatial position (x, y) to xy is done in row-major order.
    *
    * @param[in] rows Number of rows to reconstruct the covariance matrix for.
    *
    * @param[in] cols Number of columns to reconstruct the covariance matrix for.
    *
    * @return The reconstructed covariance matrix.
    *
    * @see computeFlattenedCovariance
    */
    CovMatrixMatrix computeCovariance(const int rows, const int cols);
    
    /**
    * Reconstructs a covariance matrix from the spatial autocorrelation function for a specific number of
    * rows and columns and flattens it into a 2-D matrix, so that the k-th feature of the cell at position (x, y)
    * yields to the index `(x * cols + y) * numFeatures + k` in the resulting matrix.
    */
    Matrix computeFlattenedCovariance(const int rows, const int cols, unsigned int features = 0);
    
    
    MeanVector mean; /**< Stationary negative mean of features. */
    
    CovMatrixArray cov; /**< Stationary covariance matrices for each offset as spatial autocorrelation function. */
    
    OffsetArray offsets; /**< Array of (dx, dy) spatial offsets corresponding to the elements of `cov`. */
    
    unsigned int cellSize; /**< Number of pixels per cell in both dimensions used for training this background statistics. */

};

}

#endif