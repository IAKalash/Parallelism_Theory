#include "server.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

template<typename T>
T fun_sin(T arg) { 
    return std::sin(arg); 
}

template<typename T>
T fun_sqrt(T arg) { 
    return std::sqrt(arg); 
}

template<typename T>
T fun_pow(T x, T y) { 
    return std::pow(x, y); 
}

int main(int argc, char* argv[]) {
    size_t N = 200;
    if (argc > 1) N = std::stoul(argv[1]);

    Server<double> server;
    server.start();

    auto client_worker = [&](int client_id) {
        std::mt19937_64 rng((unsigned)std::hash<std::thread::id>()(std::this_thread::get_id()) ^ (unsigned)time(nullptr));
        std::uniform_real_distribution<double> dist_sin(-100.0, 100.0);
        std::uniform_real_distribution<double> dist_sqrt(0.0, 1e6);
        std::uniform_real_distribution<double> dist_base(0.0, 10.0);
        std::uniform_int_distribution<int> dist_exp(0, 8);

        std::string fname = "client_" + std::to_string(client_id) + ".txt";
        std::ofstream out(fname, std::ios::out);
        out << std::setprecision(12);

        for (size_t i = 0; i < N; ++i) {
            if (client_id == 1) {
                double x = dist_sin(rng);
                auto task = [x]() { return fun_sin<double>(x); };
                size_t id = server.add_task(task);
                double res = server.request_result(id);
                out << id << " sin " << x << " -> " << res << '\n';
            } else if (client_id == 2) {
                double x = dist_sqrt(rng);
                auto task = [x]() { return fun_sqrt<double>(x); };
                size_t id = server.add_task(task);
                double res = server.request_result(id);
                out << id << " sqrt " << x << " -> " << res << '\n';
            } else {
                double a = dist_base(rng);
                double b = (double)dist_exp(rng);
                auto task = [a,b]() { return fun_pow<double>(a,b); };
                size_t id = server.add_task(task);
                double res = server.request_result(id);
                out << id << " pow " << a << " ^ " << b << " -> " << res << '\n';
            }
        }

        out.close();
        std::cout << "Client " << client_id << " done, wrote " << fname << "\n";
    };

    std::vector<std::thread> clients;
    clients.emplace_back(client_worker, 1);
    clients.emplace_back(client_worker, 2);
    clients.emplace_back(client_worker, 3);

    for (auto &t : clients) t.join();

    server.stop();
    std::cout << "Server stopped." << std::endl;
    return 0;
}