#ifndef PARTIAL_SPLITTER_H
#define PARTIAL_SPLITTER_H

#include "../common/Graph.h"
#include "core.h"
#include "omp.h"
#include <cstring>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#define RANDOM_SEED 3098

/**
 * 内存中需要构建一个图结构，点是subMesh，边指的是两个subMesh之间有邻接关系
 */
class PartialSplitter {
  public:
    PartialSplitter() = default;

    PartialSplitter(std::string filename);

    void buildGraph();

    PartialSplitter(MCGAL::Mesh* mesh, bool skipMarkBoundary = false);

    void loadMesh(MCGAL::Mesh* mesh, bool skipMarkBoundary = false);

    int split(int groupNumber = -1);

    void dumpSubMesh(std::string path, int groupId);

    std::vector<MCGAL::Halfedge*>& exportSeeds();

    std::set<MCGAL::Vertex*>& exportTriPoints();

    Graph exportGraph();

    std::vector<MCGAL::Mesh>& exportSubMeshes();

    std::unordered_map<int, int> exportDup2Origin();

    std::unordered_map<int, std::unordered_map<int, int>> exportOrigin2Dup();

    std::set<int> neExpand(std::set<int>& allSet, int number);

    void neAllocEdges(std::set<int> allSet, std::set<int>& coreSet, std::set<int>& boundarySet, std::set<int>& part, int x, int number);

  private:
    int markBoundry();

  private:
    std::set<MCGAL::Vertex*> triPoints;
    MCGAL::Mesh* mesh = nullptr;
    std::vector<int> unRemovedPoint;
    std::vector<MCGAL::Mesh> subMeshes;
    std::vector<MCGAL::Halfedge*> seeds;
    std::vector<std::set<int>> triJunction;
    std::unordered_map<int, int> dup2origin;
    std::vector<int> boundarys;
    /**
     * key为groupId
     * value 为origin到新点的集合，其中有pair first为groupId，second为dup的id
     */
    std::unordered_map<int, std::unordered_map<int, int>> origin2dup;
    Graph g;
};

#endif