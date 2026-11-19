//
// Created by konst on 19.11.2026.
//

#include "../hw2/DenseMatrix.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <fstream>

DenseMatrix load_matrix(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    int rows = 0;
    int cols = 0;
    if (!(input >> rows >> cols)) {
        throw std::runtime_error("Cannot read matrix dimensions");
    }

    if (rows < 0 || cols < 0) {
        throw std::runtime_error("Matrix dimensions must be non-negative");
    }

    DenseMatrix matrix(rows, cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double value = 0;
            if (!(input >> value)) {
                throw std::runtime_error("Cannot read matrix value");
            }
            matrix.set_element(r, c, value);
        }
    }

    return matrix;
}

void save_matrix(const DenseMatrix& matrix, const std::string& path) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    int r = matrix.get_rows();
    int c = matrix.get_cols();

    output << r << ' ' << c << '\n';

    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (j > 0) {
                output << ' ';
            }
            output << matrix.get_element(i, j);
        }
        output << '\n';
    }
}
