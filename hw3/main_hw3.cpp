//
// Created by konst on 28.10.2026.
//

#include "../hw2/Matrix.h"
#include "../hw2/DenseMatrix.h"
#include "SparseMatrix.h"

#include <iostream>
#include <vector>

int main() {
    DenseMatrix first(3, 4, {0, 5, 0, 2,
                                                0, 0, 0, 0,
                                                3, 0, 7, 0});
    const Matrix& a = first;


    DenseMatrix second(4, 3,{0, 2, 0,
        4, 0, 1,
        0, 3, 0,
        2, 0, 5});
    SparseMatrix secondSparse = SparseMatrix(second);
    const Matrix& b = secondSparse;

    std::unique_ptr<Matrix> result = a.mul(b);

    for (int i = 0; i < result->get_rows(); ++i) {
        for (int j = 0; j < result->get_cols(); ++j) {
            std::cout << result->get_element(i, j) << ' ';
        }
        std::cout << std::endl;
    }

    // DenseMatrix result = first.multiply_sparse(secondSparse);
    //
    // for (int i = 0; i < result.get_rows(); ++i) {
    //     for (int j = 0; j < result.get_cols(); ++j) {
    //         std::cout << result.get_element(i, j) << ' ';
    //     }
    //     std::cout << std::endl;
    // }
}