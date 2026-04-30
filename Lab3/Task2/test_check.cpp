#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>

bool approx_eq(double a, double b, double eps=1e-9) {
    return std::abs(a-b) <= eps * std::max(1.0, std::max(std::abs(a), std::abs(b)));
}

int main() {
    bool ok = true;
    for (int client=1; client<=3; ++client) {
        std::string fname = "client_" + std::to_string(client) + ".txt";
        std::ifstream in(fname);
        if (!in) {
            std::cerr << "Missing file " << fname << '\n'; ok = false; continue;
        }
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            size_t id; std::string tag;
            if (!(iss >> id >> tag)) { ok = false; break; }

            if (tag == "sin") {
                double x, arrow, res; char c;
                if (!(iss >> x)) { ok = false; break; }
                if (!(iss >> c)) { ok = false; break; }
                if (c != '-') { iss.clear(); iss.seekg(-1, std::ios::cur); }
                
                std::string tail;
                std::getline(iss, tail);
                double val = std::stod(tail.substr(tail.find_last_of(' ')));
                double expected = std::sin(x);
                if (!approx_eq(expected, val)) { ok = false; break; }

            } else if (tag == "sqrt") {
                double x; std::string arrow; double val;
                if (!(iss >> x)) { ok = false; break; }

                std::string tail; std::getline(iss, tail);
                double got = std::stod(tail.substr(tail.find_last_of(' ')));
                double expected = std::sqrt(x);
                if (!approx_eq(expected, got)) { ok = false; break; }

            } else if (tag == "pow") {
                double a; char car; double b; std::string tail;
                if (!(iss >> a)) { ok = false; break; }
                if (!(iss >> car)) { ok = false; break; }
                if (!(iss >> b)) { ok = false; break; }

                std::getline(iss, tail);
                double got = std::stod(tail.substr(tail.find_last_of(' ')));
                double expected = std::pow(a,b);
                if (!approx_eq(expected, got)) { ok = false; break; }
            }
        }
        in.close();
        std::cout << "Checked " << fname << ": " << (ok?"OK":"FAIL") << "\n";
    }
    return ok ? 0 : 2;
}