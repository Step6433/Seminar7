
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <cmath>

const int NUM_THREADS = 4;

// Генерация матрицы
void generateMatrix(std::vector<double>& M, int n) {
    for (int i = 0; i < n * n; ++i) {
        M[i] = rand() % 11;
    }
}

// Умножение матриц в порядке i, k, j
void IKJ(std::vector<double>& A,
         const std::vector<double>& B,
         const std::vector<double>& C,
         int n, int start_row, int end_row) {

    for (int i = start_row; i < end_row; ++i) {
        for (int k = 0; k < n; ++k) {
            for (int j = 0; j < n; ++j) {
                A[i * n + j] += B[i * n + k] * C[k * n + j];
            }
        }
    }
}

// Однопоточное умножение
double singleThread(std::vector<double>& A,
                    const std::vector<double>& B,
                    const std::vector<double>& C,
                    int n) {

    auto start = std::chrono::high_resolution_clock::now();

    IKJ(A, B, C, n, 0, n);

    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::milli>(
        end - start
    ).count();
}

// Многопоточное умножение с атомарным флагом
double multiThread(std::vector<double>& A,
                   const std::vector<double>& B,
                   const std::vector<double>& C,
                   int n) {

    std::atomic<bool> start_flag(false);
    std::atomic<int> ready_count(0);

    std::vector<std::thread> threads;

    // Создаем 4 потока до запуска таймера
    for (int t = 0; t < NUM_THREADS; ++t) {

        int start_row = t * n / NUM_THREADS;
        int end_row = (t + 1) * n / NUM_THREADS;

        threads.emplace_back([&, start_row, end_row]() {

            ready_count.fetch_add(1);

            // Ожидание сигнала запуска
            while (!start_flag.load()) {
                std::this_thread::yield();
            }

            IKJ(A, B, C, n, start_row, end_row);
        });
    }

    // Ждем готовности всех потоков
    while (ready_count.load() < NUM_THREADS) {
        std::this_thread::yield();
    }

    // Запускаем таймер
    auto start = std::chrono::high_resolution_clock::now();

    // Разрешаем потокам начать вычисления
    start_flag.store(true);

    // Ожидаем завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    // Останавливаем таймер
    auto end = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double, std::milli>(
        end - start
    ).count();
}

int main() {
    srand(time(nullptr));

    int sizes[] = {300, 500, 800, 1500};

    std::cout << std::fixed << std::setprecision(3);

    std::cout << "n\t1 поток (мс)\t4 потока (мс)\tУскорение\n";

    for (int n : sizes) {

        // Создаем матрицы
        std::vector<double> A1(n * n, 0.0);
        std::vector<double> A4(n * n, 0.0);
        std::vector<double> B(n * n);
        std::vector<double> C(n * n);

        generateMatrix(B, n);
        generateMatrix(C, n);

        // Однопоточное вычисление
        double T1 = singleThread(A1, B, C, n);

        // Многопоточное вычисление
        double T4 = multiThread(A4, B, C, n);

        // Проверяем совпадение результатов
        for (int i = 0; i < n * n; ++i) {
            if (std::abs(A1[i] - A4[i]) > 1e-9) {
                std::cerr << "Ошибка вычисления!" << std::endl;
                return 1;
            }
        }

        // Вычисляем ускорение
        double speedup = T1 / T4;

        // Выводим строку таблицы
        std::cout << n << "\t"
                  << T1 << "\t\t"
                  << T4 << "\t\t"
                  << speedup << std::endl;
    }

    return 0;
}
