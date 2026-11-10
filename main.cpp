
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>

const int N = 800;

void generateMatrix(double arr[N][N]) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            arr[i][j] = rand() % 11;
        }
    }
}

void IKJ(double A[N][N], double B[N][N], double C[N][N]) {
    for (int i = 0; i < N; ++i)
        for (int k = 0; k < N; ++k)
            for (int j = 0; j < N; ++j)
                A[i][j] = A[i][j] + B[i][k] * C[k][j];
}

int main() {
    srand(time(nullptr));

    double A[N][N] = {0}, B[N][N] = {0}, C[N][N] = {0};
    double total_time = 0;

    for (int i = 0; i < 5; ++i) {
        generateMatrix(B);
        generateMatrix(C);

        for (int a = 0; a < N; ++a)
            for (int b = 0; b < N; ++b)
                A[a][b] = 0.0;

        auto start_time = std::chrono::high_resolution_clock::now();

        IKJ(A, B, C);

        auto end_time = std::chrono::high_resolution_clock::now();

        total_time += std::chrono::duration<double>(
            end_time - start_time
        ).count();
    }

    std::cout << "IKJ," << N << "," << total_time / 5.0 << std::endl;

    return 0;
}
