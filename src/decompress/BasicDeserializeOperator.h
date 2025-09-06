/*
 * @Author: koi
 * @Date: 2025-09-02 10:39:10
 * @Description: 
 */
#ifndef BASIC_DESERIALIZE_OPERATOR_H
#define BASIC_DESERIALIZE_OPERATOR_H

#include "DeserializeOperator.h"
#include "Halfedge.h"
#include "Mesh.h"
#include "decompress/SymbolReadOperator.h"
#include <cstddef>
#include <memory>

class BasicDeserializeOperator : public DeserializeOperator {
  public:
    BasicDeserializeOperator(std::shared_ptr<SymbolReadOperator> symbolReadOperator, std::shared_ptr<ReconstructOperator> reconstructOperator)
        : DeserializeOperator(symbolReadOperator, reconstructOperator) {}
    
    ~BasicDeserializeOperator() = default;

    std::shared_ptr<MCGAL::Mesh> deserializeBaseMesh();

    std::shared_ptr<MCGAL::Mesh> exportMesh();
    
    void deserializeOneRound();

    MCGAL::Halfedge* seed = nullptr;
};

#endif