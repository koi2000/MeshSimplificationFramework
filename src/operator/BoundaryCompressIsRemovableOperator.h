/*
 * @Author: koi
 * @Date: 2025-09-23 20:55:06
 * @Description: 
 */
/*
 * @Author: koi
 * @Date: 2025-09-20 11:51:21
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-09-03 21:29:50
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-08-25 22:29:32
 * @Description:
 */
#ifndef BOUNDARYCOMPRESS_ISREOVABLEOPERATOR_H
#define BOUNDARYCOMPRESS_ISREOVABLEOPERATOR_H

#include "Facet.h"
#include "Halfedge.h"
#include "Vertex.h"
#include "operator/IsRemovableOperator.h"
#include <cstddef>
#include <set>
#include <vector>

class BoundaryCompressIsRemovableOperator : public IsRemovableOperator {
  public:
  BoundaryCompressIsRemovableOperator() = default;

    void init(std::vector<MCGAL::Vertex*> v, std::vector<MCGAL::Halfedge*> h, std::vector<MCGAL::Facet*> f){
        this->seeds = h;
        triPoints = std::set<MCGAL::Vertex*>(v.begin(), v.end());
    };

    bool isRemovable(MCGAL::Halfedge* halfedge);

    bool willViolateManifold(const std::vector<MCGAL::Halfedge*>& polygon);

    std::vector<MCGAL::Halfedge*> seeds;
    std::set<MCGAL::Vertex*> triPoints;
};

#endif