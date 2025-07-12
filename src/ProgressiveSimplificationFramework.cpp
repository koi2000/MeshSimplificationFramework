#include "ProgressiveSimplificationFramework.h"

ProgressiveSimplificationFramework::ProgressiveSimplificationFramework() {}

void ProgressiveSimplificationFramework::loadMesh(std::string path) {
    MCGAL::contextPool.initPoolSize(1);
    // MCGAL::contextPool.registerPool(mesh.nb_vertices, mesh.nb_faces * 3, mesh.nb_faces);

    mesh.loadOFF(path);
    partialSplitter.loadMesh(&mesh);
    partialSplitter.split(50);
    seeds = partialSplitter.exportSeeds();
    graph = partialSplitter.exportGraph();
    triPoints = partialSplitter.exportTriPoints();
}

void ProgressiveSimplificationFramework::encode(Options options) {
    const std::vector<std::pair<CompressType, LocalOperatorType>>& level_types = options.getLevelTypes();
    if(options.getRegisterPropertiesOperator()) {
        options.getRegisterPropertiesOperator()->registerProperties(mesh.meshId());
    }
    for (int i = 0; i < level_types.size(); i++) {
        switch (level_types[i].second) {
            case LocalOperatorType::VERTEX_REMOVAL: {
                if (level_types[i].first == CompressType::GREEDY) {
                    vertex_removal_greedy_encode();
                } else if (level_types[i].first == CompressType::FAST_QUADRIC) {
                }
                break;
            }
            case LocalOperatorType::EDGE_COLLAPSE: {
                options.getRegisterPropertiesOperator()->initProperties(&mesh);
                if (level_types[i].first == CompressType::GREEDY) {
                    edge_collapse_greedy_encode();
                } else if (level_types[i].first == CompressType::FAST_QUADRIC) {
                    edge_collapse_fast_quardic_encode();
                } else if (level_types[i].first == CompressType::PRIORITY_ERROR) {
                    edge_collapse_priority_error_encode();
                }
                break;
            }
            default: break;
        }
        
        resetState();
    }
    mesh.dumpto_oldtype("./encode.off");
}

void ProgressiveSimplificationFramework::resetState() {
    int meshId = mesh.meshId();
    for (int i = 0; i < MCGAL::contextPool.getVindex(meshId); i++) {
        MCGAL::Vertex* vertex = MCGAL::contextPool.getVertexByIndex(meshId, i);
        vertex->resetState();
    }
    for (int i = 0; i < MCGAL::contextPool.getFindex(meshId); i++) {
        MCGAL::Facet* face = MCGAL::contextPool.getFacetByIndex(meshId, i);
        face->resetState();
    }
    for (int i = 0; i < MCGAL::contextPool.getHindex(meshId); i++) {
        MCGAL::Halfedge* hit = MCGAL::contextPool.getHalfedgeByIndex(meshId, i);
        hit->resetState();
    }
}

void ProgressiveSimplificationFramework::vertex_removal_greedy_encode(int groupId) {
    vertexRemovalGreedyEncoder.encode(&mesh, seeds[groupId], &graph, &triPoints, groupId, true);
}

void ProgressiveSimplificationFramework::vertex_removal_greedy_encode() {
    for (int i = 0; i < seeds.size(); i++) {
        vertex_removal_greedy_encode(i);
    }
}

void ProgressiveSimplificationFramework::edge_collapse_greedy_encode() {
    for (int i = 0; i < seeds.size(); i++) {
        edge_collapse_greedy_encode(i);
    }
}

void ProgressiveSimplificationFramework::edge_collapse_fast_quardic_encode() {
    for (int i = 0; i < seeds.size(); i++) {
        edge_collapse_fast_quardic_encode(i);
    }
}

void ProgressiveSimplificationFramework::edge_collapse_priority_error_encode() {
    for (int i = 0; i < seeds.size(); i++) {
        edge_collapse_fast_quardic_encode(i);
    }
}

void ProgressiveSimplificationFramework::edge_collapse_greedy_encode(int groupId) {
    edgeCollapseGreedyEncoder.encode(&mesh, seeds[groupId], &graph, &triPoints, groupId, false);
}

void ProgressiveSimplificationFramework::edge_collapse_fast_quardic_encode(int groupId) {
    edgeCollapseFastQuardicEncoder.encode(&mesh, seeds[groupId], &graph, &triPoints, groupId, false);
}
void ProgressiveSimplificationFramework::edge_collapse_priority_error_encode(int groupId) {
    edgeCollapsePriorityErrorEncoder.encode(&mesh, seeds[groupId], &graph, &triPoints, groupId, false);
}

ProgressiveSimplificationFramework::~ProgressiveSimplificationFramework() {}

// void ProgressiveSimplificationFramework::dump2Buffer() {
//     std::vector<int> offsets(subMeshes.size(), 0);
//     writeInt(buffer, dataOffset, offsets.size());
//     dumpGraphToBuffer();
//     for (int i = 0; i < subMeshes.size(); i++) {
//         writeBaseMesh(i);
//     }
//     for (int i = 0; i < seeds.size(); i++) {
//         writeInt(buffer, dataOffset, seeds[i]->vertex->id);
//         writeInt(buffer, dataOffset, seeds[i]->end_vertex->id);
//     }
//     for (int i = 0; i < compressRounds.size(); i++) {
//         writeInt(buffer, dataOffset, compressRounds[i]);
//     }
//     int prevDataOffset = dataOffset;
//     dataOffset += offsets.size() * sizeof(int);
//     for (int i = 0; i < subMeshes.size(); i++) {
//         int prevOffset = dataOffset;
//         dumpGroupToBuffer(i);
//         int afterDataOffset = dataOffset;
//         offsets[i] = prevOffset;
//     }
//     for (int i = 0; i < offsets.size(); i++) {
//         writeInt(buffer, prevDataOffset, offsets[i]);
//     }
//     dumpToFile("./dragon_try.loc");
// }