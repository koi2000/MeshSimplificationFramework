/*
 * @Author: koi
 * @Date: 2025-08-25 22:30:09
 * @Description:
 */
#ifndef GRAPH_H
#define GRAPH_H

#include "Vertex.h"
#include <algorithm>
#include <set>
#include <unordered_map>
#include <vector>

class Node {
  public:
    int st;
    int ed;
    int stop;

    mutable bool visiable = false;
    Node() {}

    Node(int st_, int ed_, int stop_) : st(st_), ed(ed_), stop(stop_) {}

    void setVisiable() const {
        this->visiable = true;
    }

    void setUnvisiable() const {
        this->visiable = false;
    }

    bool isVisiable() const {
        return this->visiable;
    }

    bool operator==(const Node& other) const {
        return st == other.st && ed == other.ed && stop == other.stop;
    }
};

class Graph {
  public:
    Graph() = default;

    void addHEdge(int u, int v, Node&& node) {
        g[u][v].push_back(node);
    }

    void addHEdge(int u, int v) {
        g[u].insert({v, {}});
    }

    std::unordered_map<int, std::vector<Node>>& getNode(int id) {
        return g[id];
    }

    // Graph(Graph&& other) noexcept : g(std::move(other.g)) {}

    // Graph& operator=(Graph&& other) noexcept {
    //     if (this != &other) {
    //         g = std::move(other.g);
    //     }
    //     return *this;
    // }

    std::set<int> getAllGroupId() {
        std::set<int> keySet;

        std::transform(g.begin(), g.end(), std::inserter(keySet, keySet.begin()), [](const auto& pair) { return pair.first; });
        return keySet;
    }

    void setTriPoints(std::set<MCGAL::Vertex*> triPoints) {
        this->triPoints = triPoints;
    }

    std::set<MCGAL::Vertex*> getTriPoints() {
        return triPoints;
    }

    void resetState() {
        for (auto& [k, mp] : g) {
            for (auto& [k, v] : mp) {
                for (auto& node : v) {
                    node.setUnvisiable();
                }
            }
        }
    }

  private:
    std::unordered_map<int, std::unordered_map<int, std::vector<Node>>> g;
    std::set<MCGAL::Vertex*> triPoints;
};

#endif