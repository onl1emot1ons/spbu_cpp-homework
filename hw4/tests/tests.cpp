#include "../../hw2/DenseMatrix.h"
#include "../MatrixIO.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void check_matrix(const DenseMatrix& actual, const DenseMatrix& expected,
                  double tolerance = 1e-9) {
    require(actual.get_rows() == expected.get_rows() &&
            actual.get_cols() == expected.get_cols(), "Matrix sizes differ");

    for (int row = 0; row < expected.get_rows(); ++row) {
        for (int col = 0; col < expected.get_cols(); ++col) {
            const double difference =
                std::abs(actual.get_element(row, col) - expected.get_element(row, col));
            if (!(difference <= tolerance)) {
                throw std::runtime_error("Matrix values differ at (" +
                    std::to_string(row) + ", " + std::to_string(col) + ")");
            }
        }
    }
}

DenseMatrix make_matrix(int rows, int cols) {
    DenseMatrix matrix(rows, cols);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            matrix.set_element(row, col, ((row + 1) * (col + 2) - 11) / 7.0);
        }
    }
    return matrix;
}

void test_known_product() {
    const DenseMatrix a(3, 2, {1, 2, 3, 4, 5, 6});
    const DenseMatrix b(2, 2, {7, 8, 9, 10});
    const DenseMatrix expected(3, 2, {25, 28, 57, 64, 89, 100});
    check_matrix(a.multiply_parallel(b, 4), expected);
}

void test_thread_counts() {
    const DenseMatrix a = make_matrix(12, 5);
    const DenseMatrix b = make_matrix(5, 7);
    const DenseMatrix expected = a.multiply_naive(b);

    for (int threads : {1, 2, 4, 8}) {
        try {
            check_matrix(a.multiply_parallel(b, threads), expected);
        } catch (const std::exception& error) {
            throw std::runtime_error(std::to_string(threads) +
                " thread(s), " + error.what());
        }
    }
}

void test_uneven_rows() {
    const DenseMatrix a = make_matrix(7, 3);
    const DenseMatrix b = make_matrix(3, 5);
    check_matrix(a.multiply_parallel(b, 4), a.multiply_naive(b));
}

void test_zero_dimensions() {
    const DenseMatrix no_rows(0, 3);
    const DenseMatrix right(3, 2);
    check_matrix(no_rows.multiply_parallel(right, 4), DenseMatrix(0, 2));

    const DenseMatrix a(3, 0);
    const DenseMatrix b(0, 2);
    check_matrix(a.multiply_parallel(b, 4), DenseMatrix(3, 2));
}

template <typename Exception, typename Operation>
void expect_exception(Operation operation, const char* message) {
    try {
        operation();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(message);
}

void test_bad_arguments() {
    const DenseMatrix a(2, 3);
    const DenseMatrix b(3, 2);
    expect_exception<std::invalid_argument>([&] { a.multiply_parallel(a, 4); },
                                            "Accepted incompatible dimensions");
    for (int threads : {0, -1}) {
        expect_exception<std::invalid_argument>([&] { a.multiply_parallel(b, threads); },
                                                "Accepted an invalid thread count");
    }
}

struct TestFile {
    explicit TestFile(const std::string& purpose)
        : path("hw4_test_" + purpose + "_" + std::to_string(
              std::chrono::steady_clock::now().time_since_epoch().count()) + ".txt") {}

    ~TestFile() {
        std::remove(path.c_str());
    }

    const std::string path;
};

void write_fixture(const std::string& path, const char* contents) {
    std::ofstream file(path, std::ios::binary);
    file << contents;
    file.close();
    require(static_cast<bool>(file), "Could not write test file " + path);
}

void test_matrix_file_round_trip() {
    const TestFile input("input");
    const TestFile output("output");
    // Windows line endings, mixed whitespace and no newline after the last row.
    write_fixture(input.path, "  0.125\t-2.75 0\r\n1.2345678901234567 4.25 -0.5");
    const DenseMatrix expected(2, 3, {0.125, -2.75, 0.0, 1.2345678901234567, 4.25, -0.5});

    const DenseMatrix loaded = load_matrix(input.path);
    check_matrix(loaded, expected, 0.0);
    save_matrix(loaded, output.path);
    check_matrix(load_matrix(output.path), expected, 0.0);

    std::ifstream saved(output.path);
    std::string line;
    for (int row = 0; row < expected.get_rows(); ++row) {
        require(static_cast<bool>(std::getline(saved, line)), "Missing row in saved file");
        std::istringstream values(line);
        for (int col = 0; col < expected.get_cols(); ++col) {
            double value = 0.0;
            require((values >> value) && value == expected.get_element(row, col),
                    "Saved file must contain only matrix entries, row by row");
        }
        std::string extra;
        require(!(values >> extra), "Extra entry in saved file");
    }
    require(!std::getline(saved, line) && saved.eof() && !saved.bad(),
            "Extra row or read error in saved file");
}

void test_bad_matrix_files() {
    const TestFile file("invalid");
    const char* invalid_files[] = {
        "",
        "1 2\n\n3 4\n",
        "1 2\n \t\n3 4\n",
        "1 2\n3\n",
        "1 2\n3 4 5\n",
        "1 2\n3 abc\n",
        "1 2\n3 12abc\n",
    };
    for (const char* contents : invalid_files) {
        write_fixture(file.path, contents);
        expect_exception<std::runtime_error>([&] { load_matrix(file.path); },
                                             "Accepted invalid matrix file");
    }
}

} // namespace

int main() {
    const struct {
        const char* name;
        void (*run)();
    } tests[] = {
        {"Known rectangular product", test_known_product},
        {"Sequential reference, 1, 2, 4 and 8 threads", test_thread_counts},
        {"Uneven split, 7 rows on 4 threads", test_uneven_rows},
        {"Zero dimensions", test_zero_dimensions},
        {"Invalid matrix dimensions and thread counts", test_bad_arguments},
        {"Headerless matrix load, save and reload", test_matrix_file_round_trip},
        {"Empty files, invalid row lengths and invalid numbers", test_bad_matrix_files},
    };

    int failures = 0;
    for (const auto& test : tests) {
        try {
            test.run();
            std::cout << "PASS " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "FAIL " << test.name << " - " << error.what() << '\n';
            ++failures;
        }
    }

    return failures == 0 ? 0 : 1;
}
