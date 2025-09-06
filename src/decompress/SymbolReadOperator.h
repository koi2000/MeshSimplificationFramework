/*
 * @Author: koi
 * @Date: 2025-09-01 09:37:03
 * @Description:
 */
#ifndef SYMBOLREADOPERATOR_H
#define SYMBOLREADOPERATOR_H

#include "Halfedge.h"
#include "Mesh.h"

/**
 * @brief 符号收集操作符
 * @details 需要对数据结构和函数进行抽象，
 */
class SymbolReadOperator {
  public:
    SymbolReadOperator() = default;
    // 收集一轮的信息
    virtual void collect(MCGAL::Halfedge* seed,
                         char* buffer,
                         int& dataOffset,
                         std::vector<MCGAL::Vertex*>& vertices,
                         std::vector<MCGAL::Halfedge*>& halfedge,
                         std::vector<MCGAL::Facet*>& facets) = 0;
};
#endif