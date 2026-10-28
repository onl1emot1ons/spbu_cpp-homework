#include "../SparseMatrix.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

// Выбираем точное число ненулевых позиций без повторений.
DenseMatrix generate_matrix(int n, double density, std::mt19937& generator) {
    DenseMatrix matrix(n, n);
    std::size_t count = static_cast<std::size_t>(n) * n;
    std::size_t nonzero = static_cast<std::size_t>(
        std::llround(density * static_cast<double>(count))
    );
    std::vector<std::size_t> positions(count);
    std::iota(positions.begin(), positions.end(), std::size_t{0});
    std::shuffle(positions.begin(), positions.end(), generator);
    std::uniform_real_distribution<double> distribution(-5.0, 5.0);

    for (std::size_t i = 0; i < nonzero; ++i) {
        int row = static_cast<int>(positions[i] / n);
        int col = static_cast<int>(positions[i] % n);
        double value = distribution(generator);
        if (value == 0.0) {
            value = 1.0;
        }
        matrix.set_element(row, col, value);
    }
    return matrix;
}

// Читаем все вычисленные значения вне замера. CSR обходим напрямую,
// чтобы контрольная сумма не выполняла поиск для каждого нулевого элемента.
double sum_elements(const Matrix& matrix) {
    double sum = 0.0;
    const SparseMatrix* sparse = dynamic_cast<const SparseMatrix*>(&matrix);
    if (sparse != nullptr) {
        for (int row = 0; row < sparse->get_rows(); ++row) {
            for (std::size_t position = sparse->get_row_start(row);
                 position < sparse->get_row_end(row); ++position) {
                sum += sparse->get_value(position);
            }
        }
    } else {
        for (int row = 0; row < matrix.get_rows(); ++row) {
            for (int col = 0; col < matrix.get_cols(); ++col) {
                sum += matrix.get_element(row, col);
            }
        }
    }
    if (!std::isfinite(sum)) {
        throw std::runtime_error("Non-finite checksum");
    }
    return sum;
}

double measure(const Matrix& a, const Matrix& b, double& checksum) {
    auto start = std::chrono::steady_clock::now();
    std::unique_ptr<Matrix> result = a.mul(b);
    auto end = std::chrono::steady_clock::now();

    if (!result) {
        throw std::runtime_error("mul returned a null pointer");
    }
    checksum += sum_elements(*result);
    return std::chrono::duration<double>(end - start).count();
}

int main() {
#ifndef NDEBUG
    std::cerr << "Use the Release-Bench profile (Release, no sanitizers).\n";
    return 1;
#endif

    try {
        const int n = 500;
        const int repeats = 3;
        // Размер фиксирован. Добавлены промежуточные точки для поиска перехода.
        const double densities[] = {1.0, 0.5, 0.25, 0.1, 0.05, 0.01, 0.001};
        std::mt19937 generator(42);
        double checksum = 0.0;

        bool found_faster = false;
        bool have_previous = false;
        double previous_density = 0.0;
        double previous_speedup = 0.0;

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Release benchmark; size=" << n
                  << "; repeats=" << repeats << "; seed=42\n";
#ifdef _MSC_VER
        std::cout << "Compiler: MSVC " << _MSC_VER << '\n';
#else
        std::cout << "Compiler: " << __VERSION__ << '\n';
#endif
        std::cout << "DD=Dense x Dense, DS=Dense x Sparse, "
                     "SD=Sparse x Dense, SS=Sparse x Sparse\n";
        std::cout << "Density(%)\tNNZ_A\tNNZ_B\tDD(s)\tDS(s)\tSD(s)\tSS(s)\tDD/SS\n";

        for (double density : densities) {
            // Генерация и преобразование в CSR находятся вне замеров.
            DenseMatrix a = generate_matrix(n, density, generator);
            DenseMatrix b = generate_matrix(n, density, generator);
            SparseMatrix sparse_a(a);
            SparseMatrix sparse_b(b);

            const Matrix* first[] = {&a, &a, &sparse_a, &sparse_a};
            const Matrix* second[] = {&b, &sparse_b, &b, &sparse_b};

            // По одному прогревочному вызову каждого сочетания.
            for (int combination = 0; combination < 4; ++combination) {
                std::unique_ptr<Matrix> warmup =
                    first[combination]->mul(*second[combination]);
                if (!warmup) {
                    throw std::runtime_error("Warmup returned a null pointer");
                }
                checksum += sum_elements(*warmup);
            }

            double total[] = {0.0, 0.0, 0.0, 0.0};
            for (int repeat = 0; repeat < repeats; ++repeat) {
                // Порядок сдвигается, чтобы не запускать DD всегда первым.
                for (int step = 0; step < 4; ++step) {
                    int combination = (repeat + step) % 4;
                    total[combination] += measure(
                        *first[combination], *second[combination], checksum
                    );
                }
            }

            double time_dd = total[0] / repeats;
            double time_ds = total[1] / repeats;
            double time_sd = total[2] / repeats;
            double time_ss = total[3] / repeats;
            if (time_dd <= 0.0 || time_ss <= 0.0) {
                throw std::runtime_error("Timer resolution is insufficient; increase n");
            }
            double speedup = time_dd / time_ss;
            std::size_t nnz_a = sparse_a.get_row_end(n - 1);
            std::size_t nnz_b = sparse_b.get_row_end(n - 1);
            double actual_density = 100.0 * static_cast<double>(nnz_a) /
                (static_cast<double>(n) * n);

            std::cout << actual_density << '\t' << nnz_a << '\t' << nnz_b << '\t'
                      << time_dd << '\t' << time_ds << '\t' << time_sd << '\t'
                      << time_ss << '\t' << speedup << '\n';

            if (speedup > 1.0 && !found_faster) {
                std::cout << "First measured density with SS faster than DD: "
                          << actual_density << "%\n";
                if (have_previous && previous_speedup <= 1.0) {
                    std::cout << "Observed transition bracket: "
                              << actual_density << "% to "
                              << previous_density << "% (a different data pair at each density).\n";
                } else {
                    std::cout << "SS is already faster at the highest measured density; "
                                 "this run does not locate a crossover.\n";
                }
                found_faster = true;
            }
            have_previous = true;
            previous_density = actual_density;
            previous_speedup = speedup;
        }

        if (!found_faster) {
            std::cout << "SS did not outperform DD at the measured densities.\n";
        }
        std::cout << "Checksum: " << checksum << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Benchmark failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
