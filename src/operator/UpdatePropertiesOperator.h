#ifndef UPDATEPROPERTITY_H
#define UPDATEPROPERTITY_H

#include "core.h"
#include "symetric_matrix.h"

class UpdatePropertiesOperator {
  public:
    UpdatePropertiesOperator();
    ~UpdatePropertiesOperator();

    void updatePropertity(int meshId, MCGAL::Halfedge* halfedge);

  private:
    OpenMesh::VPropHandleT<SymetricMatrix> m_vpQuadrics;
    OpenMesh::HPropHandleT<double> m_epError;
    OpenMesh::HPropHandleT<MCGAL::Point> m_epTargetPoints;
    OpenMesh::HPropHandleT<bool> m_epDirty;
};

#endif