#include "PMSF.h"
#include "Handles.hh"
#include "handler/PropertyHandleWrapper.h"
#include "operator/CustomerUpdatePropertiesOperator.h"
#include "operator/DefaultUpdatePropertiesOperator.h"
#include <memory>

using epTargetPoints = OpenMesh::HPropHandleT<MCGAL::Point>;
using vpQuadrics = OpenMesh::VPropHandleT<int>;
using epError = OpenMesh::HPropHandleT<double>;
using epDirty = OpenMesh::HPropHandleT<bool>;

REGISTER_PROPERTY_HANDLE(epTargetPoints, Halfedge, MCGAL::Point);
REGISTER_PROPERTY_HANDLE(vpQuadrics, Vertex, int);
REGISTER_PROPERTY_HANDLE(epError, Halfedge, double);
REGISTER_PROPERTY_HANDLE(epDirty, Halfedge, bool);


void PMSF::RegisterProperties() {
    OpenMesh::VPropHandleT<int> m_vpQuadrics;
    OpenMesh::HPropHandleT<double> m_epError;
    OpenMesh::HPropHandleT<MCGAL::Point> m_epTargetPoints;
    OpenMesh::HPropHandleT<bool> m_epDirty;
    

    DefaultUpdatePropertiesOperator defaultUpdatePropertiesOperator;
    CustomerUpdatePropertiesOperator customerUpdatePropertiesOperator;
    std::unique_ptr<PropertyHandleWrapper> v1(new PropertyHandleWrapperImpl<OpenMesh::VPropHandleT<int>>(m_vpQuadrics));
    std::unique_ptr<PropertyHandleWrapper> h1(new PropertyHandleWrapperImpl<OpenMesh::HPropHandleT<double>>(m_epError));
    std::unique_ptr<PropertyHandleWrapper> h2(new PropertyHandleWrapperImpl<OpenMesh::HPropHandleT<MCGAL::Point>>(m_epTargetPoints));
    std::unique_ptr<PropertyHandleWrapper> h3(new PropertyHandleWrapperImpl<OpenMesh::HPropHandleT<bool>>(m_epDirty));
    std::vector<std::unique_ptr<PropertyHandleWrapper>> handles1;
    std::vector<std::unique_ptr<PropertyHandleWrapper>> handles2;
    handles1.push_back(std::move(v1));
    handles1.push_back(std::move(h1));
    handles2.push_back(std::move(h2));
    handles2.push_back(std::move(h3));
    customerUpdatePropertiesOperator.updatePropertity(0, nullptr, handles1);
    defaultUpdatePropertiesOperator.updatePropertity(0, nullptr, handles2);
}