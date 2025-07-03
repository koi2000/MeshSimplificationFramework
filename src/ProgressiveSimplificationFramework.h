#ifndef PROGRESSIVE_SIMPLIFICATION_FRAMEWORD_H
#define PROGRESSIVE_SIMPLIFICATION_FRAMEWORD_H
#include "splitter/PartialSplitter.h"
#include "core.h"
#include "common/Options.h"
#include "operator/IsRemovableOperator.h"
#include "encoder/VertexRemovalGreedyEncoder.h"
#include "encoder/EdgeCollapseGreedyEncoder.h"
#include "encoder/EdgeCollapseFastQuardicEncoder.h"
#include "encoder/EdgeCollapsePriorityErrorEncoder.h"
/**
 * 不可变的部分：
 * independent set
 * 存储的策略
 * boundary 处理
 *
 */

// 需要自己定义很多的operator基类

/**
 * 用户可以自定义的几个hook：
 * 1. 选择起始点
 * 2. 判断isRemovable()
 * 3. 贪心处理 or error based
 * 4. error需要给用户留下可以自定义的接口
 */
class ProgressiveSimplificationFramework {
  private:
    MCGAL::Mesh mesh;
    PartialSplitter partialSplitter;
    std::vector<MCGAL::Halfedge*> seeds;
    Options options;
    Graph graph;
    std::set<MCGAL::Vertex*> triPoints;

    VertexRemovalGreedyEncoder vertexRemovalGreedyEncoder;
    EdgeCollapseGreedyEncoder edgeCollapseGreedyEncoder;
    EdgeCollapseFastQuardicEncoder edgeCollapseFastQuardicEncoder;
    EdgeCollapsePriorityErrorEncoder edgeCollapsePriorityErrorEncoder;

    void encode_group();
    void priority_based_encode(int groupId);
    void fast_quardic_encode(int groupId);
    void vertex_removal_greedy_encode(int groupId);
    void edge_collapse_greedy_encode(int groupId);

    void edge_collapse_fast_quardic_encode(int groupId);
    void edge_collapse_priority_error_encode(int groupId);

    void vertex_removal_greedy_encode();
    void edge_collapse_greedy_encode();
    void edge_collapse_fast_quardic_encode();
    void edge_collapse_priority_error_encode();

    void dumpGraphToBuffer();

    void dumpToFile(std::string path);

    void writeBaseMesh(int groupId);

    void resetState();

  public:
    ProgressiveSimplificationFramework();

    void loadMesh(std::string path);

    void encode(Options options);

    ~ProgressiveSimplificationFramework();
};

#endif