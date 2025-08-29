/*
 * @Author: koi
 * @Date: 2025-08-26 22:18:50
 * @Description:
 */

#pragma once

#include "EliminateOperator.h"
#include "Halfedge.h"

class VertexRemovalEliminateOperator : public EliminateOperator {
  public:
    VertexRemovalEliminateOperator() = default;
    ~VertexRemovalEliminateOperator() = default;

    void init(std::shared_ptr<MCGAL::Mesh> mesh) override;

    void eliminate(MCGAL::Halfedge* h) override;
    void remove_point(MCGAL::Halfedge* h) override;
    void triangulate(MCGAL::Halfedge* h) override;
    void postprocess(MCGAL::Halfedge* h) override;
    void encode_boundary(MCGAL::Halfedge* h) override;
};