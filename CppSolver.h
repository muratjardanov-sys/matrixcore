#pragma once
#include "PathNode.h"

class CppSolver {
private:
    int size;
    AdjacencyList adjList;

public:
    CppSolver(int n, const AdjacencyList& list);
    double runTest(int threads, int maxK);
};