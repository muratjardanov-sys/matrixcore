#include "CppSolver.h"
#include "PathNode.h"
#include <chrono>
#include <omp.h>
using namespace std;

CppSolver::CppSolver(int n, const AdjacencyList& list) : size(n), adjList(list) {}

double CppSolver::runTest(int threads, int maxK) {
    omp_set_num_threads(threads);
    vector<vector<RouteList>> matrix(size, vector<RouteList>(size));
    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < size; ++i) {
        for (int nextV : adjList[i]) {
            matrix[i][nextV].push_back(make_shared<PathNode>(nextV, make_shared<PathNode>(i)));
        }
    }
    for (int step = 2; step < maxK; ++step) {
        vector<vector<RouteList>> nextMatrix(size, vector<RouteList>(size));

        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (matrix[i][j].empty()) continue;
                for (const Route& route : matrix[i][j]) {
                    for (int nextV : adjList[j]) {
                        if (!route->hasVertex(nextV)) {
                            nextMatrix[i][nextV].push_back(make_shared<PathNode>(nextV, route));
                        }
                    }
                }
            }
        }
        matrix = move(nextMatrix);
    }
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration<double>(end - start).count();
}