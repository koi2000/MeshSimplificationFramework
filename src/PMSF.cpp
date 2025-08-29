/*
 * @Author: koi
 * @Date: 2025-08-25 17:50:39
 * @Description:
 */
#include "PMSF.h"
#include "Global.h"
#include "Halfedge.h"
#include "Mesh.h"
#include "handler/PropertyHandleWrapper.h"
#include "operator/CustomerUpdatePropertiesOperator.h"
#include "operator/DefaultUpdatePropertiesOperator.h"
#include "selection/SelectOperator.h"
#include <iostream>
#include <memory>
#include <string>

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
void PMSF::compress(CompressOptions& options) {
    MCGAL::contextPool.initPoolSize(1);

    std::shared_ptr<MCGAL::Mesh> mesh = std::make_shared<MCGAL::Mesh>(MCGAL::Mesh(options.getPath()));
    std::shared_ptr<SelectOperator> select = options.getSelect();
    std::shared_ptr<EliminateOperator> eliminate = options.getEliminate();
    select->init(mesh);
    eliminate->init(mesh);
    int count = 0;
    for (int i = 0; i < options.getRound(); i++) {
        MCGAL::Halfedge* candidate = nullptr;

        while (select->select(candidate)) {
            if (count == 166 && i == 6) {
                int i = 0;
            }
            eliminate->eliminate(candidate);
            count++;
        }
        select->reset();
        std::cout << count << std::endl;
        count = 0;
        std::string outpath = "./mesh_" + std::to_string(i) + ".off";
        mesh->dumpto_oldtype(outpath);
        mesh->resetState();
    }
}