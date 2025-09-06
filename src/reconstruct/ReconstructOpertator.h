/*
 * @Author: koi
 * @Date: 2025-09-02 15:45:23
 * @Description:
 */
#ifndef Reconstruct_OPERATOR_H
#define Reconstruct_OPERATOR_H

#include "Halfedge.h"
#include "Mesh.h"
#include "Vertex.h"
#include <memory>
#include <vector>

class ReconstructOperator {
  public:
    ReconstructOperator() = default;

    ~ReconstructOperator() = default;


    void init(std::shared_ptr<MCGAL::Mesh> mesh) {
      this->mesh_ = mesh;
    };

    virtual void reconstruct(MCGAL::Halfedge* halfedge) = 0;

    virtual void reconstruct(std::vector<MCGAL::Vertex*> vertices, std::vector<MCGAL::Halfedge*>& halfedge, std::vector<MCGAL::Facet*>& facets) = 0;

    std::shared_ptr<MCGAL::Mesh> mesh_;
};

#endif