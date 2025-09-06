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

    bool eliminate(MCGAL::Halfedge* h) override;
    bool remove_point(MCGAL::Halfedge* h) override;
    bool triangulate(MCGAL::Halfedge* h) override;
    bool postprocess(MCGAL::Halfedge* h) override;
    bool encode_boundary(MCGAL::Halfedge* h) override;
};