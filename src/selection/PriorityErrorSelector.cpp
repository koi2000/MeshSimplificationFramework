/*
 * @Author: koi
 * @Date: 2025-08-27 15:24:33
 * @Description:
 */
#include "PriorityErrorSelector.h"
#include "selection/HalfedgeSelectionQueue.h"
#include <functional>

PriorityErrorSelector::PriorityErrorSelector(std::shared_ptr<MCGAL::Mesh> mesh, SelectOptions options) : SelectOperator(mesh, options) {
    ErrorSource source = options.getErrorSource();
    auto halfedgeErrorAccessor = options.getHalfedgeErrorAccessor();
    auto vertexErrorAccessor = options.getVertexErrorAccessor();
    queue = HalfedgeSelectionQueue(source, halfedgeErrorAccessor, vertexErrorAccessor);
}

PriorityErrorSelector::~PriorityErrorSelector() {}

bool PriorityErrorSelector::select(MCGAL::Halfedge* halfedge) {
    halfedge = queue.popNext();
    if (halfedge == nullptr) {
        return false;
    }
    return true;
}