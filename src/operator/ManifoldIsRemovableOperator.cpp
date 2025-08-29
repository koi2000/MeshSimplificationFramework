/*
 * @Author: koi
 * @Date: 2025-08-29 10:38:31
 * @Description: 
 */
/*
 * @Author: koi
 * @Date: 2025-08-28 21:51:18
 * @Description:
 */

#include "ManifoldIsRemovableOperator.h"
#include "Halfedge.h"
#include "Vertex.h"
#include "Facet.h"
#include "MeshUtils.h"


bool ManifoldIsRemovableOperator::isRemovable(MCGAL::Halfedge* h) {
    MCGAL::Vertex* v = h->end_vertex();
    if (!v->isConquered() && v->vertex_degree() > 2 && v->vertex_degree() < 8) {
        std::vector<MCGAL::Point> vh_oneRing;
        std::vector<MCGAL::Halfedge*> heh_oneRing;
        heh_oneRing.reserve(v->vertex_degree());
        for (MCGAL::Halfedge* hit : v->halfedges()) {
            if (hit->isBoundary() || hit->opposite()->isBoundary()) {
                return false;
            }

            vh_oneRing.push_back(hit->opposite()->vertex()->point());
            heh_oneRing.push_back(hit);
        }
        bool removable = !willViolateManifold(heh_oneRing);  //  && arePointsCoplanar(vh_oneRing);
        // if (removable) {
        //     return checkCompetition(v);
        // }
        return removable;
    }
    return false;
}

bool ManifoldIsRemovableOperator::willViolateManifold(const std::vector<MCGAL::Halfedge*>& polygon) const {
    unsigned i_degree = polygon.size();
    for (unsigned i = 0; i < i_degree; ++i) {
        MCGAL::Halfedge* it = polygon[i];
        if (it->face()->facet_degree() == 3) {
            std::set<int> facecnt;
            MCGAL::Halfedge* st = it->face()->proxyHalfedge();
            MCGAL::Halfedge* ed = it->face()->proxyHalfedge();
            do {
                if (st->opposite() != nullptr) {
                    if (facecnt.count(st->opposite()->face()->poolId())) {
                        return true;
                    }
                    facecnt.insert(st->opposite()->face()->poolId());
                }
                st = st->next();
            } while (st != ed);
            continue;
        }

        for (int j = 0; j < i_degree; j++) {
            MCGAL::Halfedge* jt = polygon[j];
            if (i == j)
                continue;
            if (it->face() == jt->opposite()->face()) {
                MCGAL::Vertex* v1 = it->end_vertex();
                MCGAL::Vertex* v2 = jt->end_vertex();
                for (int k = 0; k < v1->halfedges().size(); k++) {
                    if (v1->halfedges()[k]->end_vertex() == v2) {
                        return true;
                    }
                }
                for (int k = 0; k < v2->halfedges().size(); k++) {
                    if (v2->halfedges()[k]->end_vertex() == v1) {
                        return true;
                    }
                }
                if (it->face()->facet_degree() > 3) {
                    if (it->next()->next()->next() == jt->opposite()) {
                        if (isPointInTriangle(it->vertex()->point(), it->end_vertex()->point(), jt->end_vertex()->point(), it->next()->end_vertex()->point())) {
                            return true;
                        }
                    }
                }
            } else if (it->face() == jt->face()) {
                for (int k = 0; k < it->end_vertex()->halfedges().size(); k++) {
                    if (it->end_vertex()->halfedges()[k]->end_vertex() == jt->end_vertex()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ManifoldIsRemovableOperator::isPointInTriangle(const MCGAL::Point& A, const MCGAL::Point& B, const MCGAL::Point& C, const MCGAL::Point& P) const {
    MCGAL::Vector3 cross1 = MCGAL::crossProduct(A, B, P);
    MCGAL::Vector3 cross2 = MCGAL::crossProduct(B, C, P);
    MCGAL::Vector3 cross3 = MCGAL::crossProduct(C, A, P);
    bool hasNeg = (cross1.z() < 0) || (cross2.z() < 0) || (cross3.z() < 0);
    bool hasPos = (cross1.z() > 0) || (cross2.z() > 0) || (cross3.z() > 0);
    return !(hasNeg && hasPos);
}