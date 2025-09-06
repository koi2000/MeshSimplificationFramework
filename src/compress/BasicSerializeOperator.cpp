/*
 * @Author: koi
 * @Date: 2025-09-01 20:09:27
 * @Description:
 */

#include "BasicSerializeOperator.h"
#include "../common/BufferUtils.h"
#include <fstream>
#include <string>

void BasicSerializeOperator::initBuffer(int size) {
    buffer_ = new char[size];
}

void BasicSerializeOperator::serializeInt(int val) {
    writeInt(buffer_, dataOffset_, val);
}

void BasicSerializeOperator::serializeBaseMesh(std::shared_ptr<MCGAL::Mesh> mesh) {}

void BasicSerializeOperator::serializeBaseMeshWithSeed(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed) {
    mesh->garbage_collection();
    unsigned i_nbVerticesBaseMesh = mesh->size_of_vertices();
    unsigned i_nbFacesBaseMesh = mesh->size_of_facets();

    writeInt(buffer_, dataOffset_, i_nbVerticesBaseMesh);
    writeInt(buffer_, dataOffset_, i_nbFacesBaseMesh);
    int id = 0;
    seed->vertex()->setVid(id++);
    seed->end_vertex()->setVid(id++);
    writePoint(buffer_, dataOffset_, seed->vertex()->point());
    writePoint(buffer_, dataOffset_, seed->end_vertex()->point());

    for (MCGAL::Vertex* vit : mesh->vertices()) {
        if (vit->isRemoved()) {
            continue;
        }
        if (vit->poolId() == seed->vertex()->poolId() || vit->poolId() == seed->end_vertex()->poolId()) {
            continue;
        }
        MCGAL::Point point = vit->point();
        writePoint(buffer_, dataOffset_, point);
        vit->setVid(id++);
    }
    for (MCGAL::Facet* fit : mesh->faces()) {
        if (fit->isRemoved()) {
            continue;
        }
        int i_faceDegree = 0;

        MCGAL::Halfedge* st = fit->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        do {
            i_faceDegree++;
            st = st->next();
        } while (st != ed);
        writeInt(buffer_, dataOffset_, i_faceDegree);
        do {
            writeInt(buffer_, dataOffset_, st->vertex()->vid());
            st = st->next();
        } while (st != ed);
    }
}

void BasicSerializeOperator::serializeCharPointer(char* val, int size) {}

void BasicSerializeOperator::serialize(std::string path) {
    int size = collector_->exportToBuffer(buffer_ + dataOffset_);
    std::ofstream fout(path, std::ios::binary);
    int len = dataOffset_ + size;
    fout.write((char*)&len, sizeof(int));
    fout.write(buffer_, len);
    fout.close();
}
