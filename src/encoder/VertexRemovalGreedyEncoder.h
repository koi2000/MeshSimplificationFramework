#ifndef VERTEX_REMOVAL_GREEDY_ENCODER_H
#define VERTEX_REMOVAL_GREEDY_ENCODER_H

#include "../common/Graph.h"
#include "../operator/IsRemovableOperator.h"
#include "core.h"
#include <string>

/**
 * 修改encoder，直接分割压缩 使用vertex removal，找个办法记下来
 * 同时根据点记录三group交界处
 *
 * 边界在decode的时候需要对齐，自己maintain三个点
 *
 * 队列应该是global级别的
 * 对象应该是可以复用的，每次传入一些内容即可
 *
 * encoder应该是无状态的，状态全部交由上层控制。
 */
class VertexRemovalGreedyEncoder {
  public:
    VertexRemovalGreedyEncoder();

    void encode(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed, Graph* graph, std::set<MCGAL::Vertex*>* triPoints, int groupId, bool encode_boundry);

    bool encodeInsideOp(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed);

    void encodeBoundaryOp(int groupId);

  private:
    void resetState(MCGAL::Halfedge* seed);

    void resetBfsState(MCGAL::Halfedge* seed);

    bool isRemovable(MCGAL::Vertex* v);

    MCGAL::Halfedge* vertexCut(MCGAL::Mesh* mesh, std::set<int>& boundaryIds, std::queue<int>& gateQueue, MCGAL::Halfedge* startH);

    bool boundaryRemovableInVertexRemoval(int inner, int outer, MCGAL::Halfedge* hit);

    MCGAL::Halfedge* next_boundary(int ogroupId, MCGAL::Halfedge* boundary);

    MCGAL::Halfedge* find_prev(MCGAL::Halfedge* h);

    MCGAL::Halfedge* boundary_opposite(MCGAL::Halfedge* hit);

    MCGAL::Vertex* opposite_vertex(MCGAL::Vertex* vit, int outer);

    void encodeFacetSymbolOp(int groupId);

    void encodeHalfedgeSymbolOp(int groupId);

    void encodeBoundarySymbolOp(int groupId);

    bool isBoundaryRemovable(MCGAL::Halfedge* h);

    bool checkCompetition(MCGAL::Vertex* v) const;

    bool arePointsCoplanar(std::vector<MCGAL::Point>& points);

    bool isPlanar(const std::vector<MCGAL::Vertex*>& polygon, float epsilon) const;

    bool willViolateManifold(const std::vector<MCGAL::Halfedge*>& polygon) const;

    bool willViolateManifoldInDup(int inner, int outer, const std::vector<MCGAL::Halfedge*>& inner_oneRing, const std::vector<MCGAL::Halfedge*>& outer_oneRing);

  private:
    // 传入的参数
    MCGAL::Halfedge* seed = nullptr;
    int meshId = -1;
    MCGAL::Mesh* mesh = nullptr;
    std::set<MCGAL::Vertex*>* triPoints;

    Graph* graph = nullptr;
    char* buffer;

    std::vector<std::shared_ptr<IsRemovableOperator>> isRemovableOperator;
    std::vector<std::shared_ptr<IsRemovableOperator>> boundaryIsRemovableOperator;

    std::vector<std::vector<std::deque<char>>> connectFaceSyms;
    std::vector<std::vector<std::deque<char>>> connectEdgeSyms;
    std::vector<std::vector<std::deque<char>>> connectBounaryEdgeSyms;
    std::vector<std::vector<std::deque<MCGAL::Point>>> facetPointSyms;
    std::vector<std::vector<std::deque<MCGAL::Point>>> halfedgePointSyms;
};

#endif