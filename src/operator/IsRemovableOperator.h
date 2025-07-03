#ifndef ISREOVABLEOPERATOR_H
#define ISREOVABLEOPERATOR_H

#include "core.h"

class IsRemovableOperator {
  public:
    IsRemovableOperator() {};

    virtual bool isRemovable(MCGAL::Halfedge* halfedge) = 0;
};

#endif