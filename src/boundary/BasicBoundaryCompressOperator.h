/*
 * @Author: koi
 * @Date: 2025-09-23 20:24:20
 * @Description: 
 */

#ifndef BASIC_BOUNDARY_COMPRESS_OPERATOR_H
#define BASIC_BOUNDARY_COMPRESS_OPERATOR_H

#include "Halfedge.h"
#include "Mesh.h"
#include "common/Graph.h"
#include <memory>
#include "BoundaryCompressOperator.h"

class BasicBoundaryCompressOperator : public BoundaryCompressOperator {
  public:
    BasicBoundaryCompressOperator() = default;

    ~BasicBoundaryCompressOperator() = default;

    bool compress_boundary();

    bool compress_group(int groupId);
};

#endif