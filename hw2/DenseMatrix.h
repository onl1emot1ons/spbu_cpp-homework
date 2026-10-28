//
// Created by konst on 27.10.2026.
//

#ifndef CPP_HOMEWORK_DENSEMATRIX_H
#define CPP_HOMEWORK_DENSEMATRIX_H

#include "Matrix.h"
#include <cstddef>
#include <vector>
#include <stdexcept>

class SparseMatrix;

class DenseMatrix : public Matrix {
private:
    int rows_;
    int cols_;
    std::vector<double> data_;
public:
    DenseMatrix(int rows, int cols, const std::vector<double>& data) : rows_(rows), cols_(cols), data_(data) {
        if (rows < 0) {
            throw std::invalid_argument("rows must be non-negative");
        }
        if (cols < 0) {
            throw std::invalid_argument("cols must be non-negative");
        }
        if (data.size() != static_cast<std::size_t>(rows) * cols) {
            throw std::invalid_argument("Data size does not match matrix dimensions");
        }
    }

    DenseMatrix(int rows, int cols) : rows_(rows), cols_(cols) {
        if (rows < 0) {
            throw std::invalid_argument("rows must be non-negative");
        }
        if (cols < 0) {
            throw std::invalid_argument("cols must be non-negative");
        }
        data_.resize(static_cast<std::size_t>(rows) * cols, 0.0);
    }


    DenseMatrix multiply_naive(const DenseMatrix& second) const {
        if (cols_ != second.rows_) {
            throw std::invalid_argument("Incompatible matrix dimensions");
        }

        DenseMatrix result(rows_, second.cols_);

        for (int i=0; i < rows_; ++i) {
            for (int j=0; j < second.cols_; ++j) {
                for (int k=0; k < cols_; ++k) {
                    result.data_[static_cast<std::size_t>(i) * result.cols_ + j] += data_[
                        static_cast<std::size_t>(i) * cols_ + k] * second.data_[
                        static_cast<std::size_t>(k) * second.cols_ + j];
                }
            }
        }
        return result;
    }

    DenseMatrix multiply_optimized(const DenseMatrix& second) const {
        if (cols_ != second.rows_) {
            throw std::invalid_argument("Incompatible matrix dimensions");
        }

        DenseMatrix result(rows_, second.cols_);

        for (int i=0; i < rows_; ++i) {
            for (int k=0; k < cols_; ++k) {
                double a_ik = data_[static_cast<std::size_t>(i) * cols_ + k];

                for (int j=0; j < second.cols_; ++j) {
                    result.data_[static_cast<std::size_t>(i) * result.cols_ + j] += a_ik *
                        second.data_[static_cast<std::size_t>(k) * second.cols_ + j];
                }
            }
        }
        return result;
    }

    DenseMatrix multiply_sparse(const SparseMatrix& second) const;

    std::unique_ptr<Matrix> mul(const Matrix& second) const override;

    int get_rows() const override {
        return rows_;
    }

    int get_cols() const override {
        return cols_;
    }

    double get_element(int row, int col) const override {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::out_of_range("Matrix index out of range");
        }

        return data_[static_cast<std::size_t>(row) * cols_ + col];
    }

    void set_element(int row, int col, double value) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::out_of_range("Matrix index out of range");
        }

        data_[static_cast<std::size_t>(row) * cols_ + col] = value;
    }
};

#endif //CPP_HOMEWORK_DENSEMATRIX_H
