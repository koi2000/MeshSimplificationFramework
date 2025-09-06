/*
 * @Author: koi
 * @Date: 2025-08-26 22:01:15
 * @Description:
 */
/*
 * @Author: koi
 * @Description: 请填写简介
 */
#ifndef SELECT_OPERATOR_H
#define SELECT_OPERATOR_H

#include "Halfedge.h"
#include "Mesh.h"
#include "options/SelectOptions.h"
#include "operator/IsRemovableOperator.h"
#include <memory>

/**
 * 使用的顺序是先select出来一条边，传入的会有IsRemovable，用于检查选出来的时候可以用
 * 然后进入eliminate，eliminate后会进行一系列的mark，然后会有一个postprocessor
 *
 */

class SelectOperator {
  public:
    SelectOperator(SelectOptions options) : options_(options){};

    virtual void init(std::shared_ptr<MCGAL::Mesh> mesh) = 0;

    virtual ~SelectOperator() = default;
    /**
     * @brief 选择一条边
     *
     * @param halfedge 指针形式传入，函数内部设置
     * @return true 这一轮仍有可以选择的会返回true，否则会返回false
     * @return false
     */
    virtual bool select(MCGAL::Halfedge*& halfedge) = 0;

    virtual void addIsRemovableOperator(std::shared_ptr<IsRemovableOperator> op) = 0;

    virtual void reset() = 0;

    std::shared_ptr<MCGAL::Mesh> mesh_;
    SelectOptions options_;
};

#endif