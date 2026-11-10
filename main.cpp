
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>

const int N = 550;
const int REPEATS = 5;

void generateMatrix(double arr[N][N]) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            arr[i][j] = rand() % 11;
        }
    }
}

void IKJ(double A[N][N], double B[N][N], double C[N][N],
         int start_row, int end_row) {
    for (int i = start_row; i < end_row; ++i)
        for (int k = 0; k < N; ++k)
            for (int j = 0; j < N; ++j)
                A[i][j] = A[i][j] + B[i][k] * C[k][j];
}

int main() {
    srand(time(nullptr));

    static double A[N][N] = {0};
    static double B[N][N] = {0};
    static double C[N][N] = {0};

    unsigned int num_threads = std::thread::hardware_concurrency();

    if (num_threads == 0)
        num_threads = 1;

    num_threads = std::min(num_threads, static_cast<unsigned int>(N));

    std::mutex mtx;
    std::condition_variable cv_start;
    std::condition_variable cv_done;

    int generation = 0;
    unsigned int finished = 0;
    bool stop = false;

    std::vector<std::thread> threads;

    // Создаем потоки только один раз
    for (unsigned int t = 0; t < num_threads; ++t) {
        int start_row = t * N / num_threads;
        int end_row = (t + 1) * N / num_threads;

        threads.emplace_back([&, start_row, end_row]() {
            int last_generation = 0;

            while (true) {
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv_start.wait(lock, [&]() {
                        return generation > last_generation || stop;
                    });

                    if (stop)
                        return;

                    last_generation = generation;
                }

                IKJ(A, B, C, start_row, end_row);

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    ++finished;

                    if (finished == num_threads)
                        cv_done.notify_one();
                }
            }
        });
    }

    double total_time = 0;

    for (int i = 0; i < REPEATS; ++i) {
        generateMatrix(B);
        generateMatrix(C);

        for (int a = 0; a < N; ++a)
            for (int b = 0; b < N; ++b)
                A[a][b] = 0.0;

        auto start_time = std::chrono::high_resolution_clock::now();

        {
            std::lock_guard<std::mutex> lock(mtx);
            finished = 0;
            ++generation;
        }

        cv_start.notify_all();

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv_done.wait(lock, [&]() {
                return finished == num_threads;
            });
        }

        auto end_time = std::chrono::high_resolution_clock::now();

        total_time += std::chrono::duration<double>(
            end_time - start_time
        ).count();
    }

    // Завершаем рабочие потоки
    {
        std::lock_guard<std::mutex> lock(mtx);
        stop = true;
    }

    cv_start.notify_all();

    for (auto& thread : threads)
        thread.join();

    std::cout << "Threads: " << num_threads << std::endl;
    std::cout << "IKJ," << N << "," << total_time / REPEATS << std::endl;

    return 0;
}
