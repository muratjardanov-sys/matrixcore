#include "CppSolver.h"
#include <chrono>
#include <algorithm>
#include <omp.h>

using namespace std;

CppSolver::CppSolver(int n, const AdjacencyList& list, const set<int>& failed, int minBw)
    : size(n), adjList(list), failedNodes(failed), minBandwidthFilter(minBw) {}

static EvaluatedPath unwrapRoute(const Route& route, int src, int dst) {
    EvaluatedPath ep;
    ep.totalLatency = route->totalLatency;
    ep.bottleneckBw = route->minBandwidth;
    ep.intermediateMask = 0ULL;

    const PathNode* curr = route.get();
    while (curr) {
        ep.nodes.push_back(curr->vertex);
        if (curr->vertex != src && curr->vertex != dst && curr->vertex >= 0 && curr->vertex < 64) {
            ep.intermediateMask |= (1ULL << curr->vertex);
        }
        curr = curr->parent.get();
    }
    reverse(ep.nodes.begin(), ep.nodes.end());
    return ep;
}

AnalysisResult CppSolver::solveSDN(int threads, int maxK, int src, int dst) {
    omp_set_num_threads(threads);
    AnalysisResult result;
    result.executionTimeMs = 0.0;

    if (src < 0 || src >= size || dst < 0 || dst >= size || isFailed(src) || isFailed(dst)) {
        return result;
    }

    auto start = chrono::high_resolution_clock::now();

    // Матрица текущего шага свертки A^(k)
    vector<vector<RouteList>> matrix(size, vector<RouteList>(size));
    vector<Route> collectedTargetRoutes;

    // Этап 1. Инициализация базовой матрицы A^(1) с учетом фильтрации отказов и QoS
    for (int i = 0; i < size; ++i) {
        if (isFailed(i)) continue;
        for (const auto& edge : adjList[i]) {
            if (isFailed(edge.to) || edge.bandwidth < minBandwidthFilter) continue;
            auto root = make_shared<PathNode>(i, 0, edge.bandwidth, nullptr);
            auto node = make_shared<PathNode>(edge.to, edge.latency, edge.bandwidth, root);
            matrix[i][edge.to].push_back(node);

            if (i == src && edge.to == dst) {
                collectedTargetRoutes.push_back(node);
            }
        }
    }

    // Этап 2. Итерационное (1,0)-свернутое произведение многомерных матриц с OpenMP
    for (int step = 2; step <= maxK; ++step) {
        vector<vector<RouteList>> nextMatrix(size, vector<RouteList>(size));

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < size; ++i) {
            if (isFailed(i)) continue;
            for (int j = 0; j < size; ++j) {
                if (matrix[i][j].empty()) continue;

                for (const Route& route : matrix[i][j]) {
                    for (const auto& edge : adjList[j]) {
                        // Модифицированный дельта-фильтр:
                        // 1) Проверка отсутствия цикла за O(1)
                        // 2) Проверка работоспособности узла
                        // 3) Фильтрация по минимальной пропускной способности (QoS)
                        if (!route->hasVertex(edge.to) &&
                            !isFailed(edge.to) &&
                            edge.bandwidth >= minBandwidthFilter) {

                            int newLat = route->totalLatency + edge.latency;
                            int newBw = min(route->minBandwidth, edge.bandwidth);
                            auto newNode = make_shared<PathNode>(edge.to, newLat, newBw, route);
                            nextMatrix[i][edge.to].push_back(newNode);
                        }
                    }
                }
            }
        }

        // Сохраняем все найденные пути между целевыми узлами src и dst длины step
        for (const Route& r : nextMatrix[src][dst]) {
            collectedTargetRoutes.push_back(r);
        }

        matrix = move(nextMatrix);
    }

    // Этап 3. Поиск основного и независимого резервного маршрута (Node-Disjoint)
    for (const Route& r : collectedTargetRoutes) {
        result.allTargetPaths.push_back(unwrapRoute(r, src, dst));
    }

    // Сортируем маршруты по возрастанию суммарной задержки (при равенстве — по макс. ширине канала)
    sort(result.allTargetPaths.begin(), result.allTargetPaths.end(),
        [](const EvaluatedPath& a, const EvaluatedPath& b) {
            if (a.totalLatency != b.totalLatency) return a.totalLatency < b.totalLatency;
            return a.bottleneckBw > b.bottleneckBw;
        });

    if (!result.allTargetPaths.empty()) {
        result.primaryPath = result.allTargetPaths[0];
        result.hasPrimary = true;

        // Ищем полностью узло-независимый резервный маршрут (пересечение битовых масок == 0)
        for (size_t idx = 1; idx < result.allTargetPaths.size(); ++idx) {
            const auto& candidate = result.allTargetPaths[idx];
            if ((result.primaryPath.intermediateMask & candidate.intermediateMask) == 0ULL) {
                result.backupPath = candidate;
                result.hasBackup = true;
                break;
            }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    result.executionTimeMs = chrono::duration<double, milli>(end - start).count();
    return result;
}