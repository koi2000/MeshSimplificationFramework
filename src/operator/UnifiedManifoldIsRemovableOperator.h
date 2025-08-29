/*
 * @Author: koi
 * @Date: 2025-08-28 21:51:09
 * @Description:
 */
#ifndef UNIFIEDMANIFOLD_ISREOVABLEOPERATOR_H
#define UNIFIEDMANIFOLD_ISREOVABLEOPERATOR_H

#include "Halfedge.h"
#include "Vertex.h"
#include "operator/IsRemovableOperator.h"

class UnifiedManifoldIsRemovableOperator : public IsRemovableOperator {
  public:
    UnifiedManifoldIsRemovableOperator() = default;

    bool isRemovable(MCGAL::Halfedge* halfedge);
};

#endif