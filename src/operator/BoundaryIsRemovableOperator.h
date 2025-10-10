/*
 * @Author: koi
 * @Date: 2025-09-20 11:51:21
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-09-03 21:29:50
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-08-25 22:29:32
 * @Description:
 */
#ifndef BOUNDARY_ISREOVABLEOPERATOR_H
#define BOUNDARY_ISREOVABLEOPERATOR_H

#include "Facet.h"
#include "Halfedge.h"
#include "Vertex.h"
#include "operator/IsRemovableOperator.h"
#include <cstddef>
#include <set>
#include <vector>

class BoundaryIsRemovableOperator : public IsRemovableOperator {
  public:
    BoundaryIsRemovableOperator() = default;

    void init(std::vector<MCGAL::Vertex*> v, std::vector<MCGAL::Halfedge*> h, std::vector<MCGAL::Facet*> f){};

    bool isRemovable(MCGAL::Halfedge* halfedge) {
        if (halfedge->isBoundary()) {
            return false;
        }
        for (MCGAL::Halfedge* hit : halfedge->vertex()->halfedges()) {
            if (hit->isBoundary()) {
                return false;
            }
        }
        for (MCGAL::Halfedge* hit : halfedge->end_vertex()->halfedges()) {
            if (hit->isBoundary()) {
                return false;
            }
        }
        return true;
        // if (!halfedge->isBoundary()) {
        //     return true;
        // }
        // return check(halfedge) && check(halfedge->next_boundary());
    };

    bool check(MCGAL::Halfedge* halfedge) {
        if (!halfedge) {
            return false;
        }
        std::set<int> cnt;
        for (MCGAL::Halfedge* vhit : halfedge->vertex()->halfedges()) {
            if (vhit->face()->groupId()) {
                cnt.insert(vhit->face()->groupId());
            }
        }
        if (cnt.size() >= 3) {
            return false;
        }
        cnt.clear();
        for (MCGAL::Halfedge* vhit : halfedge->end_vertex()->halfedges()) {
            if (vhit->face()->groupId()) {
                cnt.insert(vhit->face()->groupId());
            }
        }
        if (cnt.size() >= 3) {
            return false;
        }

        return true;
    }
};

#endif