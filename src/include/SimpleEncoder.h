#ifndef SIMPLE_SIMPLIFICATION_H
#define SIMPLE_SIMPLIFICATION_H

#include "core.h"
#include "symetric_matrix.h"

class SimpleEncoder {
  public:
    SimpleEncoder(std::string path);
    ~SimpleEncoder();

    void SimplifyVertexTo(size_t uiRemainedVertexNum, double dAgressiveness = 7);

    void simple_encode();
    void fast_quardic_encode();
    void encodeVertexSymbol();

    void writeBaseMesh();
    void dumpTo(std::string path);
    void dumpToBuffer();
    void dumpToFile(std::string path);

    bool IsFlipped(MCGAL::Halfedge* edge, const MCGAL::Point& ptTarget);

    void Initialize();
    double CalculateError(MCGAL::Halfedge* edge, MCGAL::Point& ptResult);
    double VertexError(const SymetricMatrix& q, double x, double y, double z);
    void UpdateFaceNormal(MCGAL::Vertex* v0);
    void UpdateEdgePropertyAroundV(MCGAL::Vertex* v0);
    void serializeVertexSplitNode(MCGAL::VertexSplitNode* node);
    void buildFromBuffer(MCGAL::Mesh& subMesh, std::deque<MCGAL::Point>* p_pointDeque, std::deque<uint32_t*>* p_faceDeque);

    void readBaseMeshs();
    void loadBuffer(std::string path);

  private:
    int dataOffset = 0;
    char* buffer;
    MCGAL::Mesh mesh;
    MCGAL::Halfedge* seed;

    OpenMesh::VPropHandleT<SymetricMatrix> m_vpQuadrics;
    OpenMesh::HPropHandleT<double> m_epError;
    OpenMesh::HPropHandleT<MCGAL::Point> m_epTargetPoints;
    OpenMesh::HPropHandleT<bool> m_epDirty;
    std::vector<std::deque<char>> vertexSymbols;
    std::vector<std::deque<MCGAL::VertexSplitNode*>> vertexSplitNodes;
};

#endif