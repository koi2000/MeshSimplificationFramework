/*
 * @Author: koi
 * @Date: 2025-08-29 20:50:10
 * @Description:
 */
#ifndef SYMBOLCOLLECTOPERATOR_H
#define SYMBOLCOLLECTOPERATOR_H

#include "Mesh.h"
#include <deque>
#include <memory>
#include <vector>

/**
 * @brief 符号收集操作符
 * @details 需要对数据结构和函数进行抽象，
 */
class SymbolCollectOperator {
  public:
    SymbolCollectOperator() = default;
    virtual void init(std::shared_ptr<MCGAL::Mesh> mesh) = 0;
    // 收集一轮的信息
    virtual void collect(MCGAL::Halfedge* h) = 0;
    virtual void collect(std::vector<MCGAL::Halfedge*> hs) {};
    // 将收集到的信息导出到buffer中
    virtual int exportToBuffer(char* buffer, bool enableQuantization = false) = 0;

    std::shared_ptr<MCGAL::Mesh> mesh_;
};
#endif