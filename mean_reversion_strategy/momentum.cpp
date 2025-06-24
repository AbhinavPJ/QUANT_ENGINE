#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <cmath>
#include <random>
#include <ctime>
#include <algorithm>

using namespace std;

// Helper to split a CSV line
vector<string> split(const string &line, char delim = ',') {
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while (getline(ss, token, delim)) tokens.push_back(token);
    return tokens;
}

bool is_number(const string& s) {
    try {
        stod(s);
        return true;
    } catch (...) {
        return false;
    }
}

int main() {
    ifstream returns_file("../stock_data/returns.csv");
    if (!returns_file.is_open()) {
        cerr << "Error: could not open returns.csv" << endl;
        return 1;
    }

    string line;
    getline(returns_file, line);
    vector<string> headers = split(line);
    vector<string> stocks(headers.begin() + 2, headers.end());

    double wealth = 100000.0;
    vector<double> wealth_history;
    wealth_history.push_back(wealth);
    vector<unordered_map<string, double>> allocation_log;
    mt19937 rng(static_cast<unsigned int>(time(0)));

    vector<vector<string>> lines;
    while (getline(returns_file, line)) {
        vector<string> row = split(line);
        lines.push_back(row);
    }

       for (int i = 3; i < lines.size(); i++) {
        vector<pair<double, string>> returns;

        for (int j = 2; j < lines[i].size(); j++) {
            double cumulative_return = 0.0;
            int count = 0;
            int cur = i - 1;

            while (cur >= 0 && count < 3) {
                if (j < lines[cur].size() && is_number(lines[cur][j])) {
                    cumulative_return += stod(lines[cur][j]);
                    count++;
                }
                cur--;
            }

            if (count == 3) {
                returns.push_back({ cumulative_return, headers[j] });
            }
        }
        //cout<<returns.size()<<endl;
        if (returns.size() < 10) {
            wealth_history.push_back(wealth);
            allocation_log.push_back({});
            continue;
        }

        // Sort descending for long and ascending for short
        sort(returns.begin(), returns.end(), greater<>());

        int top_k = max(1, static_cast<int>(returns.size() * 0.3));
        int bottom_k = top_k;

        unordered_map<string, double> allocation_row;
        double daily_growth = 0.0;

        // Long: top 30%
        double long_alloc = 0.5 / top_k;
        for (int k = 0; k < bottom_k; k++) {
            string stock = returns[returns.size() - 1 - k].second;
            int col_index = find(headers.begin(), headers.end(), stock) - headers.begin();
            if (col_index < headers.size() && is_number(lines[i][col_index])) {
                double today_return = stod(lines[i][col_index]);
                allocation_row[stock] = long_alloc;
                daily_growth += long_alloc * (today_return - 1.0);
            }
        }

       double short_alloc = -0.5 / bottom_k;
        for (int k = 0; k < top_k; k++) {
            string stock = returns[k].second;
            int col_index = find(headers.begin(), headers.end(), stock) - headers.begin();
            if (col_index < headers.size() && is_number(lines[i][col_index])) {
                double today_return = stod(lines[i][col_index]);

                if (today_return > 0.0) {  // protect against division by zero
                    double short_pnl = short_alloc * (1.0 - 1.0 / today_return);
                    daily_growth += short_pnl;
                    allocation_row[stock] = short_alloc;
                }
            }
        }


        wealth *= (1.0 + daily_growth);
        wealth_history.push_back(wealth);
        allocation_log.push_back(allocation_row);
    }

    returns_file.close();

    ofstream pnl("pnl.csv");
    pnl << "day,wealth\n";
    for (size_t i = 0; i < wealth_history.size(); ++i) {
        pnl << i << "," << wealth_history[i] << "\n";
    }
    pnl.close();

    ofstream alloc_out("allocation.csv");
    alloc_out << "day";
    for (const string& sym : stocks) {
        alloc_out << "," << sym;
    }
    alloc_out << "\n";

    for (size_t day = 0; day < allocation_log.size(); ++day) {
        alloc_out << day;
        for (const string& sym : stocks) {
            double val = allocation_log[day].count(sym) ? allocation_log[day][sym] : 0.0;
            alloc_out << "," << val;
        }
        alloc_out << "\n";
    }
    alloc_out.close();

    cout << "✅ Simulation complete. Final wealth: " << wealth << endl;
    return 0;
}
