/*
 * @Author: koi
 * @Date: 2025-09-01 09:36:36
 * @Description:
 */

#ifndef SEGMENTATION_SYMBOLREADOPERATOR_H
#define SEGMENTATION_SYMBOLREADOPERATOR_H

#include "SymbolReadOperator.h"
#include "Vertex.h"
#include <memory>
#include <vector>

class SegmentationSymbolReadOperator : public SymbolReadOperator {
  public:
    SegmentationSymbolReadOperator() = default;

    void collect(MCGAL::Halfedge* seed,
                 char* buffer,
                 int& dataOffset,
                 std::vector<MCGAL::Vertex*>& vertices,
                 std::vector<MCGAL::Halfedge*>& halfedge,
                 std::vector<MCGAL::Facet*>& facets);

    void collect(std::vector<MCGAL::Halfedge*> seeds,
                 char* buffer,
                 int& dataOffset,
                 std::vector<MCGAL::Vertex*>& vertices,
                 std::vector<MCGAL::Halfedge*>& halfedge,
                 std::vector<MCGAL::Facet*>& facets);

  private:
    // void collectHalfedgeSymbol(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed);
    // void collectFacetSymbol(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed);
};

#endif