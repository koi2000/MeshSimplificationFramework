/*
 * @Author: koi
 * @Date: 2025-09-19 15:46:28
 * @Description: 
 */

#include "SegmentationSerializeOperator.h"
#include "../common/BufferUtils.h"
#include "Halfedge.h"
#include "Point.h"
#include <cstdint>
#include <fstream>
#include <string>

void SegmentationSerializeOperator::initBuffer(int size) {
    buffer_ = new char[size];
}

void SegmentationSerializeOperator::serializeInt(int val) {
    writeInt(buffer_, dataOffset_, val);
}

void SegmentationSerializeOperator::serializeBaseMesh(std::shared_ptr<MCGAL::Mesh> mesh) {}

void SegmentationSerializeOperator::serializeBaseMeshWithSeed(std::shared_ptr<MCGAL::Mesh> mesh, std::vector<MCGAL::Halfedge*> seeds) {
    mesh->garbage_collection();
    
}

void SegmentationSerializeOperator::serialize(std::string path) {
    
}
