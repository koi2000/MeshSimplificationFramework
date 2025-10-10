/*
 * @Author: koi
 * @Date: 2025-09-01 20:09:27
 * @Description:
 */

#include "BasicSerializeOperator.h"
#include "../common/BufferUtils.h"
#include "../common/EntropyCodec.h"
#include "Halfedge.h"
#include "Point.h"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

void BasicSerializeOperator::initBuffer(int size) {
    buffer_ = new char[size];
}

void BasicSerializeOperator::serializeInt(int val) {
    writeInt(buffer_, dataOffset_, val);
}

void BasicSerializeOperator::serializeBaseMesh(std::shared_ptr<MCGAL::Mesh> mesh) {}

void BasicSerializeOperator::serializeBaseMeshWithSeed(std::shared_ptr<MCGAL::Mesh> mesh, std::vector<MCGAL::Halfedge*> seeds) {
    mesh->garbage_collection();
    unsigned i_nbVerticesBaseMesh = mesh->size_of_vertices();
    unsigned i_nbFacesBaseMesh = mesh->size_of_facets();

    if (enableQuantization_) {
        writeChar(buffer_, dataOffset_, mesh->i_nbQuantBits);
        writePoint(buffer_, dataOffset_, mesh->bboxMin);
        writeFloat(buffer_, dataOffset_, mesh->f_quantStep);
    }

    writeInt(buffer_, dataOffset_, i_nbVerticesBaseMesh);
    writeInt(buffer_, dataOffset_, i_nbFacesBaseMesh);
    int id = 0;
    unsigned i_bitOffset = 0;
    MCGAL::Halfedge* seed = seeds[0];
    seed->vertex()->setVid(id++);
    seed->end_vertex()->setVid(id++);
    if (enableQuantization_) {
        MCGAL::Point p = seed->vertex()->point();
        MCGAL::PointInt pi = mesh->floatPosToInt(p);
        for (unsigned i = 0; i < 3; ++i) {
            assert(pi[i] < 1 << mesh->i_nbQuantBits);
            writeBits((uint32_t)pi[i], mesh->i_nbQuantBits, buffer_, i_bitOffset, dataOffset_);
        }
        p = seed->end_vertex()->point();
        pi = mesh->floatPosToInt(p);
        for (unsigned i = 0; i < 3; ++i) {
            assert(pi[i] < 1 << mesh->i_nbQuantBits);
            writeBits((uint32_t)pi[i], mesh->i_nbQuantBits, buffer_, i_bitOffset, dataOffset_);
        }
    } else {
        writePoint(buffer_, dataOffset_, seed->vertex()->point());
        writePoint(buffer_, dataOffset_, seed->end_vertex()->point());
    }

    for (MCGAL::Vertex* vit : mesh->vertices()) {
        if (vit->isRemoved()) {
            continue;
        }
        if (vit->poolId() == seed->vertex()->poolId() || vit->poolId() == seed->end_vertex()->poolId()) {
            continue;
        }
        MCGAL::Point point = vit->point();
        if (enableQuantization_) {
            MCGAL::PointInt p = mesh->floatPosToInt(point);
            for (unsigned i = 0; i < 3; ++i) {
                assert(p[i] < 1 << mesh->i_nbQuantBits);
                writeBits(p[i], mesh->i_nbQuantBits, buffer_, i_bitOffset, dataOffset_);
            }
        } else {
            writePoint(buffer_, dataOffset_, point);
        }
        vit->setVid(id++);
    }
    // i_bitOffset = 0;
    // dataOffset_++;
    // char connBit = ceil(log2(i_nbVerticesBaseMesh + 1));
    // writeChar(buffer_, dataOffset_, connBit);
     for (MCGAL::Facet* fit : mesh->faces()) {
        if (fit->isRemoved()) {
            continue;
        }
        int i_faceDegree = 0;

        MCGAL::Halfedge* st = fit->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        // do {
        //     i_faceDegree++;
        //     st = st->next();
        // } while (st != ed);
        // writeInt(buffer_, dataOffset_, i_faceDegree);
        // writeBits(i_faceDegree, 2, buffer_, i_bitOffset, dataOffset_);
        do {
            writeInt(buffer_, dataOffset_, st->vertex()->vid());
            // writeBits((uint32_t)st->vertex()->vid(), connBit, buffer_, i_bitOffset, dataOffset_);
            st = st->next();
        } while (st != ed);
    }
}



void BasicSerializeOperator::serialize(std::string path) {
    int size = collector_->exportToBuffer(buffer_ + dataOffset_, enableQuantization_);
    char* outBlock = buffer_;
    int outBlockSize = dataOffset_ + size;
    if (enableCompress_) {
        serializeCharPointer(buffer_, dataOffset_ + size, outBlock, outBlockSize);
    }

    std::ofstream fout(path, std::ios::binary);
    int len = outBlockSize;
    fout.write((char*)&len, sizeof(int));
    fout.write(outBlock, len);
    fout.close();
}
