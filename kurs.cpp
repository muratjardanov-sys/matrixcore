#include <iostream>
#include <vector>
#include <fstream>
#include <ctime>
#include <string>
#include "PathNode.h"
#include "CppSolver.h"
#include "SqlSolver.h"

using namespace std;

void runExperiment(int nodes, double density, string filename) {
    cout << "\nЗапуск эксперимента: " << nodes << " узлов, плотность " << density << endl;
    AdjacencyList adjList(nodes);
    vector<pair<int, int>> edges;
    for (int i = 0; i < nodes; ++i) {
        for (int j = 0; j < nodes; ++j) {
            if (i != j && (rand() / (double)RAND_MAX) < density) {
                adjList[i].push_back(j);
                edges.push_back({ i, j });
            }
        }
    }
    CppSolver cpp(nodes, adjList);
    SqlSolver sql;
    sql.fillData(edges);
    ofstream outFile(filename);
    outFile << "PathLength,Sequential,Parallel,SQL\n";
    cout << "| k | Последоват. | Параллел. | SQL |" << endl;
    cout << "---------------------------------------" << endl;
    for (int k = 2; k <= 7; ++k) {
        double tSeq = cpp.runTest(1, k);      // 1 поток
        double tPar = cpp.runTest(4, k);      // 4 потока
        double tSql = sql.runTest(k);         // SQLite

        printf("| %d | %10.6fs | %10.6fs | %10.6fs |\n", k, tSeq, tPar, tSql);
        outFile << k << "," << tSeq << "," << tPar << "," << tSql << "\n";
    }
    outFile.close();
}

int main() {
    setlocale(LC_ALL, "Russian");
    srand(static_cast<unsigned>(time(0)));
    runExperiment(40, 0.15, "res_1.csv");
    runExperiment(60, 0.10, "res_2.csv");
    runExperiment(30, 0.30, "res_3.csv");
    return 0;
}