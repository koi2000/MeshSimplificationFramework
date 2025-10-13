/*
 * @Author: koi
 * @Date: 2025-08-27 15:24:33
 * @Description:
 */
#include "PriorityErrorSelector.h"
#include "selection/HalfedgeSelectionQueue.h"
#include <functional>

PriorityErrorSelector::PriorityErrorSelector(SelectOptions options) : SelectOperator(options) {
    ErrorSource source = options.getErrorSource();
    auto halfedgeErrorAccessor = options.getHalfedgeErrorAccessor();
    auto vertexErrorAccessor = options.getVertexErrorAccessor();
    queue = HalfedgeSelectionQueue(source, halfedgeErrorAccessor, vertexErrorAccessor, options.getIsRemovableOperators());
}

void PriorityErrorSelector::init(std::shared_ptr<MCGAL::Mesh> mesh) {
    mesh_ = mesh;
    queue.buildFromMesh(mesh);
}

void PriorityErrorSelector::addIsRemovableOperator(std::shared_ptr<IsRemovableOperator> op) {
    queue.addIsRemovableOperator(op);
}

PriorityErrorSelector::~PriorityErrorSelector() {}

void PriorityErrorSelector::reset() {
    mesh_->resetState();
    queue.buildFromMesh(mesh_);
}

bool PriorityErrorSelector::select(MCGAL::Halfedge*& halfedge, MCGAL::Point& p) {
    auto pair = queue.popNext();
    halfedge = pair.first;
    p = pair.second;
    if (halfedge == nullptr) {
        return false;
    }
    return true;
}

bool PriorityErrorSelector::update(MCGAL::Halfedge*& halfedge) {
    for (MCGAL::Halfedge* h : halfedge->vertex()->halfedges()) {
        if (queue.isValid(h)) {
            queue.pushOrUpdate(h);
        }
    }

    return true;
}