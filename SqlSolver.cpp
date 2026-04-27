#include "SqlSolver.h"
#include <chrono>
#include <sstream>

SqlSolver::SqlSolver() { sqlite3_open(":memory:", &db); }
SqlSolver::~SqlSolver() { sqlite3_close(db); }

void SqlSolver::fillData(const std::vector<std::pair<int, int>>& edges) {
    sqlite3_exec(db, "DROP TABLE IF EXISTS Edges; CREATE TABLE Edges (src INT, dst INT);", 0, 0, 0);
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, "INSERT INTO Edges VALUES (?, ?);", -1, &stmt, 0);
    for (const auto& e : edges) {
        sqlite3_bind_int(stmt, 1, e.first);
        sqlite3_bind_int(stmt, 2, e.second);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT; CREATE INDEX idx_src ON Edges(src);", 0, 0, 0);
}

double SqlSolver::runTest(int k) {
    auto start = std::chrono::high_resolution_clock::now();
    std::stringstream ss;
    ss << "SELECT count(*) FROM Edges AS e1";
    for (int i = 2; i <= k; ++i) {
        ss << " JOIN Edges AS e" << i << " ON e" << i - 1 << ".dst = e" << i << ".src";
    }
    sqlite3_exec(db, ss.str().c_str(), 0, 0, 0);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}