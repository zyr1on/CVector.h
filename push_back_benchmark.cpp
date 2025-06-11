#include <vector>
#include "CVector.h"
#include <chrono>
#include <iostream>
#include <numeric>

int main(int argc, char const *argv[])
{
    constexpr int N = 1000000;
    constexpr int RUNS = 5;

    double cvec_times[RUNS] = {0};
    double stlvec_times[RUNS] = {0};

    for (int run = 0; run < RUNS; ++run) {
        // CVector benchmark
        vector(int) vec;
        vector_init(vec);

        auto start_cvec = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < N; i++)
            vector_push_back(vec, i);
        auto end_cvec = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_cvec = end_cvec - start_cvec;

        cvec_times[run] = duration_cvec.count();
        vector_destroy(vec);

        // STL vector benchmark
        std::vector<int> stlvec;
        auto start_stl = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < N; i++)
            stlvec.push_back(i);
        auto end_stl = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_stl = end_stl - start_stl;

        stlvec_times[run] = duration_stl.count();

        std::cout << "Run " << run+1 << ": "
                  << "CVector: " << cvec_times[run] << " ms, "
                  << "std::vector: " << stlvec_times[run] << " ms"
                  << std::endl;
    }

    double cvec_avg = std::accumulate(cvec_times, cvec_times + RUNS, 0.0) / RUNS;
    double stlvec_avg = std::accumulate(stlvec_times, stlvec_times + RUNS, 0.0) / RUNS;

    std::cout << "-----------------------------" << std::endl;
    std::cout << "CVector push_back average: " << cvec_avg << " ms" << std::endl;
    std::cout << "std::vector push_back average: " << stlvec_avg << " ms" << std::endl;

    return 0;
}
