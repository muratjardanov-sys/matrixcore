#pragma once
#include <vector>
#include <memory>
#include <cstdint>

struct EdgeInfo {
    int to;
    int latency;    // Задержка канала (мс)
    int bandwidth;  // Пропускная способность (Гбит/с)
};

struct PathNode {
    int vertex;
    int totalLatency;
    int minBandwidth;
    uint64_t visitedMask; // Битовая маска посещенных узлов для O(1) фильтрации циклов
    std::shared_ptr<PathNode> parent;

    PathNode(int v, int lat = 0, int bw = 1000000, std::shared_ptr<PathNode> p = nullptr)
        : vertex(v), totalLatency(lat), minBandwidth(bw), parent(p) {
        uint64_t bit = (v >= 0 && v < 64) ? (1ULL << v) : 0ULL;
        visitedMask = p ? (p->visitedMask | bit) : bit;
    }

    // Дельта-функция проверки отсутствия циклов за O(1) (до 64 узлов)
    bool hasVertex(int v) const {
        if (v >= 0 && v < 64) {
            return (visitedMask & (1ULL << v)) != 0;
        }
        const PathNode* curr = this;
        while (curr) {
            if (curr->vertex == v) return true;
            curr = curr->parent.get();
        }
        return false;
    }
};

using Route = std::shared_ptr<PathNode>;
using RouteList = std::vector<Route>;
using AdjacencyList = std::vector<std::vector<EdgeInfo>>;