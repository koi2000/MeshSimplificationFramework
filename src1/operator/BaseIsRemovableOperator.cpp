#include "core.h"
#include "BaseIsRemovableOperator.h"

bool BaseIsRemovableOperator::isRemovable(MCGAL::Halfedge* halfedge) {
    if(halfedge->isBoundary() || halfedge->opposite()->isBoundary()) {
        return false;
    }
    return true;
}
