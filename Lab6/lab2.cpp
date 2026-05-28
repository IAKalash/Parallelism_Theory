#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <chrono>

#include <boost/program_options.hpp>

namespace po = boost::program_options;
constexpr double TAU = -0.01;

struct Config {
    int size = 30;
    double eps = 1e-6;
    int max_iter = 1000000;
    std::string out_file = "result.dat";
};

Config parse_args(int argc, char** argv) {
    Config config;
    po::options_description desc("Heat equation solver options");
    desc.add_options()
        ("help,h", "Show help message")
        ("size,s", po::value<int>(&config.size)->default_value(30), "Grid size (NxN)")
        ("accuracy,a", po::value<double>(&config.eps)->default_value(1e-6), "Convergence tolerance")
        ("max-iter,i", po::value<int>(&config.max_iter)->default_value(1000000), "Maximum iterations")
        ("output,o", po::value<std::string>(&config.out_file)->default_value("result.dat"), "Output file");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << desc << "\n";
        std::exit(0);
    }
    
    return config;
}

using Grid = std::vector<double>;

int idx(int y, int x, int size) {
    return y * size + x;
}

Grid make_grid(int size) {
    return Grid(size * size, 0.0);
}

void init_matrix(double* grid, int size) {
    if (size < 2) {
        return;
    }

    const double c00 = 10.0;
    const double c10 = 20.0;
    const double c11 = 30.0;
    const double c01 = 20.0;

    for (int x = 0; x < size; ++x) {
        const double t = static_cast<double>(x) / (size - 1);
        grid[idx(0, x, size)] = c00 + t * (c10 - c00);
        grid[idx(size - 1, x, size)] = c01 + t * (c11 - c01);
    }

    for (int y = 0; y < size; ++y) {
        const double t = static_cast<double>(y) / (size - 1);
        grid[idx(y, 0, size)] = c00 + t * (c01 - c00);
        grid[idx(y, size - 1, size)] = c10 + t * (c11 - c10);
    }
}

double iterate_step_and_maxdelta(const double* current, double* next, int size) {
    double error = 0.0;
    const int total = size * size;

    #pragma acc parallel loop collapse(2) present(current[0:total], next[0:total]) reduction(max:error)
    for (int y = 1; y < size - 1; ++y) {
        for (int x = 1; x < size - 1; ++x) {
            const int position = idx(y, x, size);
            const double laplace = -4.0 * current[position]
                                 + current[idx(y - 1, x, size)]
                                 + current[idx(y + 1, x, size)]
                                 + current[idx(y, x - 1, size)]
                                 + current[idx(y, x + 1, size)];
            const double val = current[position] - TAU * laplace;
            next[position] = val;
            error = std::max(error, std::abs(val - current[position]));
        }
    }
    return error;
}

void write_result(const Grid& grid, const char* file_name) {
    std::ofstream out(file_name, std::ios::binary);
    out.write(reinterpret_cast<const char*>(grid.data()), static_cast<std::streamsize>(grid.size() * sizeof(double)));
}

void print_grid(const Grid& grid) {
    const int size = static_cast<int>(std::sqrt(static_cast<double>(grid.size())));
    std::cout << std::fixed << std::setprecision(2);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            std::cout << std::setw(7) << grid[idx(y, x, size)] << ' ';
        }
        std::cout << '\n';
    }
}

int main(int argc, char** argv) {
    Config config = parse_args(argc, argv);

    Grid current = make_grid(config.size);
    Grid next = make_grid(config.size);
    init_matrix(current.data(), config.size);
    init_matrix(next.data(), config.size);

    int iterations = 0;
    double error = 0.0;

    double* current_base = current.data();
    double* next_base = next.data();
    double* current_ptr = current_base;
    double* next_ptr = next_base;
    const int total = config.size * config.size;

    const auto t_start = std::chrono::steady_clock::now();
    #pragma acc data copy(current_base[0:total], next_base[0:total])
    {
        do {
            error = iterate_step_and_maxdelta(current_ptr, next_ptr, config.size);

            std::swap(current_ptr, next_ptr);

            ++iterations;
        } while (error >= config.eps && iterations < config.max_iter);
    }

    const auto t_end = std::chrono::steady_clock::now();
    const double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(t_end - t_start).count();
    std::cout << "Elapsed time (s): " << elapsed_seconds << "\n";

    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Final error: " << error << "\n";

    write_result(current, config.out_file.c_str());
    std::cout << "Result saved to " << config.out_file << "\n";

    if (config.size <= 15) {
        print_grid(current);
    }
    return 0;
}
