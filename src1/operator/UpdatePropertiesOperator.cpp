#include "UpdatePropertiesOperator.h"
#include "MeshUtils.h"
void UpdatePropertiesOperator::updatePropertity(int meshId, MCGAL::Halfedge* halfedge) {
    double dError = 0;
    MCGAL::Point ptResult;
    MCGAL::Vertex* v0 = halfedge->vertex();
    MCGAL::Vertex* v1 = halfedge->vertex();

    SymetricMatrix v0m = MCGAL::contextPool.property(meshId, m_vpQuadrics, v0);
    SymetricMatrix v1m = MCGAL::contextPool.property(meshId, m_vpQuadrics, v1);
    for (MCGAL::Halfedge* hIt : v0->halfedges()) {
        dError = MCGAL::CalculateError(v0m, v1m, hIt, ptResult);
        MCGAL::contextPool.property(meshId, m_epError, hIt) = dError;
        MCGAL::contextPool.property(meshId, m_epTargetPoints, hIt) = ptResult;
        MCGAL::contextPool.property(meshId, m_epDirty, hIt) = true;
    }

    // for (MCGAL::Halfedge* hIt : v1->halfedges()) {
    //     dError = MCGAL::CalculateError(v1m, v0m, hIt, ptResult);
    //     MCGAL::contextPool.property(meshId, m_epError, hIt) = dError;
    //     MCGAL::contextPool.property(meshId, m_epTargetPoints, hIt) = ptResult;
    //     MCGAL::contextPool.property(meshId, m_epDirty, hIt) = true;
    // }
};
