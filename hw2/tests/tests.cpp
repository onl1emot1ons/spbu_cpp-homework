//
// Created by konst on 27.10.2026.
//


#include "../DenseMatrix.h"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <random>

void check_equal(const DenseMatrix& actual,
                 const DenseMatrix& expected) {
    if (actual.get_rows() != expected.get_rows() ||
        actual.get_cols() != expected.get_cols()) {
        throw std::runtime_error("Matrix dimensions differ");
        }

    for (int i = 0; i < actual.get_rows(); ++i) {
        for (int j = 0; j < actual.get_cols(); ++j) {
            double difference = std::abs(
                actual.get_element(i, j) -
                expected.get_element(i, j)
            );

            if (!std::isfinite(difference) || difference > 1e-9) {
                throw std::runtime_error("Matrix elements differ");
            }
        }
    }
}

void test_small_multiplication() {
    DenseMatrix a(2, 3, {1, 2, 3,
                         4, 5, 6});

    DenseMatrix b(3, 2, {7,  8,
                         9, 10,
                        11, 12});

    DenseMatrix naive = a.multiply_naive(b);
    DenseMatrix optimized = a.multiply_optimized(b);

    if (naive.get_rows() != 2 || naive.get_cols() != 2 ||
        optimized.get_rows() != 2 || optimized.get_cols() != 2) {
        throw std::runtime_error("Incorrect result dimensions");
        }

    const double expected[2][2] = {
        {58, 64},
        {139, 154}
    };

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            if (std::abs(naive.get_element(i, j) - expected[i][j]) > 1e-9 ||
                std::abs(optimized.get_element(i, j) - expected[i][j]) > 1e-9) {
                throw std::runtime_error("Incorrect multiplication result");
                }
        }
    }
}

void test_identity() {
    DenseMatrix a(2, 3, {1, 2, 3,
                         4, 5, 6});

    DenseMatrix identity(3, 3, {1, 0, 0,
                               0, 1, 0,
                               0, 0, 1});

    check_equal(a.multiply_naive(identity), a);
    check_equal(a.multiply_optimized(identity), a);
}

void test_zero() {
    DenseMatrix a(2, 3, {1, 2, 3,
                         4, 5, 6});

    DenseMatrix zero(3, 4);
    DenseMatrix expected(2, 4);

    check_equal(a.multiply_naive(zero), expected);
    check_equal(a.multiply_optimized(zero), expected);
}

void test_incompatible_dimensions() {
    DenseMatrix a(2, 3);
    DenseMatrix b(2, 4);

    bool naive_threw = false;
    bool optimized_threw = false;

    try {
        a.multiply_naive(b);
    } catch (const std::invalid_argument&) {
        naive_threw = true;
    }

    try {
        a.multiply_optimized(b);
    } catch (const std::invalid_argument&) {
        optimized_threw = true;
    }

    if (!naive_threw || !optimized_threw) {
        throw std::runtime_error(
            "Incompatible dimensions were not rejected"
        );
    }
}

void test_random_matrices() {
    std::mt19937 generator(42);
    std::uniform_real_distribution<double> distribution(-5.0, 5.0);

    const int shapes[][3] = {
        {1, 1, 1},
        {2, 3, 4},
        {7, 5, 9},
        {20, 30, 15}
    };

    for (const auto& shape : shapes) {
        int m = shape[0];
        int n = shape[1];
        int p = shape[2];

        DenseMatrix a(m, n);
        DenseMatrix b(n, p);

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                a.set_element(i, j, distribution(generator));
            }
        }

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < p; ++j) {
                b.set_element(i, j, distribution(generator));
            }
        }

        check_equal(
            a.multiply_naive(b),
            a.multiply_optimized(b)
        );
    }
}

void test_matrix_interface() {
    DenseMatrix a(2, 3, {1, 2, 3,
                         4, 5, 6});

    DenseMatrix b(3, 2, {7,  8,
                         9, 10,
                        11, 12});

    DenseMatrix expected(2, 2, {58, 64,
                               139, 154});

    const Matrix& first = a;
    const Matrix& second = b;

    std::unique_ptr<Matrix> result = first.mul(second);

    if (!result) {
        throw std::runtime_error("mul returned a null pointer");
    }

    if (result->get_rows() != expected.get_rows() ||
        result->get_cols() != expected.get_cols()) {
        throw std::runtime_error("Incorrect interface result dimensions");
        }

    for (int i = 0; i < expected.get_rows(); ++i) {
        for (int j = 0; j < expected.get_cols(); ++j) {
            double difference = std::abs(
                result->get_element(i, j) - expected.get_element(i, j)
            );

            if (!std::isfinite(difference) || difference > 1e-9) {
                throw std::runtime_error("Incorrect interface result");
            }
        }
    }
}

int main() {
    try {
        test_small_multiplication();
        std::cout << "PASSED: Small multiplication\n";
        test_identity();
        std::cout << "PASSED: Identity matrix multiplication\n";
        test_zero();
        std::cout << "PASSED: Zero matrix multiplication\n";
        test_incompatible_dimensions();
        std::cout << "PASSED: Incompatible dimensions\n";
        test_random_matrices();
        std::cout << "PASSED: Random matrices\n";
        test_matrix_interface();
        std::cout << "PASSED: Matrix interface\n";

        std::cout << "All tests passed\n";
    } catch (const std::exception& error) {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }

    return 0;
}