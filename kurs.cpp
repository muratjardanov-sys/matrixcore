#include <iostream>
#include <vector>
#include <set>
#include <iomanip>
#include "CppSolver.h"

using namespace std;

void printPathJson(const EvaluatedPath& p) {
    cout << "{\"nodes\":[";
    for (size_t i = 0; i < p.nodes.size(); ++i) {
        cout << p.nodes[i] << (i + 1 < p.nodes.size() ? "," : "");
    }
    cout << "],\"latency\":" << p.totalLatency
        << ",\"bandwidth\":" << p.bottleneckBw << "}";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Формат входного потока от Spring Boot:
    // maxNodeId threads maxK src dst minBw
    // failedCount [список отключенных узлов...]
    // edgeCount [u v latency bandwidth ...]
    int maxNodeId, threads, maxK, src, dst, minBw;
    if (!(cin >> maxNodeId >> threads >> maxK >> src >> dst >> minBw)) {
        return 0;
    }

    int failedCount;
    cin >> failedCount;
    set<int> failedNodes;
    for (int i = 0; i < failedCount; ++i) {
        int fn;
        cin >> fn;
        failedNodes.insert(fn);
    }

    int edgeCount;
    cin >> edgeCount;
    AdjacencyList adj(maxNodeId + 1);
    for (int i = 0; i < edgeCount; ++i) {
        int u, v, lat, bw;
        cin >> u >> v >> lat >> bw;
        if (u >= 0 && u <= maxNodeId && v >= 0 && v <= maxNodeId) {
            adj[u].push_back({ v, lat, bw });
        }
    }

    CppSolver solver(maxNodeId + 1, adj, failedNodes, minBw);
    AnalysisResult res = solver.solveSDN(threads, maxK, src, dst);

    // Вывод результата в формате JSON для Spring Boot
    cout << fixed << setprecision(3);
    cout << "{";
    cout << "\"engine\":\"C++ OpenMP (AMM)\",";
    cout << "\"executionTimeMs\":" << res.executionTimeMs << ",";
    cout << "\"totalPathsFound\":" << res.allTargetPaths.size() << ",";

    cout << "\"primaryPath\":";
    if (res.hasPrimary) printPathJson(res.primaryPath);
    else cout << "null";
    cout << ",";

    cout << "\"backupPath\":";
    if (res.hasBackup) printPathJson(res.backupPath);
    else cout << "null";
    cout << ",";

    cout << "\"allPaths\":[";
    size_t limit = min<size_t>(res.allTargetPaths.size(), 50); // Отдаем топ-50 путей для таблицы
    for (size_t i = 0; i < limit; ++i) {
        printPathJson(res.allTargetPaths[i]);
        if (i + 1 < limit) cout << ",";
    }
    cout << "]}";

    return 0;
}