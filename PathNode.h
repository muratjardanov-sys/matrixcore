#pragma once
#include "PathNode.h"
#include <vector>
#include <memory>

using namespace std;


struct PathNode {
    int vertex;
    shared_ptr<PathNode> parent;

    PathNode(int v, shared_ptr<PathNode> p = nullptr) : vertex(v), parent(p) {}

    bool hasVertex(int v) const {
        const PathNode* curr = this;
        while (curr) {
            if (curr->vertex == v) return true;
            curr = curr->parent.get();
        }
        return false;
    }
};

using Route = shared_ptr<PathNode>;
using RouteList = vector<Route>;
using AdjacencyList = vector<vector<int>>;
