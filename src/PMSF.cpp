/*
 * @Author: koi
 * @Date: 2025-08-25 17:50:39
 * @Description:
 */
#include "PMSF.h"
#include "Global.h"
#include "Halfedge.h"
#include "Mesh.h"
#include "compress/BasicSerializeOperator.h"
#include "compress/BasicSymbolCollectOperator.h"
#include "decompress/DeserializeOperator.h"
#include "handler/PropertyHandleWrapper.h"
#include "operator/CustomerUpdatePropertiesOperator.h"
#include "operator/DefaultUpdatePropertiesOperator.h"
#include "operator/SeedIsRemovableOperator.h"
#include "selection/SelectOperator.h"
#include <iostream>
#include <memory>
#include <string>

#define DEBUG

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
    std::shared_ptr<SymbolCollectOperator> collector = std::make_shared<BasicSymbolCollectOperator>();
    std::shared_ptr<SerializeOperator> serializer =
        std::make_shared<BasicSerializeOperator>(collector, options.isEnablePrediction(), options.isEnableQuantization(), options.isEnableCompress());
    std::shared_ptr<IsRemovableOperator> seedIsRemovableOperator = std::make_shared<SeedIsRemovableOperator>();

    // set seed
    MCGAL::Halfedge* seed = nullptr;
    select->init(mesh);

    eliminate->init(mesh);
    collector->init(mesh);
    select->select(seed);
    seedIsRemovableOperator->init(nullptr, seed, nullptr);
    select->addIsRemovableOperator(seedIsRemovableOperator);
    int count = 0;
#ifdef DEBUG
    std::cout << count << std::endl;
    count = 0;
    std::string outpath = "./mesh_origin.off";
    mesh->dumpto_oldtype(outpath);
    mesh->resetState();
#endif
    for (int i = 0; i < options.getRound(); i++) {
        MCGAL::Halfedge* candidate = nullptr;
        while (select->select(candidate)) {
            if (eliminate->eliminate(candidate)) {
                count++;
            }
        }
        collector->collect(seed);
        select->reset();
#ifdef DEBUG
        std::cout << count << std::endl;
        count = 0;
        std::string outpath = "./mesh_" + std::to_string(i) + ".off";
        mesh->dumpto_oldtype(outpath);
        // mesh->resetState();
#endif
    }
    serializer->initBuffer(BUFFER_SIZE);
    serializer->serializeInt(options.getRound());
    serializer->serializeBaseMeshWithSeed(mesh, seed);
    serializer->serialize(options.getOutputPath());
}

void PMSF::decompress(DecompressOptions& options) {
    std::shared_ptr<DeserializeOperator> deserializeOperator = options.getDeserializeOperator();
    deserializeOperator->init(options.getPath());
    int round;
    deserializeOperator->deserializeInt(round);
    deserializeOperator->deserializeBaseMesh();
    while (round--) {
        std::shared_ptr<MCGAL::Mesh> mesh = deserializeOperator->exportMesh();

        std::string outpath = "./demesh_" + std::to_string(round) + ".off";
        mesh->dumpto_oldtype(outpath);
        mesh->resetState();

        deserializeOperator->deserializeOneRound();
    }
    std::string outpath = "./demesh_fin.off";
    deserializeOperator->exportMesh()->dumpto_oldtype(outpath);
}