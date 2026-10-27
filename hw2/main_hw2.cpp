#include <iostream>
#include "DenseMatrix.h"
#include <chrono>

// using namespace std;


int main() {

    DenseMatrix a(2, 3, {1, 2, 3,
                     4, 5, 6});

    DenseMatrix b(3, 2, {7,  8,
                         9, 10,
                        11, 12});

    const Matrix& first = a;

    std::unique_ptr<Matrix> result = first.mul(b);

    for (int i = 0; i < result->get_rows(); ++i) {
        for (int j = 0; j < result->get_cols(); ++j) {
            std::cout << result->get_element(i, j) << ' ';
        }
        std::cout << '\n';
    }

    return 0;
}