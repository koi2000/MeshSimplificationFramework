#ifndef BASEISREOVABLEOPERATOR_H
#define BASEISREOVABLEOPERATOR_H

#include "./IsRemovableOperator.h"

class BaseIsRemovableOperator : public  IsRemovableOperator {
  public:
    BaseIsRemovableOperator() {};

    bool isRemovable(MCGAL::Halfedge* halfedge) override;
};

#endif