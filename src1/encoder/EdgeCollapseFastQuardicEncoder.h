#ifndef EDGE_COLLAPSE_FAST_QUARDIC_ENCODER_H
#define EDGE_COLLAPSE_FAST_QUARDIC_ENCODER_H

#include "../common/Graph.h"
#include "../operator/IsRemovableOperator.h"
#include "core.h"
#include <string>
#include "symetric_matrix.h"
#include "../operator/UpdatePropertiesOperator.h"

class EdgeCollapseFastQuardicEncoder {
  public:
    EdgeCollapseFastQuardicEncoder() {};

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

    OpenMesh::VPropHandleT<SymetricMatrix> m_vpQuadrics;
    OpenMesh::HPropHandleT<double> m_epError;
    OpenMesh::HPropHandleT<MCGAL::Point> m_epTargetPoints;
    OpenMesh::HPropHandleT<bool> m_epDirty;

    std::shared_ptr<UpdatePropertiesOperator> updatePropertiesOperator;
};

#endif