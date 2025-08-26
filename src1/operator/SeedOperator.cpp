
#include "SeedOperator.h"
#include "core.h"

MCGAL::Halfedge* SeedOperator::chooseSeed(MCGAL::Mesh& mesh) {
    return mesh.faces_[0]->proxyHalfedge();
};
