#include "Halfedge.h"
#include "IsFlipOperator.h"
#include "Mesh.h"
#include "PMSF.h"
#include "elimination/EliminateOperator.h"
#include "elimination/VertexRemovalEliminateOperator.h"
#include "operator/IsRemovableOperator.h"
#include "options/CompressOptions.h"
#include "options/SelectOptions.h"
#include "selection/ErrorSource.h"
#include "selection/PriorityErrorSelector.h"
#include "selection/SelectOperator.h"
#include "symetric_matrix.h"
#include <memory>

/**
    这是一个QEM算法使用PMSF实现的实例程序

    首先需要注册error字段，同时注册error初始化函数以及error更新函数，
    还需要选择EliminateOperator

    目前还差一个更新周围邻居

    做完这些后就可以全自动的交给PMSF去做了
*/

int main() {
    PMSF pmsf;
    OpenMesh::VPropHandleT<SymetricMatrix> m_vpQuadrics;
    OpenMesh::HPropHandleT<double> m_epError;
    OpenMesh::HPropHandleT<MCGAL::Point> m_epTargetPoints;
    OpenMesh::HPropHandleT<bool> m_epDirty;

    // pmsf.registerPropertity();

    auto CalculateError = [&](MCGAL::Halfedge* edge, MCGAL::Point& ptResult) {
        MCGAL::Halfedge* h0 = edge;
        MCGAL::Vertex* v0 = h0->vertex();
        MCGAL::Vertex* v1 = h0->end_vertex();
        SymetricMatrix q = MCGAL::contextPool.property(m_vpQuadrics, v0) + MCGAL::contextPool.property(m_vpQuadrics, v1);
        auto VertexError = [&](const SymetricMatrix& q, double x, double y, double z) {
            return q[0] * x * x + 2 * q[1] * x * y + 2 * q[2] * x * z + 2 * q[3] * x + q[4] * y * y + 2 * q[5] * y * z + 2 * q[6] * y + q[7] * z * z +
                   2 * q[8] * z + q[9];
        };
        double dError = 0;
        // The number for Det, is the index in q
        double det = q.Det(0, 1, 2, 1, 4, 5, 2, 5, 7);
        if (fabs(det) > 1e-6 && !edge->isBoundary()) {
            ptResult[0] = -1 / det * (q.Det(1, 2, 3, 4, 5, 6, 5, 7, 8));  // vx = A41/det(q_delta)
            ptResult[1] = 1 / det * (q.Det(0, 2, 3, 1, 5, 6, 2, 7, 8));   // vy = A42/det(q_delta)
            ptResult[2] = -1 / det * (q.Det(0, 1, 3, 1, 4, 6, 2, 5, 8));  // vz = A43/det(q_delta)

            dError = VertexError(q, ptResult[0], ptResult[1], ptResult[2]);
        } else {
            // find if v0, or v1, or midpoint
            MCGAL::Point pt0 = v0->point();
            MCGAL::Point pt1 = v1->point();
            MCGAL::Point pt2 = (pt0 + pt1) / 2;
            double error0 = VertexError(q, pt0[0], pt0[1], pt0[2]);
            double error1 = VertexError(q, pt1[0], pt1[1], pt1[2]);
            double error2 = VertexError(q, pt2[0], pt2[1], pt2[2]);
            dError = std::min(error0, std::min(error1, error2));
            if (error0 == dError) {
                ptResult = pt0;
            }
            if (error1 == dError) {
                ptResult = pt1;
            }
            if (error2 == dError) {
                ptResult = pt2;
            }
        }

        return dError;
    };

    std::shared_ptr<IsRemovableOperator> fop = std::make_shared<IsFlipOperator>();
    std::shared_ptr<SelectOptions> soption = SelectOptions::Builder()
                                                 .withErrorSource(ErrorSource::Halfedge)
                                                 .withHalfedgeErrorAccessor([&](MCGAL::Halfedge* hit, MCGAL::Point& p) {
                                                     p = MCGAL::contextPool.property(m_epTargetPoints, hit);
                                                     return MCGAL::contextPool.property(m_epError, hit);
                                                 })
                                                 .withRegisterProperties([&](std::shared_ptr<MCGAL::Mesh> mesh) {
                                                     MCGAL::contextPool.add_property(mesh->meshId(), m_vpQuadrics);
                                                     MCGAL::contextPool.add_property(mesh->meshId(), m_epError);
                                                     MCGAL::contextPool.add_property(mesh->meshId(), m_epTargetPoints);
                                                     MCGAL::contextPool.add_property(mesh->meshId(), m_epDirty);

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
                                                             dError = CalculateError(st, ptResult);
                                                             MCGAL::contextPool.property(mesh->meshId(), m_epError, st) = dError;
                                                             MCGAL::contextPool.property(mesh->meshId(), m_epTargetPoints, st) = ptResult;
                                                             st = st->next();
                                                         } while (st != ed);
                                                     }
                                                 })
                                                 .withIsRemovableOperator(fop)
                                                 .build();
    std::shared_ptr<EliminateOperator> eop = std::make_shared<VertexRemovalEliminateOperator>();
    std::shared_ptr<SelectOperator> sop = std::make_shared<PriorityErrorSelector>(*soption);
    CompressOptions coption = CompressOptions::Builder()
                                  .setRound(10)
                                  .setPath("/home/koi/mastercode/MeshSimplificationFramework/static/untitled.off")
                                  .setEnablePrediction(false)
                                  .setEnableQuantization(false)
                                  .setEnableSegmentation(false)
                                  .setSelect(sop)
                                  .setEliminate(eop)
                                  .setOutputPath("./qem.bin")
                                  .build();
    pmsf.compress(coption);
    return 0;
}