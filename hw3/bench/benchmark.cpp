#include "../../hw2/DenseMatrix.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

DenseMatrix make_matrix(int size, std::mt19937& random) {
    std::uniform_real_distribution<double> value(-1.0, 1.0);
    DenseMatrix matrix(size, size);
    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            matrix.set_element(row, col, value(random));
        }
    }
    return matrix;
}

template <typename Multiply>
double median_seconds(Multiply multiply, double& checksum) {
    using Clock = std::chrono::steady_clock;
    std::array<double, 3> times{};
    for (double& seconds : times) {
        const auto start = Clock::now();
        const DenseMatrix result = multiply();
        const auto end = Clock::now();
        seconds = std::chrono::duration<double>(end - start).count();

        for (int row = 0; row < result.get_rows(); ++row) {
            for (int col = 0; col < result.get_cols(); ++col) {
                checksum += result.get_element(row, col);
            }
        }
    }
    std::sort(times.begin(), times.end());
    return times[1];
}

// Both versions use the same algorithm. The second mutex only changes how
// often the workers lock during the arithmetic.
template <bool lock_elements>
DenseMatrix multiply_for_lock_experiment(const DenseMatrix& a, const DenseMatrix& b,
                                         int thread_count) {
    const int rows = a.get_rows();
    const int inner = a.get_cols();
    const int cols = b.get_cols();
    DenseMatrix result(rows, cols);
    int next_row = 0;
    std::mutex row_mutex;
    std::mutex element_mutex;

    auto worker = [&] {
        while (true) {
            int row;
            {
                std::lock_guard<std::mutex> lock(row_mutex);
                if (next_row >= rows) {
                    return;
                }
                row = next_row++;
            }

            for (int k = 0; k < inner; ++k) {
                const double a_ik = a.get_element(row, k);
                for (int col = 0; col < cols; ++col) {
                    const double product = a_ik * b.get_element(k, col);
                    std::unique_lock<std::mutex> lock(element_mutex, std::defer_lock);
                    if (lock_elements) {
                        lock.lock();
                    }
                    result.set_element(row, col, result.get_element(row, col) + product);
                }
            }
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    auto join_threads = [&] {
        for (auto& thread : threads) {
            thread.join();
        }
    };
    try {
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back(worker);
        }
    } catch (...) {
        join_threads();
        throw;
    }
    join_threads();
    return result;
}

void check_lock_experiment() {
    std::mt19937 random(31);
    const DenseMatrix a = make_matrix(15, random);
    const DenseMatrix b = make_matrix(15, random);
    const DenseMatrix expected = a.multiply_naive(b);
    const DenseMatrix row_lock = multiply_for_lock_experiment<false>(a, b, 4);
    const DenseMatrix element_lock = multiply_for_lock_experiment<true>(a, b, 4);
    for (int row = 0; row < expected.get_rows(); ++row) {
        for (int col = 0; col < expected.get_cols(); ++col) {
            const double value = expected.get_element(row, col);
            if (!(std::abs(row_lock.get_element(row, col) - value) <= 1e-9 &&
                  std::abs(element_lock.get_element(row, col) - value) <= 1e-9)) {
                throw std::runtime_error("Lock experiment produced an incorrect result");
            }
        }
    }
}

} // namespace

int main() {
    constexpr int size = 1000;
    constexpr int lock_size = 192;
    constexpr int lock_threads = 4;
    constexpr int thread_counts[] = {1, 2, 3, 4, 8, 14, 15, 16, 32};

    check_lock_experiment();
    std::mt19937 random(42);
    double checksum = 0.0;
    const DenseMatrix a = make_matrix(size, random);
    const DenseMatrix b = make_matrix(size, random);

    const double naive_time = median_seconds([&] {
        return a.multiply_naive(b);
    }, checksum);
    const double sequential_time = median_seconds([&] {
        return a.multiply_optimized(b);
    }, checksum);

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Hardware concurrency hint: " << std::thread::hardware_concurrency() << '\n'
              << "Matrices: " << size << " x " << size << ", median of 3 runs\n"
              << "Sequential naive (multiply_naive): " << naive_time << " s\n"
              << "Sequential optimized (multiply_optimized): " << sequential_time << " s\n\n"
              << std::setw(8) << "Threads" << std::setw(14) << "Time (s)"
              << std::setw(15) << "vs 1 thread" << std::setw(17) << "Efficiency (%)"
              << std::setw(16) << "vs seq. opt." << '\n';
    double one_thread_time = 0.0;
    for (int threads : thread_counts) {
        const double seconds = median_seconds([&] {
            return a.multiply_parallel(b, threads);
        }, checksum);
        if (threads == 1) {
            one_thread_time = seconds;
        }
        const double speedup = one_thread_time / seconds;
        std::cout << std::setw(8) << threads << std::setw(14) << seconds
                  << std::setw(15) << speedup << std::setw(17) << 100.0 * speedup / threads
                  << std::setw(16) << sequential_time / seconds
                  << std::endl;
    }

    const DenseMatrix small_a = make_matrix(lock_size, random);
    const DenseMatrix small_b = make_matrix(lock_size, random);
    const double row_lock_time = median_seconds([&] {
        return multiply_for_lock_experiment<false>(small_a, small_b, lock_threads);
    }, checksum);
    const double element_lock_time = median_seconds([&] {
        return multiply_for_lock_experiment<true>(small_a, small_b, lock_threads);
    }, checksum);
    std::cout << "\nGranularity experiment: " << lock_size << " x " << lock_size
              << ", " << lock_threads << " threads, median of 3 runs\n"
              << "Lock on row counter only: " << row_lock_time << " s\n"
              << "Also lock on each C[i][j] += update: " << element_lock_time << " s\n"
              << "Slowdown (element / row): " << element_lock_time / row_lock_time << "x\n"
              << "Checksum: " << checksum << '\n';
}
