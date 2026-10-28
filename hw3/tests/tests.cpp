#include "../SparseMatrix.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

void check_equal(const Matrix& actual, const Matrix& expected) {
    if (actual.get_rows() != expected.get_rows() ||
        actual.get_cols() != expected.get_cols()) {
        throw std::runtime_error("Matrix dimensions differ");
    }

    for (int i = 0; i < actual.get_rows(); ++i) {
        for (int j = 0; j < actual.get_cols(); ++j) {
            double difference = std::abs(
                actual.get_element(i, j) - expected.get_element(i, j)
            );
            if (!std::isfinite(difference) || difference > 1e-9) {
                throw std::runtime_error(
                    "Matrix elements differ at (" + std::to_string(i) +
                    ", " + std::to_string(j) + ")"
                );
            }
        }
    }
}

void check_product(const Matrix& a, const Matrix& b,
                   const Matrix& expected, const std::string& label) {
    try {
        std::unique_ptr<Matrix> result = a.mul(b);
        if (!result) {
            throw std::runtime_error("mul returned a null pointer");
        }
        check_equal(*result, expected);
    } catch (const std::exception& error) {
        throw std::runtime_error(label + ": " + error.what());
    }
}

// Во всех четырёх случаях меняется только представление исходных данных.
void check_all_combinations(const DenseMatrix& a, const DenseMatrix& b,
                            const DenseMatrix& expected) {
    SparseMatrix sparse_a(a);
    SparseMatrix sparse_b(b);

    check_product(a, b, expected, "Dense x Dense");
    check_product(a, sparse_b, expected, "Dense x Sparse");
    check_product(sparse_a, b, expected, "Sparse x Dense");
    check_product(sparse_a, sparse_b, expected, "Sparse x Sparse");
}

void test_csr_conversion() {
    DenseMatrix dense(3, 4, {0, 5, 0, 2,
                            0, 0, 0, 0,
                            3, 0, 7, 0});
    SparseMatrix sparse(dense);
    check_equal(sparse, dense);

    const std::size_t boundaries[] = {0, 2, 2, 4};
    const double values[] = {5, 2, 3, 7};
    const int columns[] = {1, 3, 0, 2};

    for (int row = 0; row < 3; ++row) {
        if (sparse.get_row_start(row) != boundaries[row] ||
            sparse.get_row_end(row) != boundaries[row + 1]) {
            throw std::runtime_error("Incorrect CSR row boundaries");
        }
    }
    for (std::size_t position = 0; position < 4; ++position) {
        if (sparse.get_value(position) != values[position] ||
            sparse.get_col_index(position) != columns[position]) {
            throw std::runtime_error("Incorrect CSR record");
        }
    }

    SparseMatrix zero(3, 4);
    DenseMatrix dense_zero(3, 4);
    check_equal(zero, dense_zero);
    for (int row = 0; row < 3; ++row) {
        if (zero.get_row_start(row) != 0 || zero.get_row_end(row) != 0) {
            throw std::runtime_error("Incorrect zero matrix CSR boundaries");
        }
    }
}

void test_small_multiplication() {
    DenseMatrix a(3, 4, {0, 5, 0, 2,
                         0, 0, 0, 0,
                         3, 0, 7, 0});
    DenseMatrix b(4, 3, {0, 2, 0,
                         4, 0, 1,
                         0, 3, 0,
                         2, 0, 5});
    DenseMatrix expected(3, 3, {24, 0, 15,
                                0, 0, 0,
                                0, 27, 0});
    check_all_combinations(a, b, expected);
}

void test_identity() {
    DenseMatrix a(2, 3, {0, -2, 3,
                         4, 0, 5});
    DenseMatrix right_identity(3, 3, {1, 0, 0,
                                      0, 1, 0,
                                      0, 0, 1});
    DenseMatrix left_identity(2, 2, {1, 0,
                                     0, 1});
    check_all_combinations(a, right_identity, a);
    check_all_combinations(left_identity, a, a);
}

void test_zero() {
    DenseMatrix a(2, 3, {1, 0, -3,
                         4, 5, 0});
    DenseMatrix right_zero(3, 4);
    DenseMatrix left_zero(4, 2);
    DenseMatrix expected_right(2, 4);
    DenseMatrix expected_left(4, 3);

    check_all_combinations(a, right_zero, expected_right);
    check_all_combinations(left_zero, a, expected_left);
}

void expect_incompatible(const Matrix& a, const Matrix& b,
                         const std::string& label) {
    try {
        a.mul(b);
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error(label + ": incompatible dimensions were accepted");
}

void test_incompatible_dimensions() {
    DenseMatrix a(2, 3);
    DenseMatrix b(2, 4);
    SparseMatrix sparse_a(a);
    SparseMatrix sparse_b(b);

    expect_incompatible(a, b, "Dense x Dense");
    expect_incompatible(a, sparse_b, "Dense x Sparse");
    expect_incompatible(sparse_a, b, "Sparse x Dense");
    expect_incompatible(sparse_a, sparse_b, "Sparse x Sparse");
}

void test_cancellation() {
    // Два ненулевых вклада полностью уничтожаются в каждом столбце.
    DenseMatrix a(1, 2, {1, 1});
    DenseMatrix b(2, 3, {2, -3, 4,
                        -2, 3, -4});
    DenseMatrix expected(1, 3);
    check_all_combinations(a, b, expected);

    SparseMatrix sparse_a(a);
    SparseMatrix sparse_b(b);
    std::unique_ptr<Matrix> result = sparse_a.mul(sparse_b);
    const SparseMatrix* sparse_result =
        dynamic_cast<const SparseMatrix*>(result.get());
    if (sparse_result == nullptr ||
        sparse_result->get_row_start(0) != 0 ||
        sparse_result->get_row_end(0) != 0) {
        throw std::runtime_error("Cancelled entries were not removed from CSR");
    }
}

void test_zero_dimensions() {
    const int shapes[][3] = {{0, 3, 2}, {2, 0, 4}, {2, 3, 0}, {0, 0, 0}};
    for (const auto& shape : shapes) {
        DenseMatrix a(shape[0], shape[1]);
        DenseMatrix b(shape[1], shape[2]);
        DenseMatrix expected(shape[0], shape[2]);
        check_all_combinations(a, b, expected);
    }
}

DenseMatrix generate_matrix(int rows, int cols, double density,
                             std::mt19937& generator) {
    DenseMatrix matrix(rows, cols);
    std::bernoulli_distribution keep(density);
    std::uniform_real_distribution<double> value(-5.0, 5.0);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            if (keep(generator)) {
                matrix.set_element(row, col, value(generator));
            }
        }
    }
    return matrix;
}

void test_random_matrices() {
    std::mt19937 generator(42);
    const int shapes[][3] = {
        {1, 1, 1}, {2, 3, 4}, {7, 5, 9}, {20, 30, 15}, {31, 17, 23}
    };
    const double densities[] = {0.0, 0.001, 0.01, 0.1, 0.5, 1.0};

    for (const auto& shape : shapes) {
        for (double density : densities) {
            // Одинаковая плотность обоих множителей.
            DenseMatrix a = generate_matrix(shape[0], shape[1], density, generator);
            DenseMatrix b = generate_matrix(shape[1], shape[2], density, generator);
            DenseMatrix expected = a.multiply_naive(b);
            check_all_combinations(a, b, expected);

            // Разреженные данные первого множителя и полностью заполненный второй.
            DenseMatrix full_b = generate_matrix(shape[1], shape[2], 1.0, generator);
            DenseMatrix mixed_expected = a.multiply_naive(full_b);
            check_all_combinations(a, full_b, mixed_expected);
        }
    }
}

int main() {
    try {
        test_csr_conversion();
        std::cout << "PASSED: CSR conversion and empty rows\n";
        test_small_multiplication();
        std::cout << "PASSED: Small rectangular multiplication (all four combinations)\n";
        test_identity();
        std::cout << "PASSED: Left and right identity matrices\n";
        test_zero();
        std::cout << "PASSED: Left and right zero matrices\n";
        test_incompatible_dimensions();
        std::cout << "PASSED: Incompatible dimensions (all four combinations)\n";
        test_cancellation();
        std::cout << "PASSED: Cancellation and removal of zero CSR entries\n";
        test_zero_dimensions();
        std::cout << "PASSED: Zero dimensions\n";
        test_random_matrices();
        std::cout << "PASSED: Random cross-check against HW2 naive multiplication\n";
        std::cout << "All tests passed\n";
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
