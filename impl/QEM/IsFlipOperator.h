#pragma once

#include "operator/IsRemovableOperator.h"

class IsFlipOperator : public IsRemovableOperator {
  public:
    IsFlipOperator() = default;

    void init(std::vector<MCGAL::Vertex*> v, std::vector<MCGAL::Halfedge*> h, std::vector<MCGAL::Facet*> f) {
        this->seeds = h;
    };

    bool isRemovable(MCGAL::Halfedge* halfedge) {
        for (MCGAL::Halfedge* seed : seeds) {
            if (seed->vertex()->poolId() == halfedge->vertex()->poolId() || seed->end_vertex()->poolId() == halfedge->end_vertex()->poolId() ||
                seed->end_vertex()->poolId() == halfedge->vertex()->poolId() || seed->vertex()->poolId() == halfedge->end_vertex()->poolId()) {
                return false;
            }
        }
        return true;
    };

    std::vector<MCGAL::Halfedge*> seeds;
};