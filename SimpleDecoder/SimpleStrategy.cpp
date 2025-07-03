#include "BufferUtils.h"
#include "VertexSplitNode.h"
#include "biops.h"
#include "include/SimpleEncoder.h"
#include <fstream>
#include <unordered_set>

void SimpleEncoder::SimplifyVertexTo(size_t uiRemainedVertexNum, double dAgressiveness /*=7*/) {
    if (uiRemainedVertexNum >= mesh.vertices().size()) {
        return;
    }

    size_t nCurCollapses = 0;
    size_t nCollapses = mesh.vertices().size() - uiRemainedVertexNum;

    // m_mesh.update_face_normals();

    Initialize();

    for (int iteration = 0; iteration < 100; iteration++) {
        if (nCurCollapses >= nCollapses)
            break;
        double dThreshold = 0.000000001 * pow(double(iteration + 3), dAgressiveness);

        for (int i = 0; i < MCGAL::contextPool.getHindex(mesh.meshId()); i++) {
            MCGAL::Halfedge* eIt = MCGAL::contextPool.getHalfedgeByIndex(mesh.meshId(), i);
            MCGAL::contextPool.property(mesh.meshId(), m_epDirty, eIt) = false;
        }

        for (int i = 0; i < MCGAL::contextPool.getHindex(mesh.meshId()); i++) {
            MCGAL::Halfedge* eIt = MCGAL::contextPool.getHalfedgeByIndex(mesh.meshId(), i);
            MCGAL::Halfedge* h0 = eIt;
            MCGAL::Vertex* v0 = h0->vertex();
            MCGAL::Vertex* v1 = h0->end_vertex();
            if (eIt->isRemoved()) {
                continue;
            }
            if (MCGAL::contextPool.property(mesh.meshId(), m_epError, eIt) > dThreshold)
                continue;
            if (MCGAL::contextPool.property(mesh.meshId(), m_epDirty, eIt))
                continue;

            MCGAL::Point ptTarget = MCGAL::contextPool.property(mesh.meshId(), m_epTargetPoints, eIt);
            if (IsFlipped(eIt, ptTarget)) {
                continue;
            }
            if (IsFlipped(eIt->opposite(), ptTarget)) {
                continue;
            }
            MCGAL::Halfedge* h1 = h0->opposite();
            // seed不能被压缩
            if (h0 == seed || h0->next() == seed || h0->next()->next() == seed || h0->opposite() == seed || h0->opposite()->next() == seed || h0->opposite()->next()->next() == seed) {
                continue;
            }
            if (h0 != seed && h0->opposite() != seed && mesh.is_collapse_ok(h0)) {
                MCGAL::Vertex* v = mesh.halfedge_collapse(h0, ptTarget);
                std::string path = "./tp1/res" + std::to_string(v->vsplitNode()->order) + ".off";
                if (v->vsplitNode()->order > 5000) {
                    // mesh.dumpto_oldtype(path);
                }
            } else {
                continue;
            }

            nCurCollapses++;

            // Update related face normal
            UpdateFaceNormal(v1);
            UpdateFaceNormal(v0);

            auto& v0Quadric = MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, v0);
            auto& v1Quadric = MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, v1);
            MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, v1) += v0Quadric;
            MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, v0) += v1Quadric;
            UpdateEdgePropertyAroundV(v1);
            UpdateEdgePropertyAroundV(v0);
            if (nCurCollapses >= nCollapses)
                break;
        }
    }
    encodeVertexSymbol();
    dumpToBuffer();
}

bool SimpleEncoder::IsFlipped(MCGAL::Halfedge* edge, const MCGAL::Point& ptTarget) {
    MCGAL::Vertex* v0 = edge->vertex();
    MCGAL::Vertex* v1 = edge->end_vertex();
    // // 检查坍缩后是否会导致重叠面片或非流形网格
    // // 坍缩后确保其他点的邻居数不能少于三个
    // std::vector<MCGAL::Facet*> fset1;
    // std::vector<MCGAL::Facet*> fset2;
    // for (MCGAL::Halfedge* outer : v0->halfedges()) {
    //     if (outer->face() == edge->face() || outer->face() == edge->opposite()->face()) {
    //         continue;
    //     }
    //     if(outer->end_vertex()->halfedges().size() < 4) {
    //         return true;
    //     }
    //     MCGAL::Facet* f1 = outer->face();
    //     std::set<MCGAL::Vertex*> fset1;
    //     MCGAL::Halfedge* st = f1->proxyHalfedge();
    //     MCGAL::Halfedge* ed = st;
    //     do {
    //         if (st->vertex() != v1 || st->vertex() != v0) {
    //             fset1.insert(st->vertex());
    //         }
    //         st = st->next();
    //     } while (st != ed);
    //     for (MCGAL::Halfedge* inner : v1->halfedges()) {
    //         if (inner->face() == edge->face() || inner->face() == edge->opposite()->face()) {
    //             continue;
    //         }
    //         MCGAL::Facet* f2 = inner->face();
    //         std::set<MCGAL::Vertex*> fset2;
    //         MCGAL::Halfedge* st = f2->proxyHalfedge();
    //         MCGAL::Halfedge* ed = st;
    //         do {
    //             if (st->vertex() != v1 || st->vertex() != v0) {
    //                 fset2.insert(st->vertex());
    //             }
    //             st = st->next();
    //         } while (st != ed);
    //         if (fset1 == fset2) {
    //             return true;
    //         }
    //     }
    // }

    for (MCGAL::Halfedge* h : v0->halfedges()) {
        if (h->face()->isRemoved()) {
            continue;
        }
        std::vector<MCGAL::Vertex*> vs;
        MCGAL::Halfedge* st = h->face()->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        do {
            vs.push_back(st->vertex());
            st = st->next();
        } while (st != ed);
        if (vs[0] == v1 || vs[1] == v1 || vs[2] == v1) {
            continue;
        }
        int idxV0 = 0;
        for (int i = 0; i < vs.size(); i++) {
            MCGAL::Vertex* v = vs[i];
            if (vs[i] == v0) {
                idxV0 = i;
            }
        }
        MCGAL::Point pt0 = v0->point();
        MCGAL::Point pt1 = vs[(idxV0 + 1) % 3]->point();
        MCGAL::Point pt2 = vs[(idxV0 + 2) % 3]->point();

        MCGAL::Point dir1 = pt1 - ptTarget;
        dir1.normalize();
        MCGAL::Point dir2 = pt2 - ptTarget;
        dir2.normalize();

        // The angle below can be adjusted, but now is enough
        // if the angle between dir1 and dir2 small than 2.6 angle, return true
        if (fabs(dir1.dot(dir2)) > 0.999)
            return true;
        MCGAL::Point normold;
        try {
            normold = h->face()->computeNormal();
        } catch (std::string exp) { return true; }
        MCGAL::Point norm = dir1.cross(dir2);
        norm.normalize();

        // if the angle between normold and norm large than 78.5 angle, return true
        if ((normold.dot(norm)) < 0.2)
            return true;
    }
    return false;
}

void SimpleEncoder::Initialize() {
    for (MCGAL::Vertex* vertex : mesh.vertices()) {
        MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, vertex).Clear();
    }
    for (MCGAL::Facet* facet : mesh.faces()) {
        MCGAL::Vertex* vh0 = facet->proxyHalfedge()->vertex();
        MCGAL::Vertex* vh1 = facet->proxyHalfedge()->next()->vertex();
        MCGAL::Vertex* vh2 = facet->proxyHalfedge()->next()->next()->vertex();
        const auto& n = facet->computeNormal();

        const double a = n.x();
        const double b = n.y();
        const double c = n.z();
        const double d = -(vh0->point().dot(n));

        SymetricMatrix q(a, b, c, d);

        MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, vh0) += q;
        MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, vh1) += q;
        MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, vh2) += q;
    }

    double dError = 0;
    MCGAL::Point ptResult;
    for (MCGAL::Facet* face : mesh.faces()) {
        MCGAL::Halfedge* st = face->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        do {
            dError = CalculateError(st, ptResult);
            MCGAL::contextPool.property(mesh.meshId(), m_epError, st) = dError;
            MCGAL::contextPool.property(mesh.meshId(), m_epTargetPoints, st) = ptResult;
            st = st->next();
        } while (st != ed);
    }
}

double SimpleEncoder::CalculateError(MCGAL::Halfedge* edge, MCGAL::Point& ptResult) {
    MCGAL::Halfedge* h0 = edge;
    MCGAL::Vertex* v0 = h0->vertex();
    MCGAL::Vertex* v1 = h0->end_vertex();
    SymetricMatrix q = MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, v0) + MCGAL::contextPool.property(mesh.meshId(), m_vpQuadrics, v1);

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
}

double SimpleEncoder::VertexError(const SymetricMatrix& q, double x, double y, double z) {
    return q[0] * x * x + 2 * q[1] * x * y + 2 * q[2] * x * z + 2 * q[3] * x + q[4] * y * y + 2 * q[5] * y * z + 2 * q[6] * y + q[7] * z * z + 2 * q[8] * z + q[9];
}

void SimpleEncoder::UpdateFaceNormal(MCGAL::Vertex* v0) {}

void SimpleEncoder::UpdateEdgePropertyAroundV(MCGAL::Vertex* v0) {
    double dError = 0;
    MCGAL::Point ptResult;
    for (MCGAL::Halfedge* hIt : v0->halfedges()) {
        dError = CalculateError(hIt, ptResult);
        MCGAL::contextPool.property(mesh.meshId(), m_epError, hIt) = dError;
        MCGAL::contextPool.property(mesh.meshId(), m_epTargetPoints, hIt) = ptResult;
        MCGAL::contextPool.property(mesh.meshId(), m_epDirty, hIt) = true;
    }
}