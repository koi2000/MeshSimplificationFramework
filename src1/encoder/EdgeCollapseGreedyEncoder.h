#ifndef EDGE_COLLAPSE_GREEDY_ENCODER_H
#define EDGE_COLLAPSE_GREEDY_ENCODER_H

#include "../common/Graph.h"
#include "../operator/IsRemovableOperator.h"
#include "core.h"
#include <string>

/**
 * 修改encoder，直接分割压缩 使用vertex removal，找个办法记下来
 * 同时根据点记录三group交界处
 *
 * 边界在decode的时候需要对齐，自己maintain三个点
 */
class EdgeCollapseGreedyEncoder {
  public:
    EdgeCollapseGreedyEncoder() {};

    void encode(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed, Graph* graph, std::set<MCGAL::Vertex*>* triPoints, int groupId, bool encode_boundry);

    bool encodeInsideOp(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed);

    void encodeBoundaryOp(int groupId);

    MCGAL::Halfedge* edgeCut(MCGAL::Mesh* mesh, std::set<int>& boundaryIds, std::queue<int>& gateQueue, MCGAL::Halfedge* hit);

  private:
    bool isRemovable(MCGAL::Halfedge* hit);

    void resetBfsState();

    void resetState();

    MCGAL::Halfedge* next_boundary(int ogroupId, MCGAL::Halfedge* boundary);

    bool boundaryRemovableInEdgeCollapse(int inner, int outer, MCGAL::Halfedge* hit);
    
    bool isFlipped(MCGAL::Halfedge* edge, const MCGAL::Point& ptTarget);

  private:
    int dataOffset = 0;

    MCGAL::Halfedge* seed = nullptr;
    int meshId = -1;
    MCGAL::Mesh* mesh = nullptr;

    Graph* graph = nullptr;
    char* buffer;
    std::set<MCGAL::Vertex*>* triPoints;

    std::vector<std::shared_ptr<IsRemovableOperator>> isRemovableOperator;
    std::vector<std::shared_ptr<IsRemovableOperator>> boundaryIsRemovableOperator;

    std::vector<std::vector<std::deque<char>>> connectFaceSyms;
    std::vector<std::vector<std::deque<char>>> connectEdgeSyms;
    std::vector<std::vector<std::deque<char>>> connectBounaryEdgeSyms;
    std::vector<std::vector<std::deque<MCGAL::Point>>> facetPointSyms;
    std::vector<std::vector<std::deque<MCGAL::Point>>> halfedgePointSyms;
};

#endif