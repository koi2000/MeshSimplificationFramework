/*
 * @Author: koi
 * @Date: 2025-10-05 12:54:48
 * @Description:
 */
#pragma once

#include "Halfedge.h"
#include "operator/IsRemovableOperator.h"
class VertexBasedBoundaryIsRemovableOperator : public IsRemovableOperator {
  public:
    VertexBasedBoundaryIsRemovableOperator() = default;
    ~VertexBasedBoundaryIsRemovableOperator() = default;
    bool isRemovable(MCGAL::Halfedge* h) {
        for (MCGAL::Halfedge* h : h->vertex()->halfedges()) {
            if (h->vertex()->groupId() != h->end_vertex()->groupId()) {
              
                return false;
            }
        }
        for (MCGAL::Halfedge* h : h->end_vertex()->halfedges()) {
            if (h->vertex()->groupId() != h->end_vertex()->groupId()) {
                return false;
            }
        }
        return true;
        //  h->vertex()->groupId() == h->end_vertex()->groupId();
    };
};