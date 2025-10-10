/*
 * @Author: koi
 * @Date: 2025-09-02 10:39:21
 * @Description:
 */
#include "SegmentationDeserializeOperator.h"
#include "../common/BufferUtils.h"
#include "Configuration.h"
#include "Global.h"
#include "Halfedge.h"
#include "Mesh.h"
#include "MeshUtils.h"
#include "Vertex.h"
#include <cstring>
#include <fstream>
#include <memory>
#include <vector>
#include "../common/DebugTool.h"

std::shared_ptr<MCGAL::Mesh> SegmentationDeserializeOperator::deserializeBaseMesh() {
    // 从readBaseMesh就错了，必须从头开始fix
    
    MCGAL::contextPool.initPoolSize(1);
    unsigned i_nbVerticesBaseMesh = readInt(buffer_, dataOffset_);
    unsigned i_nbFacesBaseMesh = readInt(buffer_, dataOffset_);
    MCGAL::contextPool.registerPool(i_nbVerticesBaseMesh * 5, i_nbFacesBaseMesh * 5, i_nbFacesBaseMesh * 15);
    // std::shared_ptr<MCGAL::Mesh> mesh = MCGAL::readBaseMesh(DEFAULT_MESH_ID, buffer_, dataOffset_);
    mesh_ = std::make_shared<MCGAL::Mesh>();
    mesh_->setMeshId(0);
    // find seed
    // 首先读取出n个int，代表每个group的数量
    std::vector<int> groupSize = MCGAL::charPtrToVectorInt(buffer_, dataOffset_, header_.getGroupNumber());
    int sum = 0;
    for (int a : groupSize) {
        sum += a;
    }
    for (int i = 0; i < header_.getGroupNumber(); i++) {
        MCGAL::Vertex* v0 = MCGAL::contextPool.allocateVertexFromPool(DEFAULT_MESH_ID, readPoint(buffer_, dataOffset_));
        MCGAL::Vertex* v1 = MCGAL::contextPool.allocateVertexFromPool(DEFAULT_MESH_ID, readPoint(buffer_, dataOffset_));
        v0->setGroupId(i);
        v1->setGroupId(i);
        mesh_->add_vertex(v0);
        mesh_->add_vertex(v1);
        // for (MCGAL::Halfedge* hit : v0->halfedges()) {
        //     if (hit->end_vertex() == v1) {
        //         seeds.push_back(hit);
        //         break;
        //     }
        // }

    }
    for (int j = 0; j < groupSize.size(); j++) {
        for (int i = 0; i < groupSize[j]; i++) {
            MCGAL::Vertex* vt = MCGAL::contextPool.allocateVertexFromPool(DEFAULT_MESH_ID, readPoint(buffer_, dataOffset_));
            vt->setGroupId(j);
            mesh_->add_vertex(vt);
        }
    }
    for (int i = 0; i < i_nbFacesBaseMesh; ++i) {
        std::vector<MCGAL::Vertex*> vts;
        for (int j = 0; j < 3; ++j) {
            int vertex_index = readInt(buffer_, dataOffset_);
            vts.push_back(mesh_->get_vertex(vertex_index));
        }
        MCGAL::Facet* face = MCGAL::contextPool.allocateFaceFromPool(DEFAULT_MESH_ID, vts);
        mesh_->add_face(face);
    }
    int beforeOffset = dataOffset_;
    groupOffset = MCGAL::charPtrToVectorInt(buffer_, dataOffset_, header_.getGroupNumber());
    for (int i = 0; i < header_.getGroupNumber(); i++) {
        groupOffset[i] += dataOffset_;
        MCGAL::Vertex* v0 = mesh_->get_vertex(i * 2);
        MCGAL::Vertex* v1 = mesh_->get_vertex(i * 2 + 1);
        for (MCGAL::Halfedge* hit : v0->halfedges()) {
            if (hit->end_vertex() == v1) {
                seeds.push_back(hit);
                break;
            }
        }
        std::string path = "./debug/seed" + std::to_string(i) + ".off"; 
        // DEBUG_DUMP_VERTEX_THREE_RING(seeds[i]->vertex(), path);
        DEBUG_DUMP_VERTEX_ONE_RING(seeds[i]->vertex(), path);
    }
    // this->mesh_ = mesh;
    reconstructOperator_->init(mesh_);
    return mesh_;
}

void SegmentationDeserializeOperator::deserializeOneRound() {
    std::vector<MCGAL::Vertex*> vertices;
    std::vector<MCGAL::Halfedge*> halfedges;
    std::vector<MCGAL::Facet*> facets;
    // symbolReadOperator_->enableQuantization(mesh_->bboxMin, mesh_->f_quantStep);
    for (int i = 0; i < seeds.size(); i++) {
        symbolReadOperator_->collect(seeds[i], buffer_, groupOffset[i], vertices, halfedges, facets);
    }
    reconstructOperator_->reconstruct(vertices, halfedges, facets);
}

std::shared_ptr<MCGAL::Mesh> SegmentationDeserializeOperator::exportMesh() {
    return mesh_;
}
