/*
 * @Author: koi
 * @Date: 2025-08-25 22:29:32
 * @Description:
 */
#ifndef ISREOVABLEOPERATOR_H
#define ISREOVABLEOPERATOR_H

#include "Halfedge.h"
#include "Vertex.h"

class IsRemovableOperator {
  public:
    IsRemovableOperator() = default;

    virtual void init(MCGAL::Vertex* v, MCGAL::Halfedge* h, MCGAL::Facet* f){};

    virtual bool isRemovable(MCGAL::Halfedge* halfedge) = 0;
};

#endif