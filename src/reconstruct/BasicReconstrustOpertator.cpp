/*
 * @Author: koi
 * @Date: 2025-09-02 15:45:26
 * @Description:
 */
#include "BasicReconstructOpertator.h"
#include "Facet.h"
#include "Mesh.h"
#include "MeshUtils.h"

void BasicReconstructOpertator::reconstruct(MCGAL::Halfedge* halfedge) {}

void BasicReconstructOpertator::reconstruct(std::vector<MCGAL::Vertex*>& vertices,
                                            std::vector<MCGAL::Halfedge*>& halfedge,
                                            std::vector<MCGAL::Facet*>& facets) {
    int count = 0;
    for (MCGAL::Halfedge* hit : halfedge) {
        if (count == 89) {
            int i = 0;
        }
        merge_face(hit);
        count++;
    }
    count = 0;
    for (MCGAL::Facet* f : facets) {
        if (count == 57) {
            int i = 0;
        }
        insert_point(f);
        count++;
    }
}

void BasicReconstructOpertator::merge_face(MCGAL::Halfedge* halfedge) {
    if (halfedge->isRemoved()) {
        return;
    } 
    MCGAL::join_face(halfedge);
}

/**
看不懂在做什么，明天继续debug
 */
void BasicReconstructOpertator::insert_point(MCGAL::Facet* facet) {
    mesh_->create_center_vertex_without_init(facet->proxyHalfedge(), facet->getRemovedVertexPos());
}