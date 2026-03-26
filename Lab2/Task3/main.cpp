#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdlib>

#include <omp.h>

const double EPSILON = 1e-5;

double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void init(std::vector<double>& A, std::vector<double>& b, std::vector<double>& x, int N) {
    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = N / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (N - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; ++i) {
            for (int j = 0; j < N; ++j) {
                A[i * N + j] = (i == j) ? 2.0 : 1.0;
            }
            b[i] = N + 1.0;
            x[i] = 0.0;
        }
    }
}

void solve_v1(const std::vector<double>& A, const std::vector<double>& b, std::vector<double>& x, int N, double tau) {
    std::vector<double> x_new(N);
    double norm_b = 0.0;
    
    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = N / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (N - 1) : (lb + items_per_thread - 1);

        double local_norm_b = 0.0;
        for (int i = lb; i <= ub; ++i) {
            local_norm_b += b[i] * b[i];
        }
        #pragma omp atomic
        norm_b += local_norm_b;
    }
    norm_b = std::sqrt(norm_b);

    bool converged = false;
    double norm_res;

    while (!converged) {
        norm_res = 0.0;

        #pragma omp parallel
        {
            int nthreads = omp_get_num_threads();
            int threadid = omp_get_thread_num();
            int items_per_thread = N / nthreads;
            int lb = threadid * items_per_thread;
            int ub = (threadid == nthreads - 1) ? (N - 1) : (lb + items_per_thread - 1);

            double local_norm_res = 0.0;
            for (int i = lb; i <= ub; ++i) {
                double ax = 0.0;
                for (int j = 0; j < N; ++j) {
                    ax += A[i * N + j] * x[j];
                }
                double res = ax - b[i];
                local_norm_res += res * res;
                x_new[i] = x[i] - tau * res;
            }
            
            #pragma omp atomic
            norm_res += local_norm_res;
        }

        if (std::sqrt(norm_res) / norm_b < EPSILON) {
            converged = true;
        }

        #pragma omp parallel
        {
            int nthreads = omp_get_num_threads();
            int threadid = omp_get_thread_num();
            int items_per_thread = N / nthreads;
            int lb = threadid * items_per_thread;
            int ub = (threadid == nthreads - 1) ? (N - 1) : (lb + items_per_thread - 1);

            for (int i = lb; i <= ub; ++i) {
                x[i] = x_new[i];
            }
        }
    }
}

void solve_v2(const std::vector<double>& A, const std::vector<double>& b, std::vector<double>& x, int N, double tau) {
    std::vector<double> x_new(N);
    double norm_b = 0.0;
    bool converged = false;
    double norm_res = 0.0;
    
    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = N / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (N - 1) : (lb + items_per_thread - 1);

        double local_norm_b = 0.0;
        for (int i = lb; i <= ub; ++i) {
            local_norm_b += b[i] * b[i];
        }
        
        #pragma omp atomic
        norm_b += local_norm_b;
        
        #pragma omp barrier

        #pragma omp single
        {
            norm_b = std::sqrt(norm_b);
        }

        while (!converged) {
            double local_norm_res = 0.0;

            for (int i = lb; i <= ub; ++i) {
                double ax = 0.0;
                for (int j = 0; j < N; ++j) {
                    ax += A[i * N + j] * x[j];
                }
                double res = ax - b[i];
                local_norm_res += res * res;
                x_new[i] = x[i] - tau * res;
            }

            #pragma omp atomic
            norm_res += local_norm_res;

            #pragma omp barrier

            #pragma omp single
            {
                if (std::sqrt(norm_res) / norm_b < EPSILON) {
                    converged = true;
                }
                norm_res = 0.0;
            }

            for (int i = lb; i <= ub; ++i) {
                x[i] = x_new[i];
            }
            
            #pragma omp barrier 
        }
    }
}

int main(int argc, char** argv) {
    int N = 2000;
    if (argc > 1) {
        N = std::atoi(argv[1]);
    }
    
    int num_threads = omp_get_max_threads();

    std::vector<double> A(N * N);
    std::vector<double> b(N);
    std::vector<double> x(N);

    double tau = 2.0 / (N + 2.0); 

    init(A, b, x, N);
    double t1 = cpuSecond();
    solve_v1(A, b, x, N, tau);
    t1 = cpuSecond() - t1;

    init(A, b, x, N);
    double t2 = cpuSecond();
    solve_v2(A, b, x, N, tau);
    t2 = cpuSecond() - t2;
    
    std::cout << num_threads << "," << t1 << "," << t2 << "\n";

    return 0;
}
