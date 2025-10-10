/*
 * @Author: koi
 * @Date: 2025-09-17 17:19:40
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-08-29 21:11:03
 * @Description:
 */

#ifndef SEGMENTATION_SYMBOLCOLLECTOPERATOR_H
#define SEGMENTATION_SYMBOLCOLLECTOPERATOR_H

#include "SymbolCollectOperator.h"
#include <memory>
#include <vector>

class SegmentationSymbolCollectOperator : public SymbolCollectOperator {
  public:
    SegmentationSymbolCollectOperator() = default;

    ~SegmentationSymbolCollectOperator(){};

    void init(std::shared_ptr<MCGAL::Mesh> mesh) override {  
      this->mesh_ = mesh;
    };

    void collect(MCGAL::Halfedge* seed) override;
    void collect(std::vector<MCGAL::Halfedge*> seed) override;
    int exportToBuffer(char* buffer, bool enableQuantization = false) override;

  private:
    void collectHalfedgeSymbol(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed);
    void collectFacetSymbol(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed);

    std::vector<std::vector<std::deque<MCGAL::Point>>> segmentPointQueues;
    std::vector<std::vector<std::deque<MCGAL::PointInt>>> segmentIpointQueues;
    std::vector<std::vector<std::deque<char>>> segmentFacetSymbolQueues;
    std::vector<std::vector<std::deque<char>>> segmentEdgeSymbolQueues;
    int groupIdx = 0;
    int inited = false;

    int fcount = 0;
    int ecount = 0;
};

#endif