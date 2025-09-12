/*
 * @Author: koi
 * @Date: 2025-08-29 21:11:03
 * @Description:
 */

#ifndef BASIC_SYMBOLCOLLECTOPERATOR_H
#define BASIC_SYMBOLCOLLECTOPERATOR_H

#include "SymbolCollectOperator.h"
#include <memory>

class BasicSymbolCollectOperator : public SymbolCollectOperator {
  public:
    BasicSymbolCollectOperator() = default;

    void init(std::shared_ptr<MCGAL::Mesh> mesh) override {
        this->mesh_ = mesh;
    };

    void collect(MCGAL::Halfedge* seed) override;
    int exportToBuffer(char* buffer, bool enableQuantization = false) override;

  private:
    void collectHalfedgeSymbol(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed);
    void collectFacetSymbol(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed);

    std::vector<std::deque<MCGAL::Point>> pointQueues;
    std::vector<std::deque<char>> facetSymbolQueues;
    std::vector<std::deque<char>> edgeSymbolQueues;
    
};

#endif