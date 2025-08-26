#ifndef VERTEX_REMOVAL_PRIORITY_ERROR_ENCODER_H
#define VERTEX_REMOVAL_PRIORITY_ERROR_ENCODER_H

#include "core.h"
#include <string>

class VertexRemovalPriorityErrorEncoder {
  public:
    VertexRemovalPriorityErrorEncoder() {};

  private:


  private:
    int dataOffset = 0;
    MCGAL::Mesh mesh;
    char* buffer;
};

#endif