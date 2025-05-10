#ifndef MULTI_SIMPLE_SIMPLIFICATION_H
#define MULTI_SIMPLE_SIMPLIFICATION_H

#include "PartialSplitter.h"
#include "core.h"
#include "symetric_matrix.h"

class MultiSimpleEncoder {
  public:
    MultiSimpleEncoder(std::string path);
    ~MultiSimpleEncoder();

    void SimplifyVertexTo(size_t uiRemainedVertexNum, int groupId, MCGAL::Halfedge* seed, double dAgressiveness = 7);

    void encode_group(int groupId, size_t uiRemainedVertexNum);

    void encodeBoundary();

    void encode();

    void simple_encode();

    void encodeVertexSymbol(int groupId);

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

    void dumpGroupTo(std::string path, int groupId);

    void readBaseMeshs();
    void loadBuffer(std::string path);

  private:
    int dataOffset = 0;
    char* buffer;
    MCGAL::Mesh mesh;

    OpenMesh::VPropHandleT<SymetricMatrix> m_vpQuadrics;
    OpenMesh::HPropHandleT<double> m_epError;
    OpenMesh::HPropHandleT<MCGAL::Point> m_epTargetPoints;
    OpenMesh::HPropHandleT<bool> m_epDirty;
    std::vector<std::vector<std::deque<char>>> multiVertexSymbols;
    std::vector<std::vector<std::deque<MCGAL::VertexSplitNode*>>> multiVertexSplitNodes;
    PartialSplitter splitter;
    std::vector<MCGAL::Halfedge*> seeds;
};

#endif