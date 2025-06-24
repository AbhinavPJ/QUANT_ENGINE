#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cmath>
#include <random>
#include <ctime>

// Helper to split a CSV line
std::vector<std::string> split(const std::string &line, char delim = ',') {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (getline(ss, token, delim)) tokens.push_back(token);
    return tokens;
}

int main() {
    std::ifstream returns_file("../stock_data/returns.csv");
    if (!returns_file.is_open()) {
        std::cerr << "Error: could not open returns.csv" << std::endl;
        return 1;
    } 

    std::string line;
    getline(returns_file, line); // header
    std::vector<std::string> headers = split(line);
    std::vector<std::string> stocks(headers.begin() + 2, headers.end());

    double wealth = 100000.0;
    std::vector<double> wealth_history;
    wealth_history.push_back(wealth);

    // For allocation logging
    std::vector<std::unordered_map<std::string, double>> allocation_log;

    // RNG for random long/short assignment
    std::mt19937 rng(static_cast<unsigned int>(time(0)));
    std::uniform_int_distribution<int> sign_dist(0, 1);

    while (getline(returns_file, line)) {
        std::vector<std::string> row = split(line);
        std::string date = row[1];

        std::unordered_map<std::string, double> daily_returns;
        for (size_t i = 2; i < row.size(); ++i) {
            const std::string& val = row[i];
            if (!val.empty() && val != "nan" && val != "NaN") {
                daily_returns[stocks[i - 2]] = std::stod(val);
            }
        }

        int available = daily_returns.size();
        if (available == 0) {
            wealth_history.push_back(wealth);
            allocation_log.push_back({});
            continue;
        }

        // Randomly assign long/short
        std::vector<std::string> longs, shorts;
        for (const auto& [symbol, _] : daily_returns) {
            if (sign_dist(rng)) longs.push_back(symbol);
            else shorts.push_back(symbol);
        }

        // Ensure both sides have at least 1 stock
        if (longs.empty() || shorts.empty()) {
            wealth_history.push_back(wealth);
            allocation_log.push_back({});
            continue;
        }

        double long_alloc = 0.5 / longs.size();
        double short_alloc = -0.5 / shorts.size();
        double day_growth = 0.0;
        std::unordered_map<std::string, double> allocation_row;

        for (const std::string& sym : longs) {
            double alloc = long_alloc;
            day_growth += alloc * (daily_returns[sym] - 1.0);
            allocation_row[sym] = alloc;
        }

        for (const std::string& sym : shorts) {
            double alloc = short_alloc;
            day_growth += alloc * (daily_returns[sym] - 1.0);
            allocation_row[sym] = alloc;
        }

        wealth *= (1.0 + day_growth);
        wealth_history.push_back(wealth);
        allocation_log.push_back(allocation_row);
    }

    returns_file.close();

    // Save PnL to CSV
    std::ofstream pnl("pnl.csv");
    pnl << "day,wealth\n";
    for (size_t i = 0; i < wealth_history.size(); ++i) {
        pnl << i << "," << wealth_history[i] << "\n";
    }
    pnl.close();

    // Save Allocation to CSV
    std::ofstream alloc_out("allocation.csv");
    alloc_out << "day";
    for (const std::string& sym : stocks) {
        alloc_out << "," << sym;
    }
    alloc_out << "\n";

    for (size_t day = 0; day < allocation_log.size(); ++day) {
        alloc_out << day;
        for (const std::string& sym : stocks) {
            double val = allocation_log[day].count(sym) ? allocation_log[day][sym] : 0.0;
            alloc_out << "," << val;
        }
        alloc_out << "\n";
    }
    alloc_out.close();

    std::cout << "✅ Simulation complete. Final wealth: " << wealth << std::endl;
    return 0;
}
