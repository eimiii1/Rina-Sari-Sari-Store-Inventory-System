#include "CSVHandler.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace {
    // Splits a single CSV line on commas. Good enough here because none
    // of our fields (names, categories, units) are expected to contain
    // commas themselves — a production CSV parser would need to handle
    // quoted fields, but that's overkill for this case study's data.
    std::vector<std::string> splitLine(const std::string& line) {
        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }
        return fields;
    }

    bool fileExistsAndNonEmpty(const std::string& filename) {
        std::ifstream f(filename, std::ios::ate);
        return f.good() && f.tellg() > 0;
    }
}

namespace CSVHandler {

std::vector<Product> loadInventory(const std::string& filename) {
    std::vector<Product> products;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "No existing inventory file found at '" << filename
                  << "' — starting with an empty inventory.\n";
        return products;
    }

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (isHeader) { isHeader = false; continue; } // skip "code,name,category,..." header row

        std::vector<std::string> fields = splitLine(line);
        if (fields.size() < 8) continue; // malformed row — skip rather than crash

        Product p;
        p.code = fields[0];
        p.name = fields[1];
        p.category = fields[2];
        p.unit = fields[3];
        p.costPrice = std::stod(fields[4]);
        p.sellingPrice = std::stod(fields[5]);
        p.quantity = std::stoi(fields[6]);
        p.reorderLevel = std::stoi(fields[7]);
        products.push_back(p);
    }

    return products;
}

void saveInventory(const std::string& filename, const std::vector<Product>& products) {
    std::ofstream file(filename); // truncates and rewrites the whole file
    file << "code,name,category,unit,cost_price,selling_price,quantity,reorder_level\n";
    for (const auto& p : products) {
        file << p.code << "," << p.name << "," << p.category << "," << p.unit << ","
             << p.costPrice << "," << p.sellingPrice << "," << p.quantity << "," << p.reorderLevel << "\n";
    }
}

void logSale(const std::string& filename, const std::string& timestamp,
             const std::string& productName, int quantity,
             double unitCost, double unitPrice, double total) {
    bool needsHeader = !fileExistsAndNonEmpty(filename);
    std::ofstream file(filename, std::ios::app); // append, never overwrite past sales

    if (needsHeader) {
        file << "timestamp,product_name,quantity,unit_cost,unit_price,total\n";
    }
    file << timestamp << "," << productName << "," << quantity << ","
         << unitCost << "," << unitPrice << "," << total << "\n";
}

std::vector<SaleRecord> loadSales(const std::string& filename) {
    std::vector<SaleRecord> records;
    std::ifstream file(filename);
    if (!file.is_open()) return records;

    std::string line;
    bool isHeader = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (isHeader) { isHeader = false; continue; }

        std::vector<std::string> fields = splitLine(line);
        if (fields.size() < 6) continue;

        SaleRecord r;
        r.timestamp = fields[0];
        r.productName = fields[1];
        r.quantity = std::stoi(fields[2]);
        r.unitCost = std::stod(fields[3]);
        r.unitPrice = std::stod(fields[4]);
        r.total = std::stod(fields[5]);
        records.push_back(r);
    }
    return records;
}

void removeLastSale(const std::string& filename) {
    std::vector<SaleRecord> all = loadSales(filename);
    if (all.empty()) return; // nothing to undo, nothing to rewrite

    all.pop_back();

    std::ofstream file(filename); // truncates and rewrites the whole file
    file << "timestamp,product_name,quantity,unit_cost,unit_price,total\n";
    for (const auto& r : all) {
        file << r.timestamp << "," << r.productName << "," << r.quantity << ","
             << r.unitCost << "," << r.unitPrice << "," << r.total << "\n";
    }
}

}
