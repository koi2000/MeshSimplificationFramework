/*
 * @Author: koi
 * @Date: 2025-09-03 21:29:50
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-08-25 22:29:32
 * @Description:
 */
#ifndef SEED_ISREOVABLEOPERATOR_H
#define SEED_ISREOVABLEOPERATOR_H

#include "Halfedge.h"
#include "Vertex.h"
#include "operator/IsRemovableOperator.h"
#include <cstddef>

class SeedIsRemovableOperator : public IsRemovableOperator {
  public:
    SeedIsRemovableOperator() = default;

    void init(MCGAL::Vertex* v, MCGAL::Halfedge* h, MCGAL::Facet* f) {
        this->seed = h;
    };

    bool isRemovable(MCGAL::Halfedge* halfedge) {
        if (seed->vertex()->poolId() == halfedge->vertex()->poolId() || seed->end_vertex()->poolId() == halfedge->end_vertex()->poolId() ||
            seed->end_vertex()->poolId() == halfedge->vertex()->poolId() || seed->vertex()->poolId() == halfedge->end_vertex()->poolId()) {
            return false;
        }
        return true;
    };

    MCGAL::Halfedge* seed = nullptr;
};

#endif