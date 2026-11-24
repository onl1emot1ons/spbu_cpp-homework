//
// Created by konst on 19.11.2026.
//

#include "../hw2/DenseMatrix.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>
#include <limits>
#include <iomanip>
#include <vector>
#include <fstream>

DenseMatrix load_matrix(const std::string& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::vector<double> data;
    std::size_t rows = 0;
    std::size_t cols = 0;
    std::string line;

    while (std::getline(input, line)) {
        std::istringstream ss(line);
        std::size_t cnt_cols = 0;
        std::string token;

        while (ss >> token) {
            std::istringstream number(token);
            double value;
            if (!(number >> value) || !number.eof()) {
                throw std::runtime_error("Invalid number: " + token);
            }

            data.push_back(value);
            ++cnt_cols;
        }

        if (cnt_cols == 0) {
            throw std::runtime_error("Matrix contains an empty row");
        }

        if (rows == 0) {
            cols = cnt_cols;
        }
        else if (cnt_cols != cols) {
            throw std::runtime_error("Matrix rows have different lengths");
        }

        ++rows;
    }

    if (input.bad() || !input.eof()) {
        throw std::runtime_error("Cannot read file: " + path);
    }
    if (rows == 0) {
        throw std::runtime_error("Matrix file has no numbers");
    }
    if (rows > std::numeric_limits<int>::max() || cols > std::numeric_limits<int>::max()) {
        throw std::runtime_error("Matrix dimensions are too large");
    }

    return DenseMatrix(static_cast<int>(rows), static_cast<int>(cols), data);

}

void save_matrix(const DenseMatrix& matrix, const std::string& path) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    output << std::setprecision(std::numeric_limits<double>::max_digits10);

    int r = matrix.get_rows();
    int c = matrix.get_cols();

    if (r == 0 || c == 0) {
        throw std::runtime_error("Cannot save a matrix with a zero dimension");
    }

    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (j > 0) {
                output << ' ';
            }
            output << matrix.get_element(i, j);
        }
        output << '\n';
    }

    output.close();
    if (!output) {
        throw std::runtime_error("Cannot write file: " + path);
    }
}
