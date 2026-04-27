#pragma once
#include <vector>
#include <string>
#include "sqlite3.h"

class SqlSolver {
private:
    sqlite3* db;
public:
    SqlSolver();
    ~SqlSolver();
    void fillData(const std::vector<std::pair<int, int>>& edges);
    double runTest(int k);
};
