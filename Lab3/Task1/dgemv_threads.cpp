#include <pthread.h>
#include <sched.h>
#include <time.h>

#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

double cpuSecond()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void run_parallel(size_t m,
                  size_t n,
                  int nthreads,
                  int pin_threads,
                  double* init_time,
                  double* compute_time,
                  double* checksum)
{
    double* a = (double*)malloc(sizeof(*a) * m * n);
    double* b = (double*)malloc(sizeof(*b) * n);
    double* c = (double*)malloc(sizeof(*c) * m);

    if (a == nullptr || b == nullptr || c == nullptr)
    {
        free(a);
        free(b);
        free(c);
        std::printf("Error allocate memory!\n");
        std::exit(1);
    }

    double t = cpuSecond();

    {
        std::vector<std::thread> workers;
        workers.reserve((size_t)nthreads);

        for (int tid = 0; tid < nthreads; ++tid)
        {
            workers.emplace_back([&, tid]() {
                if (pin_threads)
                {
                    cpu_set_t cpuset;
                    CPU_ZERO(&cpuset);

                    unsigned int hw = std::thread::hardware_concurrency();
                    if (hw == 0)
                        hw = 1;

                    CPU_SET(tid % (int)hw, &cpuset);
                    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
                }

                size_t lb = m * (size_t)tid / (size_t)nthreads;
                size_t ub = m * (size_t)(tid + 1) / (size_t)nthreads;

                for (size_t i = lb; i < ub; ++i)
                {
                    for (size_t j = 0; j < n; ++j)
                        a[i * n + j] = (double)(i + j);
                    c[i] = 0.0;
                }
            });
        }

        for (auto& t_worker : workers)
            t_worker.join();
    }

    {
        std::vector<std::thread> workers;
        workers.reserve((size_t)nthreads);
        for (int tid = 0; tid < nthreads; ++tid)
        {
            workers.emplace_back([&, tid]() {
                if (pin_threads)
                {
                    cpu_set_t cpuset;
                    CPU_ZERO(&cpuset);

                    unsigned int hw = std::thread::hardware_concurrency();
                    if (hw == 0)
                        hw = 1;

                    CPU_SET(tid % (int)hw, &cpuset);
                    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
                }

                size_t lb = n * (size_t)tid / (size_t)nthreads;
                size_t ub = n * (size_t)(tid + 1) / (size_t)nthreads;
                for (size_t j = lb; j < ub; ++j)
                    b[j] = (double)j;
            });
        }
        for (auto& t_worker : workers)
            t_worker.join();
    }

    *init_time = cpuSecond() - t;

    t = cpuSecond();

    {
        std::vector<std::thread> workers;
        workers.reserve((size_t)nthreads);

        for (int tid = 0; tid < nthreads; ++tid)
        {
            workers.emplace_back([&, tid]() {
                if (pin_threads)
                {
                    cpu_set_t cpuset;
                    CPU_ZERO(&cpuset);

                    unsigned int hw = std::thread::hardware_concurrency();
                    if (hw == 0)
                        hw = 1;

                    CPU_SET(tid % (int)hw, &cpuset);
                    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
                }

                size_t lb = m * (size_t)tid / (size_t)nthreads;
                size_t ub = m * (size_t)(tid + 1) / (size_t)nthreads;

                for (size_t i = lb; i < ub; ++i)
                {
                    double sum = 0.0;
                    for (size_t j = 0; j < n; ++j)
                        sum += a[i * n + j] * b[j];

                    c[i] = sum;
                }
            });
        }

        for (auto& t_worker : workers)
            t_worker.join();
    }

    *compute_time = cpuSecond() - t;

    *checksum = 0.0;
    for (size_t i = 0; i < m; ++i)
        *checksum += c[i];

    free(a);
    free(b);
    free(c);
}

int main(int argc, char* argv[])
{
    size_t M = 20000;
    size_t N = 20000;
    int nthreads = 1;
    int pin_threads = 0;

    if (argc > 1)
        M = (size_t)std::strtoull(argv[1], nullptr, 10);
    if (argc > 2)
        N = (size_t)std::strtoull(argv[2], nullptr, 10);
    if (argc > 3)
        nthreads = std::atoi(argv[3]);
    if (argc > 4)
        pin_threads = std::atoi(argv[4]);

    if (nthreads < 1)
        nthreads = 1;

    std::printf("Matrix size: %zu x %zu\n", M, N);

    double init_time = 0.0;
    double compute_time = 0.0;
    double checksum = 0.0;
    run_parallel(M, N, nthreads, pin_threads, &init_time, &compute_time, &checksum);

    std::printf("Threads: %d\n", nthreads);
    std::printf("Elapsed time (init): %.6f sec.\n", init_time);
    std::printf("Elapsed time (parallel compute): %.6f sec.\n", compute_time);
    std::printf("Checksum: %.6f\n", checksum);

    return 0;
}
