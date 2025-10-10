/*
 * @Author: koi
 * @Date: 2025-09-02 10:39:21
 * @Description:
 */
#include "BasicDeserializeOperator.h"
#include "../common/BufferUtils.h"
#include "Global.h"
#include "Halfedge.h"
#include "MeshUtils.h"
#include "Vertex.h"
#include <cstring>
#include <fstream>
#include <memory>

std::shared_ptr<MCGAL::Mesh> BasicDeserializeOperator::deserializeBaseMesh() {
    MCGAL::contextPool.initPoolSize(1);

    std::shared_ptr<MCGAL::Mesh> mesh = MCGAL::readBaseMesh(DEFAULT_MESH_ID, buffer_, dataOffset_);
    // find seed
    MCGAL::Vertex* v0 = mesh->vertices()[0];
    MCGAL::Vertex* v1 = mesh->vertices()[1];
    for (MCGAL::Halfedge* hit : v0->halfedges()) {
        if (hit->end_vertex() == v1) {
            seed = hit;
            break;
        }
    }
    this->mesh_ = mesh;
    reconstructOperator_->init(mesh);
    return mesh;
}

void BasicDeserializeOperator::deserializeOneRound() {
    std::vector<MCGAL::Vertex*> vertices;
    std::vector<MCGAL::Halfedge*> halfedges;
    std::vector<MCGAL::Facet*> facets;
    // symbolReadOperator_->enableQuantization(mesh_->bboxMin, mesh_->f_quantStep);
    symbolReadOperator_->collect(seed, buffer_, dataOffset_, vertices, halfedges, facets);
    reconstructOperator_->reconstruct(vertices, halfedges, facets);
}

std::shared_ptr<MCGAL::Mesh> BasicDeserializeOperator::exportMesh() {
    return mesh_;
}
