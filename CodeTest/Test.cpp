#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

// 定义每个 symbol 的统计信息
struct TradeStats {
    long long lastTimestamp;       // 最近一笔交易的时间戳
    long long maxTimeGap;          // 最大时间间隔
    long long totalVolume;         // 总交易量（Quantity 求和）
    long long totalPriceQuantity;  // 累计价格×交易量，用于计算加权平均价格
    long long maxPrice;            // 最大交易价格
};

int main(int argc, char* argv[]) {
    // 检查命令行参数
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " input.csv output.csv" << std::endl;
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin.is_open()) {
        std::cerr << "Error opening input file: " << argv[1] << std::endl;
        return 1;
    }

    // 用 unordered_map 保存每个 symbol 的交易统计数据
    std::unordered_map<std::string, TradeStats> symbolStats;
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string token;

        // 解析 CSV 行格式：<TimeStamp>,<Symbol>,<Quantity>,<Price>
        std::getline(iss, token, ',');
        long long timestamp = std::stoll(token);

        std::getline(iss, token, ',');
        std::string symbol = token;

        std::getline(iss, token, ',');
        long long quantity = std::stoll(token);

        std::getline(iss, token, ',');
        long long price = std::stoll(token);

        // 查找或创建对应 symbol 的统计数据
        auto it = symbolStats.find(symbol);
        if (it == symbolStats.end()) {
            // 首次出现该 symbol，不存在时间间隔的计算
            TradeStats stats;
            stats.lastTimestamp = timestamp;
            stats.maxTimeGap = 0;
            stats.totalVolume = quantity;
            stats.totalPriceQuantity = price * quantity;
            stats.maxPrice = price;
            symbolStats[symbol] = stats;
        } else {
            TradeStats &stats = it->second;
            // 计算与上一次交易的时间差
            long long gap = timestamp - stats.lastTimestamp;
            if (gap > stats.maxTimeGap) {
                stats.maxTimeGap = gap;
            }
            stats.lastTimestamp = timestamp;
            stats.totalVolume += quantity;
            stats.totalPriceQuantity += price * quantity;
            if (price > stats.maxPrice) {
                stats.maxPrice = price;
            }
        }
    }
    fin.close();

    // 将 symbol 提取到 vector 中，并排序（升序）
    std::vector<std::string> symbols;
    for (const auto &entry : symbolStats) {
        symbols.push_back(entry.first);
    }
    std::sort(symbols.begin(), symbols.end());

    std::ofstream fout(argv[2]);
    if (!fout.is_open()) {
        std::cerr << "Error opening output file: " << argv[2] << std::endl;
        return 1;
    }

    // 输出格式：<symbol>,<MaxTimeGap>,<Volume>,<WeightedAveragePrice>,<MaxPrice>
    // 加权平均价格 = totalPriceQuantity / totalVolume（取整）
    for (const auto &sym : symbols) {
        const TradeStats &stats = symbolStats[sym];
        long long weightedAvgPrice = stats.totalPriceQuantity / stats.totalVolume;
        fout << sym << ","
             << stats.maxTimeGap << ","
             << stats.totalVolume << ","
             << weightedAvgPrice << ","
             << stats.maxPrice << "\n";
    }
    fout.close();

    return 0;
}
