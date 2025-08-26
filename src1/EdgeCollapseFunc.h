#ifndef EDGE_COLLAPSE_FUNC
#define EDGE_COLLAPSE_FUNC

#include "MeshUtils.h"
#include "core.h"
#include "symetric_matrix.h"

OpenMesh::VPropHandleT<SymetricMatrix> m_vpQuadrics;
OpenMesh::HPropHandleT<double> m_epError;
OpenMesh::HPropHandleT<MCGAL::Point> m_epTargetPoints;
OpenMesh::HPropHandleT<bool> m_epDirty;

std::function<void(int)> register_func = [](int meshId) {
    MCGAL::contextPool.add_property(meshId, m_vpQuadrics);
    MCGAL::contextPool.add_property(meshId, m_epError);
    MCGAL::contextPool.add_property(meshId, m_epTargetPoints);
    MCGAL::contextPool.add_property(meshId, m_epDirty);
};

std::function<void(MCGAL::Mesh*)> init_func = [](MCGAL::Mesh* mesh) {
    for (MCGAL::Vertex* vertex : mesh->vertices()) {
        MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, vertex).Clear();
    }
    for (MCGAL::Facet* facet : mesh->faces()) {
        MCGAL::Vertex* vh0 = facet->proxyHalfedge()->vertex();
        MCGAL::Vertex* vh1 = facet->proxyHalfedge()->next()->vertex();
        MCGAL::Vertex* vh2 = facet->proxyHalfedge()->next()->next()->vertex();
        const auto& n = facet->computeNormal();

        const double a = n.x();
        const double b = n.y();
        const double c = n.z();
        const double d = -(vh0->point().dot(n));

        SymetricMatrix q(a, b, c, d);

        MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, vh0) += q;
        MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, vh1) += q;
        MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, vh2) += q;
    }

    double dError = 0;
    MCGAL::Point ptResult;
    for (MCGAL::Facet* face : mesh->faces()) {
        MCGAL::Halfedge* st = face->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        do {
            SymetricMatrix v0m = MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, st->vertex());
            SymetricMatrix v1m = MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, st->end_vertex());
            dError = MCGAL::CalculateError(v0m, v1m, st, ptResult);
            MCGAL::contextPool.property(mesh->meshId(), m_epError, st) = dError;
            MCGAL::contextPool.property(mesh->meshId(), m_epTargetPoints, st) = ptResult;
            st = st->next();
        } while (st != ed);
    }
};

#endif