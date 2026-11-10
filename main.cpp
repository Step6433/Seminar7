
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>
#include <functional>

const int N = 800;

void generateMatrix(double arr[N][N]) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            arr[i][j] = rand() % 11;
        }
    }
}

void IJK(double A[N][N], double B[N][N], double C[N][N],
         int start_row, int end_row,
         std::atomic<bool>& start_flag,
         std::atomic<unsigned int>& ready_count) {

    ready_count.fetch_add(1);

    while (!start_flag.load()) {
        std::this_thread::yield();
    }

    // Порядок циклов i, j, k
    for (int i = start_row; i < end_row; ++i)
        for (int j = 0; j < N; ++j)
            for (int k = 0; k < N; ++k)
                A[i][j] = A[i][j] + B[i][k] * C[k][j];
}

int main() {
    srand(time(nullptr));

    static double A[N][N] = {0};
    static double B[N][N] = {0};
    static double C[N][N] = {0};

    generateMatrix(B);
    generateMatrix(C);

    unsigned int num_threads = std::thread::hardware_concurrency();

    if (num_threads == 0)
        num_threads = 1;

    num_threads = std::min(num_threads, static_cast<unsigned int>(N));

    std::atomic<bool> start_flag(false);
    std::atomic<unsigned int> ready_count(0);

    std::vector<std::thread> threads;

    for (unsigned int t = 0; t < num_threads; ++t) {
        int start_row = t * N / num_threads;
        int end_row = (t + 1) * N / num_threads;

        threads.emplace_back(
            IJK, A, B, C, start_row, end_row,
            std::ref(start_flag), std::ref(ready_count)
        );
    }

    while (ready_count.load() < num_threads) {
        std::this_thread::yield();
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    start_flag.store(true);

    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    double total_time = std::chrono::duration<double>(
        end_time - start_time
    ).count();

    std::cout << "Threads: " << num_threads << std::endl;
    std::cout << "IJK," << N << "," << total_time << std::endl;

    return 0;
}
