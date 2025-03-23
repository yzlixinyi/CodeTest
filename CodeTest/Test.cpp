#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

// ����ÿ�� symbol ��ͳ����Ϣ
struct TradeStats {
    long long lastTimestamp;       // ���һ�ʽ��׵�ʱ���
    long long maxTimeGap;          // ���ʱ����
    long long totalVolume;         // �ܽ�������Quantity ��ͣ�
    long long totalPriceQuantity;  // �ۼƼ۸�������������ڼ����Ȩƽ���۸�
    long long maxPrice;            // ����׼۸�
};

int main(int argc, char* argv[]) {
    // ��������в���
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " input.csv output.csv" << std::endl;
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin.is_open()) {
        std::cerr << "Error opening input file: " << argv[1] << std::endl;
        return 1;
    }

    // �� unordered_map ����ÿ�� symbol �Ľ���ͳ������
    std::unordered_map<std::string, TradeStats> symbolStats;
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string token;

        // ���� CSV �и�ʽ��<TimeStamp>,<Symbol>,<Quantity>,<Price>
        std::getline(iss, token, ',');
        long long timestamp = std::stoll(token);

        std::getline(iss, token, ',');
        std::string symbol = token;

        std::getline(iss, token, ',');
        long long quantity = std::stoll(token);

        std::getline(iss, token, ',');
        long long price = std::stoll(token);

        // ���һ򴴽���Ӧ symbol ��ͳ������
        auto it = symbolStats.find(symbol);
        if (it == symbolStats.end()) {
            // �״γ��ָ� symbol��������ʱ�����ļ���
            TradeStats stats;
            stats.lastTimestamp = timestamp;
            stats.maxTimeGap = 0;
            stats.totalVolume = quantity;
            stats.totalPriceQuantity = price * quantity;
            stats.maxPrice = price;
            symbolStats[symbol] = stats;
        } else {
            TradeStats &stats = it->second;
            // ��������һ�ν��׵�ʱ���
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

    // �� symbol ��ȡ�� vector �У�����������
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

    // �����ʽ��<symbol>,<MaxTimeGap>,<Volume>,<WeightedAveragePrice>,<MaxPrice>
    // ��Ȩƽ���۸� = totalPriceQuantity / totalVolume��ȡ����
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
