//
// Created by konst on 19.11.2026.
//

#include <cstddef>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <thread>
#include "../hw2/DenseMatrix.h"

DenseMatrix DenseMatrix::multiply_parallel(const DenseMatrix &second, int flows) const {
    if (cols_ != second.rows_) {
        throw std::invalid_argument("Incompatible matrix dimensions");
    }
    if (flows <= 0) {
        throw std::invalid_argument("Incorrect number of flows");
    }

    DenseMatrix result(rows_, second.cols_);

    int next_row = 0;
    std::mutex row_mutex;
    auto worker = [&] {
        while (true) {
            int i;
            {
                std::lock_guard<std::mutex> lock(row_mutex);
                if (next_row >= rows_) {
                    return;
                }
                i = next_row++;
            }
            for (int k=0; k < cols_; ++k) {
                double a_ik = data_[static_cast<std::size_t>(i) * cols_ + k];

                for (int j=0; j < second.cols_; ++j) {
                    result.data_[static_cast<std::size_t>(i) * result.cols_ + j] += a_ik *
                        second.data_[static_cast<std::size_t>(k) * second.cols_ + j];
                }
            }
        }
    };

    std::vector<std::thread> treads;
    for (int t=0; t < flows; ++t) {
        treads.emplace_back(worker);
    }
    for (auto& thrd : treads) {
        thrd.join();
    }

    return result;
}
