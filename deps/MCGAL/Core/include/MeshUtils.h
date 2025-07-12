#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include "core.h"
#include "symetric_matrix.h"

namespace MCGAL {

static double VertexError(const SymetricMatrix& q, double x, double y, double z) {
    return q[0] * x * x + 2 * q[1] * x * y + 2 * q[2] * x * z + 2 * q[3] * x + q[4] * y * y + 2 * q[5] * y * z + 2 * q[6] * y + q[7] * z * z + 2 * q[8] * z + q[9];
}

static double CalculateError(SymetricMatrix v0m, SymetricMatrix v1m, MCGAL::Halfedge* edge, MCGAL::Point& ptResult) {
    MCGAL::Halfedge* h0 = edge;
    MCGAL::Vertex* v0 = h0->vertex();
    MCGAL::Vertex* v1 = h0->end_vertex();
    SymetricMatrix q = v0m + v1m;

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

static bool IsFlipped(MCGAL::Halfedge* edge, const MCGAL::Point& ptTarget) {
    MCGAL::Vertex* v0 = edge->vertex();
    MCGAL::Vertex* v1 = edge->end_vertex();
    // 检查坍缩后是否会导致重叠面片或非流形网格
    // 坍缩后确保其他点的邻居数不能少于三个

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

static MCGAL::Halfedge* next_boundary(int ogroupId, MCGAL::Halfedge* boundary) {
    MCGAL::Halfedge* nxt = boundary->next();
    if (nxt->isBoundary()) {
        return nxt;
    }
    nxt = boundary->next()->opposite()->next();
    if (nxt->isBoundary()) {
        return nxt;
    }
    for (MCGAL::Halfedge* hit : boundary->end_vertex()->halfedges()) {
        if (hit->opposite() != boundary && hit->isBoundary() && hit->opposite()->face()->groupId() == ogroupId) {
            return hit;
        }
        // if (hit->face()->groupId() != ogroupId && hit->opposite()->face()->groupId() == ogroupId) {
        //     return hit;
        // }
    }
    return nullptr;
}

Halfedge* find_prev(Halfedge* h) const {
    Halfedge* g = h;
    while (g->next() != h) {
        g = g->next();
    }
    return g;
}

}  // namespace MCGAL

#endif