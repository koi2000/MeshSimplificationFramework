#ifndef SIMPLE_SIMPLIFICATION_H
#define SIMPLE_SIMPLIFICATION_H

#include "core.h"
#include "symetric_matrix.h"

class SimpleDecoder {
  public:
    SimpleDecoder(std::string path);
    ~SimpleDecoder();

    void decode();

    std::vector<MCGAL::Vertex*> decodeVertexSymbol();

    void dumpTo(std::string path);
    void buildFromBuffer(MCGAL::Mesh& subMesh, std::deque<MCGAL::Point>* p_pointDeque, std::deque<uint32_t*>* p_faceDeque);

    void readBaseMeshs();
    void readSeed();
    void loadBuffer(std::string path);

  private:
    int dataOffset = 0;
    char* buffer;
    MCGAL::Mesh mesh;
    MCGAL::Halfedge* seed;

    std::vector<std::deque<char>> vertexSymbols;
    std::vector<std::deque<MCGAL::VertexSplitNode*>> vertexSplitNodes;
};

#endif