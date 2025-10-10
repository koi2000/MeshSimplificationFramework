/*
 * @Author: koi
 * @Date: 2025-09-23 21:57:25
 * @Description:
 */
#include "BoundaryCompressIsRemovableOperator.h"
#include "Facet.h"
#include "Halfedge.h"
#include "MeshUtils.h"
#include "Vertex.h"
#include "common/DebugTool.h"

/**
    需要注意一种情况，避免因为边界压缩导致某一个三角形被阻塞住
    甚至可能出现两个三角形被阻塞住的情况

    还有一种情况，可能直接把对面穿透了

    还有一种情况，和第三方的边界连起来了

    太麻烦了 要不就不这样做了
*/
static bool boundaryBlockCheck(MCGAL::Halfedge* hit) {
    // 更新检查条件，如果任意一个顶点含有两种面，就不压缩
    MCGAL::Halfedge* st = hit->find_prev();
    MCGAL::Halfedge* ed = st;
    std::set<MCGAL::Facet*> fset;
    // do {
    //     fset.insert(st->face());
    //     st = st->next()->opposite()->next();
    // } while (st != ed);
    for (MCGAL::Halfedge* hit : hit->end_vertex()->halfedges()) {
        fset.insert(hit->face());
    }

    do {
        MCGAL::Vertex* v = st->vertex();
        MCGAL::Halfedge* ist = v->halfedges()[0]->next();
        // if(ist->face()->groupId() != ist->opposite()->face()->groupId() && ist->face()->groupId() != hit->face()->groupId()) {
        //     return false;
        // }
        MCGAL::Halfedge* ied = ist;
        int cnv = 0;
        int lastId = ist->face()->groupId();
        if (fset.count(ist->face())) {
            lastId = -1;
        }
        do {
            if (fset.count(ist->face()) && lastId != -1) {
                cnv++;
                lastId = -1;
            } else if (lastId == -1 && !fset.count(ist->face())) {
                cnv++;
                lastId = ist->face()->groupId();
            } else if (lastId != -1 && !fset.count(ist->face()) && ist->face()->groupId() != lastId) {
                cnv++;
                lastId = ist->face()->groupId();
            }
            ist = ist->next()->opposite()->next();
        } while (ist != ied);
        if (cnv >= 3) {
            // DEBUG_DUMP_VERTEX_THREE_RING(hit->end_vertex(), "./debug-boundary-check.off");
            return false;
        }
        st = st->next()->opposite()->next();
    } while (st != ed);
    return true;
}

bool BoundaryCompressIsRemovableOperator::isRemovable(MCGAL::Halfedge* h) {
    assert(h->isBoundary());
    if (triPoints.count(h->vertex()) || triPoints.count(h->end_vertex())) {
        return false;
    }
    if (h->isBoundary() && h->next()->isBoundary()) {
        if (h->hasRemovedVertex() || h->next()->hasRemovedVertex()) {
            return false;
        }
    }
    if (!boundaryBlockCheck(h)) {
        return false;
    }
    std::vector<MCGAL::Vertex*> vs;
    MCGAL::Vertex* v = h->end_vertex();
    for (MCGAL::Halfedge* hit : h->vertex()->halfedges()) {
        if (hit->isAdded()) {
            return false;
        }
    }
    for (MCGAL::Halfedge* hit : h->end_vertex()->halfedges()) {
        if (hit->isAdded()) {
            return false;
        }
    }
    MCGAL::Halfedge* st = h->opposite()->next();
    MCGAL::Halfedge* ed = h->opposite()->next();
    do {
        if (st->isAdded()) {
            return false;
        }
        vs.push_back(st->vertex());
        st = st->next()->opposite()->next();
    } while (st != ed);
    if (!(v->vertex_degree() > 2 && v->vertex_degree() < 8)) {
        return false;
    }
    // for (MCGAL::Halfedge* hit : h->end_vertex()->halfedges()) {
    //     vs.push_back(hit->end_vertex());
    // }
    int n = vs.size();
    for (int i = 0; i < vs.size(); i++) {
        int j = ((i + 2) % n);
        while (j != ((i + n - 1) % n)) {
            for (MCGAL::Halfedge* hit : vs[i]->halfedges()) {
                if (hit->end_vertex() == vs[j]) {
                    return false;
                }
            }
            for (MCGAL::Halfedge* hit : vs[j]->halfedges()) {
                if (hit->end_vertex() == vs[i]) {
                    return false;
                }
            }
            j = (j + 1) % n;
        }
    }
    return true;
    // MCGAL::Vertex* v = hit->end_vertex();
    // // if (v->poolId() == seed->vertex()->poolId() || v->poolId() == seed->end_vertex()->poolId()) {
    // //     return false;
    // // }
    // if (!v->isConquered() && v->vertex_degree() > 2 && v->vertex_degree() < 8) {
    //     std::vector<MCGAL::Point> vh_oneRing;
    //     std::vector<MCGAL::Halfedge*> heh_oneRing;
    //     heh_oneRing.reserve(v->vertex_degree());
    //     for (MCGAL::Halfedge* hit : v->halfedges()) {
    //         // if (hit == seed || (hit->opposite() != nullptr && hit->opposite() == seed)) {
    //         //     return false;
    //         // }
    //         vh_oneRing.push_back(hit->opposite()->vertex()->point());
    //         heh_oneRing.push_back(hit);
    //     }
    //     bool removable = !willViolateManifold(heh_oneRing);  //  && arePointsCoplanar(vh_oneRing);
    //     // if (removable) {
    //     //     return checkCompetition(v);
    //     // }
    //     return removable;
    // }
    // return false;
}

bool BoundaryCompressIsRemovableOperator::willViolateManifold(const std::vector<MCGAL::Halfedge*>& polygon) {
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
                        if (MCGAL::isPointInTriangle(it->vertex()->point(), it->end_vertex()->point(), jt->end_vertex()->point(),
                                                     it->next()->end_vertex()->point())) {
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