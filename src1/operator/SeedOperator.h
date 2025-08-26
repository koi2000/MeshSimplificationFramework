#ifndef ISREOVABLEOPERATOR_H
#define ISREOVABLEOPERATOR_H

#include "core.h"

class SeedOperator {
  public:
    SeedOperator() {};

    MCGAL::Halfedge* chooseSeed(MCGAL::Mesh& mesh);
};

#endif