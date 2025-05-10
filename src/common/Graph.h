#ifndef GRAPH_H
#define GRAPH_H
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <cstddef>
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

class Hasher {
  public:
    size_t operator()(const Node& n) const {
        return std::hash<int>()(n.st) + std::hash<int>()(n.ed) + std::hash<int>()(n.stop);
    }
};

class Graph {
  public:
    Graph() = default;

    void addHEdge(int u, int v, Node&& node) {
        // if (g[u].count(v)) {
        //     if (g[u][v].dis < node.dis) {
        //         g[u][v] = node;
        //     }
        // } else {
        //     g[u].insert({v, node});
        // }
        g[u][v].push_back(node);
    }

    void addHEdge(int u, int v) {
        // if (g[u].count(v)) {
        //     if (g[u][v].dis < node.dis) {
        //         g[u][v] = node;
        //     }
        // } else {
        //     g[u].insert({v, node});
        // }
        g[u].insert({v, {}});
    }

    // void addEdge(Node& u, Node& v) {
    //     g[u.neighbour].insert(v);
    //     g[v.neighbour].insert(u);
    // }

    // void addEdge(Node&& u, Node&& v) {
    //     g[u.neighbour].insert(v);
    //     g[v.neighbour].insert(u);
    // }

    std::unordered_map<int, std::vector<Node>>& getNode(int id) {
        return g[id];
    }

    // 移动构造函数
    Graph(Graph&& other) noexcept : g(std::move(other.g)) {}

    // 移动赋值运算符
    Graph& operator=(Graph&& other) noexcept {
        if (this != &other) {
            g = std::move(other.g);
        }
        return *this;
    }

  private:
    std::unordered_map<int, std::unordered_map<int, std::vector<Node>>> g;
};

#endif