//
// Created by konst on 19.11.2026.
//

#include <vector>
#include <mutex>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "MatrixIO.h"
#include "../hw2/DenseMatrix.h"


int main() {
    DenseMatrix first = load_matrix("matrix_a.txt");
    DenseMatrix second(2, 2, {7, 8,
                                                9, 10});

    DenseMatrix result = first.multiply_parallel(second, 4);
    for (int i = 0; i < result.get_rows(); ++i) {
        for (int j = 0; j < result.get_cols(); ++j) {
            std::cout << result.get_element(i, j) << ' ';
        }
        std::cout << std::endl;
    }

    save_matrix(result, "result.txt");
}