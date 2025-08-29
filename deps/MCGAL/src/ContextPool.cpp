/*
 * @Author: koi
 * @Date: 2025-03-23 18:00:20
 * @Description: 
 */
#include "ContextPool.h"
#include "string.h"
namespace MCGAL {

ContextPool::ContextPool() {}

void ContextPool::initPoolSize(int size) {
    vindexs.resize(size);
    hindexs.resize(size);
    findexs.resize(size);
}

int ContextPool::registerPool(int estimateVSize, int estimateHSize, int estimateFSize) {
    estimateVSize *= 10;
    estimateHSize *= 10;
    estimateFSize *= 10;

    vpoolSizes.push_back(estimateVSize);
    hpoolSizes.push_back(estimateHSize);
    fpoolSizes.push_back(estimateFSize);

    vprops_.push_back(OpenMesh::PropertyContainer());
    hprops_.push_back(OpenMesh::PropertyContainer());
    fprops_.push_back(OpenMesh::PropertyContainer());

    MCGAL::Vertex* vpool = new MCGAL::Vertex[estimateVSize];
    MCGAL::Halfedge* hpool = new MCGAL::Halfedge[estimateHSize];
    MCGAL::Facet* fpool = new MCGAL::Facet[estimateFSize];

    for (int i = 0; i < estimateVSize; i++) {
        vpool[i].setPoolId(i);
    }

    for (int i = 0; i < estimateHSize; i++) {
        hpool[i].setPoolId(i);
    }

    for (int i = 0; i < estimateFSize; i++) {
        fpool[i].setPoolId(i);
    }

    vpools.push_back(vpool);
    hpools.push_back(hpool);
    fpools.push_back(fpool);
    return poolIndex++;
}

void ContextPool::releasePool(int index) {}

ContextPool::~ContextPool() {
    for (size_t i = 0; i < poolIndex; ++i) {
        delete vpools[i];
    }

    for (size_t i = 0; i < poolIndex; ++i) {
        delete hpools[i];
    }

    for (size_t i = 0; i < poolIndex; ++i) {
        delete fpools[i];
    }
}

}  // namespace MCGAL