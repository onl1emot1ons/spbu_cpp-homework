//
// Created by konst on 28.10.2026.
//

#include "../hw2/DenseMatrix.h"
#include "SparseMatrix.h"
#include <stdexcept>

DenseMatrix DenseMatrix::multiply_sparse(const SparseMatrix& second) const {
    if (cols_ != second.get_rows()) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }
    DenseMatrix result(rows_, second.get_cols());

    for (int i = 0; i < rows_; ++i) {
        for (int k = 0; k < cols_; ++k) {
            double a_ik = get_element(i, k);
            if (a_ik != 0.0) {
                for (std::size_t position = second.get_row_start(k); position < second.get_row_end(k); ++position) {
                    double b_kj = second.get_value(position);
                    int j = second.get_col_index(position);
                    result.data_[static_cast<std::size_t>(i) * result.cols_ + j] += a_ik * b_kj;
                }
            }
        }
    }
    return result;
}

std::unique_ptr<Matrix> DenseMatrix::mul(const Matrix& second) const {
    const DenseMatrix* dense = dynamic_cast<const DenseMatrix*>(&second);
    if (dense != nullptr) {
        return std::make_unique<DenseMatrix>(multiply_optimized(*dense));
    }

    const SparseMatrix* sparse = dynamic_cast<const SparseMatrix*>(&second);
    if (sparse != nullptr) {
        return std::make_unique<SparseMatrix>(multiply_sparse(*sparse));
    }

    throw std::invalid_argument("Unsupported matrix type");
}