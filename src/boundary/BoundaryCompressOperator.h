/*
 * @Author: koi
 * @Date: 2025-09-23 20:21:35
 * @Description:
 */
#ifndef BOUNDARY_COMPRESS_OPERATOR_H
#define BOUNDARY_COMPRESS_OPERATOR_H

#include "Halfedge.h"
#include "Mesh.h"
#include "common/Graph.h"
#include "elimination/EliminateOperator.h"
#include "operator/IsRemovableOperator.h"
#include <memory>
#include <vector>

class BoundaryCompressOperator {
  public:
    BoundaryCompressOperator() = default;

    virtual void init(std::shared_ptr<MCGAL::Mesh> mesh,
                      std::shared_ptr<Graph> g,
                      std::vector<std::shared_ptr<IsRemovableOperator>> isRemovableOperators,
                      std::shared_ptr<EliminateOperator> eliminateOperator) {
        this->mesh = mesh;
        this->graph = g;
        this->isRemovableOperators = isRemovableOperators;
        this->eliminateOperator = eliminateOperator;
    };

    virtual ~BoundaryCompressOperator() = default;

    virtual bool isRemovable(MCGAL::Halfedge* hit) {
        for (auto& op : isRemovableOperators) {
            if (!op->isRemovable(hit)) {
                return false;
            }
        }
        return true;
    }

    virtual bool compress_boundary() = 0;

    std::shared_ptr<MCGAL::Mesh> mesh;
    std::shared_ptr<Graph> graph;
    std::shared_ptr<EliminateOperator> eliminateOperator;
    std::vector<std::shared_ptr<IsRemovableOperator>> isRemovableOperators;
};

#endif