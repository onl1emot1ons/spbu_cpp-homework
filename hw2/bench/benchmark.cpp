//
// Created by konst on 27.10.2026.
//

#include "../DenseMatrix.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>

int main() {
    const int sizes[] = {100, 200, 500, 1000};
    const int repeats = 3;

    std::mt19937 generator(42);
    std::uniform_real_distribution<double> distribution(-5.0, 5.0);

    double checksum = 0.0;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Size\tNaive (s)\tOptimized (s)\tSpeedup\n";

    for (int n : sizes) {
        DenseMatrix a(n, n);
        DenseMatrix b(n, n);

        // Создание исходных данных не входит в замеры.
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                a.set_element(i, j, distribution(generator));
                b.set_element(i, j, distribution(generator));
            }
        }

        double total_naive = 0.0;
        double total_optimized = 0.0;

        for (int repeat = 0; repeat < repeats; ++repeat) {
            auto start_naive = std::chrono::steady_clock::now();

            DenseMatrix naive = a.multiply_naive(b);

            auto end_naive = std::chrono::steady_clock::now();

            total_naive += std::chrono::duration<double>(
                end_naive - start_naive
            ).count();

            auto start_optimized = std::chrono::steady_clock::now();

            DenseMatrix optimized = a.multiply_optimized(b);

            auto end_optimized = std::chrono::steady_clock::now();

            total_optimized += std::chrono::duration<double>(
                end_optimized - start_optimized
            ).count();

            // Используем результаты вне измеряемых участков.
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    checksum += naive.get_element(i, j);
                    checksum += optimized.get_element(i, j);
                }
            }
        }

        double time_naive = total_naive / repeats;
        double time_optimized = total_optimized / repeats;
        double speedup = time_naive / time_optimized;

        std::cout << n << '\t'
                  << time_naive << '\t'
                  << time_optimized << '\t'
                  << speedup << '\n';
    }

    std::cout << "Checksum: " << checksum << '\n';
}