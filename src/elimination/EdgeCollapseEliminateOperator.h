/*
 * @Author: koi
 * @Date: 2025-08-26 22:18:42
 * @Description:
 */
#pragma once

#include "EliminateOperator.h"
#include "Mesh.h"
#include <memory>

class EdgeCollapseEliminateOperator : public EliminateOperator {
  public:
    EdgeCollapseEliminateOperator() = default;
    ~EdgeCollapseEliminateOperator() = default;

    void eliminate(MCGAL::Halfedge* h) override;
    void remove_point(MCGAL::Halfedge* h) override;
    void triangulate(MCGAL::Halfedge* h) override;
    void postprocess(MCGAL::Halfedge* h) override;
    void encode_boundary(MCGAL::Halfedge* h) override;
};