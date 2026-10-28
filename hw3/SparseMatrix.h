//
// Created by konst on 28.10.2026.
//

#ifndef CPP_HOMEWORK_SPARSEMATRIX_H
#define CPP_HOMEWORK_SPARSEMATRIX_H


#include "../hw2/Matrix.h"
#include "../hw2/DenseMatrix.h"
#include <vector>
#include <cstddef>
#include <stdexcept>


class SparseMatrix : public Matrix {
private:
    int rows_;
    int cols_;

    std::vector<double> values_;
    std::vector<int> col_indices_;
    std::vector<std::size_t> row_ptrs_;
public:
    SparseMatrix(int rows, int cols) : rows_(rows), cols_(cols) {
        if (rows < 0 || cols < 0) {
            throw std::invalid_argument(
                "Matrix dimensions must be non-negative"
                );
        }
        row_ptrs_.resize(static_cast<std::size_t>(rows) + 1, 0);
    }
    explicit SparseMatrix(const DenseMatrix& dense) : rows_(dense.get_rows()), cols_(dense.get_cols()) {
        row_ptrs_.push_back(0);
        for (int row = 0; row < rows_; row++) {
            for (int col = 0; col < cols_; col++) {
                double value = dense.get_element(row, col);
                if (value != 0.0) {
                    values_.push_back(value);
                    col_indices_.push_back(col);
                }
            }

            row_ptrs_.push_back(values_.size());
        }
    }


    DenseMatrix multiply_dense(const DenseMatrix& second) const {
        if (cols_ != second.get_rows()) {
            throw std::invalid_argument("Incompatible matrix dimensions");
        }
        DenseMatrix result(rows_, second.get_cols());
        for (int i = 0; i < rows_; i++) {
            for (std::size_t position = row_ptrs_[i]; position < row_ptrs_[i + 1]; position++) {
                double a_ik = values_[position];
                int k = col_indices_[position];

                for (int j = 0; j < second.get_cols(); j++) {
                    result.set_element(i, j, result.get_element(i, j) + a_ik * second.get_element(k, j));
                }
            }
        }
        return result;
    }

    SparseMatrix multiply_sparse(const SparseMatrix& second) const {
        if (cols_ != second.get_rows()) {
            throw std::invalid_argument("Incompatible matrix dimensions");
        }
        SparseMatrix result(rows_, second.cols_);
        for (int i = 0; i < rows_; i++) {
            std::vector<double> row_values(second.cols_, 0);

            for (std::size_t position = row_ptrs_[i]; position < row_ptrs_[i + 1]; position++) {
                double a_ik = values_[position];
                int k = col_indices_[position];
                for (std::size_t j = second.row_ptrs_[k]; j < second.row_ptrs_[k + 1]; j++) {
                    double b_kj = second.values_[j];
                    int b_j = second.col_indices_[j];
                    row_values[b_j] += a_ik * b_kj;
                }
            }
            for (int l = 0; l < second.cols_; l++) {
                if (row_values[l] != 0) {
                    result.values_.push_back(row_values[l]);
                    result.col_indices_.push_back(l);
                }
            }
            result.row_ptrs_[i+1] = result.values_.size();
        }
        return result;
    }

    std::unique_ptr<Matrix> mul(const Matrix& second) const override { // тоже разобрать ещё раз
        const DenseMatrix* dense = dynamic_cast<const DenseMatrix*>(&second);
        if (dense != nullptr) {
            return std::make_unique<DenseMatrix>(multiply_dense(*dense));
        }

        const SparseMatrix* sparse = dynamic_cast<const SparseMatrix*>(&second);
        if (sparse != nullptr) {
            return std::make_unique<SparseMatrix>(multiply_sparse(*sparse));
        }

        throw std::invalid_argument("Unsupported matrix type");
    }

    int get_rows() const override {
        return rows_;
    }
    int get_cols() const override {
        return cols_;
    }
    double get_element(int row, int column) const override {
        if (row < 0 || row >= rows_ || column < 0 || column >= cols_) {
            throw std::out_of_range("Matrix index out of range");
        }

        for (std::size_t position = row_ptrs_[row]; position < row_ptrs_[row + 1]; position++) {
            if (col_indices_[position] == column) {
                return values_[position];
            }
        }
        return 0.0;
    }
    std::size_t get_row_start(int row) const {
        if (row < 0 || row >= rows_) {
            throw std::out_of_range("Matrix index out of range");
        }
        return row_ptrs_[row];
    }
    std::size_t get_row_end(int row) const {
        if (row < 0 || row >= rows_) {
            throw std::out_of_range("Matrix index out of range");
        }
        return row_ptrs_[row+1];
    }
    double get_value(std::size_t position) const {
        return values_.at(position);
    }
    int get_col_index(std::size_t position) const {
        return col_indices_.at(position);
    }
};


#endif //CPP_HOMEWORK_SPARSEMATRIX_H
