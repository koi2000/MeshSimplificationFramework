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

    void init(std::shared_ptr<MCGAL::Mesh> mesh, bool compress_boundary = false) override;

    bool eliminate(MCGAL::Halfedge* h) override;
    bool remove_point(MCGAL::Halfedge* h) override;
    bool triangulate(MCGAL::Halfedge* h) override;
    bool postprocess(MCGAL::Halfedge* h) override;
    MCGAL::Halfedge* encode_boundary(MCGAL::Halfedge* h) override;
    int groupId = -1;
};