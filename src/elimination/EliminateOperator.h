/*
 * @Author: koi
 * @Date: 2025-08-26 22:07:17
 * @Description:
 */
#ifndef ELIMINATE_OPERATOR_H
#define ELIMINATE_OPERATOR_H

#include "Halfedge.h"
#include "Mesh.h"
#include <memory>

/**
 * eliminate中需要传入边，然后执行删除操作，这里需要考虑一些其他问题，如何存以及周围的更新
 * 这里可以分为三步，1. 删除点，2.三角化 3. 调整点的位置
 */

class EliminateOperator {
  public:
    EliminateOperator() = default;

    virtual void init(std::shared_ptr<MCGAL::Mesh> mesh, bool compress_boundary = false) = 0;

    virtual ~EliminateOperator() = default;

    virtual bool eliminate(MCGAL::Halfedge* h, MCGAL::Point& p) = 0;

    virtual bool remove_point(MCGAL::Halfedge* h) = 0;

    virtual bool triangulate(MCGAL::Halfedge* h) = 0;

    virtual bool postprocess(MCGAL::Halfedge* h) = 0;

    virtual MCGAL::Halfedge* encode_boundary(MCGAL::Halfedge* h) = 0;

    std::shared_ptr<MCGAL::Mesh> mesh_;
    bool compress_boundary_ = false;
};

#endif