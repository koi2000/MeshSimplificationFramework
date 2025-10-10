/*
 * @Author: koi
 * @Date: 2025-09-02 10:39:10
 * @Description: 
 */
#ifndef SEGMENTATION_DESERIALIZE_OPERATOR_H
#define SEGMENTATION_DESERIALIZE_OPERATOR_H

#include "DeserializeOperator.h"
#include "Halfedge.h"
#include "Mesh.h"
#include "decompress/SymbolReadOperator.h"
#include <cstddef>
#include <memory>
#include <vector>

class SegmentationDeserializeOperator : public DeserializeOperator {
  public:
  SegmentationDeserializeOperator(std::shared_ptr<SymbolReadOperator> symbolReadOperator, std::shared_ptr<ReconstructOperator> reconstructOperator)
        : DeserializeOperator(symbolReadOperator, reconstructOperator) {}
    
    ~SegmentationDeserializeOperator() = default;

    std::shared_ptr<MCGAL::Mesh> deserializeBaseMesh();

    std::shared_ptr<MCGAL::Mesh> exportMesh();
    
    void deserializeOneRound();

    std::vector<MCGAL::Halfedge*> seeds;
    std::vector<int> groupOffset;
};

#endif