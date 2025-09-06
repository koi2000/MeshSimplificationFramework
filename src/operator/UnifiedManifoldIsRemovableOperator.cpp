/*
 * @Author: koi
 * @Date: 2025-08-29 11:12:36
 * @Description:
 */
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

#include "UnifiedManifoldIsRemovableOperator.h"
#include "Facet.h"
#include "Halfedge.h"
#include "MeshUtils.h"
#include "Point.h"
#include "Vertex.h"
#include <vector>

bool UnifiedManifoldIsRemovableOperator::isRemovable(MCGAL::Halfedge* h) {
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
        st = st->next()->opposite()->next();
    } while (st != ed);
    if (!(v->vertex_degree() > 2 && v->vertex_degree() < 8)) {
        return false;
    }
    for (MCGAL::Halfedge* hit : h->end_vertex()->halfedges()) {
        vs.push_back(hit->end_vertex());
    }
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
}
