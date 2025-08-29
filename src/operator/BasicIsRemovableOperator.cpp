/*
 * @Author: koi
 * @Date: 2025-08-28 21:51:18
 * @Description:
 */

#include "BasicIsRemovableOperator.h"
#include "Halfedge.h"

bool BasicIsRemovableOperator::isRemovable(MCGAL::Halfedge* halfedge) {
    if (halfedge->vertex()->isCollapsed() || halfedge->end_vertex()->isCollapsed()) {
        return false;
    }
    for (MCGAL::Halfedge* h : halfedge->end_vertex()->halfedges()) {
        if (h->end_vertex()->isCollapsed() || h->isAdded()) {
            return false;
        }
    }
    for (MCGAL::Halfedge* h : halfedge->vertex()->halfedges()) {
        if (h->end_vertex()->isCollapsed() || h->isAdded()) {
            return false;
        }
    }
    return true;
}
