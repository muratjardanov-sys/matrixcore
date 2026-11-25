#pragma once
#include "PathNode.h"
#include <vector>
#include <set>

struct EvaluatedPath {
    std::vector<int> nodes;
    int totalLatency;
    int bottleneckBw;
    uint64_t intermediateMask; // Маска транзитных узлов для поиска непересекающихся путей
};

struct AnalysisResult {
    double executionTimeMs;
    std::vector<EvaluatedPath> allTargetPaths;
    EvaluatedPath primaryPath;
    EvaluatedPath backupPath;
    bool hasPrimary = false;
    bool hasBackup = false;
};

class CppSolver {
private:
    int size;
    AdjacencyList adjList;
    std::set<int> failedNodes;
    int minBandwidthFilter;

    bool isFailed(int v) const {
        return failedNodes.find(v) != failedNodes.end();
    }

public:
    CppSolver(int n, const AdjacencyList& list, const std::set<int>& failed, int minBw);
    AnalysisResult solveSDN(int threads, int maxK, int src, int dst);
};