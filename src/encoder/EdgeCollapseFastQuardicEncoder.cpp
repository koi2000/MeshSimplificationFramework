#include "EdgeCollapseFastQuardicEncoder.h"
#include "BFSVersionManager.h"
#include "MeshUtils.h"

void EdgeCollapseFastQuardicEncoder::encode(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed, Graph* graph, std::set<MCGAL::Vertex*>* triPoints, int groupId, bool encode_boundry) {
    this->mesh = mesh;
    meshId = mesh->meshId();
    this->seed = seed;
    this->graph = graph;
    this->triPoints = triPoints;
    encodeInsideOp(mesh, seed);
    if (encode_boundry) {
        encodeBoundaryOp(groupId);
    }
}

bool EdgeCollapseFastQuardicEncoder::encodeInsideOp(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed) {
    // 做一轮bfs来收集所有涉及到的边
    std::queue<int> gateQueue;
    gateQueue.push(seed->poolId());
    int current_version = MCGAL::BfsVersionMananger::current_version++;
    std::vector<MCGAL::Halfedge*> halfedges;
    halfedges.reserve(mesh->nb_vertices / 10);
    while (!gateQueue.empty()) {
        int hid = gateQueue.front();
        MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndex(meshId, hid);
        gateQueue.pop();

        if (h->isVisited(current_version) || h->isRemoved()) {
            continue;
        }
        h->setVisited(current_version);
        halfedges.push_back(h);

        for (MCGAL::Halfedge* h : h->end_vertex()->halfedges()) {
            if (!h->isVisited(current_version)) {
                gateQueue.push(h->poolId());
            }
        }
    }

    // Initialize();

    for (int iteration = 0; iteration < 100; iteration++) {
        double dThreshold = 0.000000001 * pow(double(iteration + 3), 7);

        for (int i = 0; i < halfedges.size(); i++) {
            MCGAL::Halfedge* eIt = halfedges[i];
            if (!eIt->isRemoved()) {
                MCGAL::contextPool.property(mesh->meshId(), m_epDirty, eIt) = false;
            }
        }

        for (int i = 0; i < halfedges.size(); i++) {
            MCGAL::Halfedge* h0 = halfedges[i];
            MCGAL::Vertex* v0 = h0->vertex();
            MCGAL::Vertex* v1 = h0->end_vertex();
            if (h0->isRemoved()) {
                continue;
            }
            if (MCGAL::contextPool.property(mesh->meshId(), m_epError, h0) > dThreshold)
                continue;
            if (MCGAL::contextPool.property(mesh->meshId(), m_epDirty, h0))
                continue;

            MCGAL::Point ptTarget = MCGAL::contextPool.property(mesh->meshId(), m_epTargetPoints, h0);
            if (MCGAL::IsFlipped(h0, ptTarget)) {
                continue;
            }
            if (IsFlipped(h0->opposite(), ptTarget)) {
                continue;
            }
            if(!isRemovable(h0)) {
                continue;
            }
            MCGAL::Halfedge* h1 = h0->opposite();
            // seed不能被压缩
            if (h0 == seed || h0->next() == seed || h0->next()->next() == seed || h0->opposite() == seed || h0->opposite()->next() == seed || h0->opposite()->next()->next() == seed) {
                continue;
            }
            if (h0 != seed && h0->opposite() != seed && mesh->is_collapse_ok(h0)) {
                MCGAL::Vertex* v = mesh->halfedge_collapse(h0, ptTarget);
                // std::string path = "./tp1/res" + std::to_string(v->vsplitNode()->order) + ".off";
            } else {
                continue;
            }

            auto& v0Quadric = MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, v0);
            auto& v1Quadric = MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, v1);
            MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, v1) += v0Quadric;
            MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, v0) += v1Quadric;
            // UpdateEdgePropertyAroundV(v1);
            // UpdateEdgePropertyAroundV(v0);
            updatePropertiesOperator->updatePropertity(meshId, h0);
        }
    }
}

bool EdgeCollapseFastQuardicEncoder::isRemovable(MCGAL::Halfedge* hit) {
    if (hit->isRemoved() || hit->vertex()->isRemoved() || hit->end_vertex()->isRemoved() || hit->vertex()->isProcessed() || hit->end_vertex()->isProcessed()) {
        return false;
    }
    if(hit->vertex()->isBoundary() || hit->end_vertex()->isBoundary()) {
        return false;
    }
    if (hit->isBoundary() || hit->next()->isBoundary() || hit->next()->next()->isBoundary()) {
        return false;
    }
    // if (!isFlipped(hit, hit->vertex()->point()) && !isFlipped(hit->opposite(), hit->end_vertex()->point()) && MCGAL::Mesh::is_collapse_ok(hit) && MCGAL::Mesh::is_collapse_ok(hit->opposite())) {
    //     return true;
    // }
    return false;
}

void EdgeCollapseFastQuardicEncoder::encodeBoundaryOp(int groupId) {
    // Implement boundary encoding logic here
    // This function should handle the encoding of boundary edges and faces
    // based on the groupId provided.
    std::unordered_map<int, std::vector<Node>>& nodes = graph->getNode(groupId);
    for (auto& [neighbourId, nodeArr] : nodes) {
        for (auto& node : nodeArr) {
            if (node.isVisiable()) {
                continue;
            }
            node.setVisiable();
            int poolId = node.st;
            MCGAL::Vertex* vt = MCGAL::contextPool.getVertexByIndex(meshId, poolId);
            MCGAL::Halfedge* boundary = nullptr;
            for (MCGAL::Halfedge* hit : vt->halfedges()) {
                if (hit->isBoundary()) {
                    if (hit->end_vertex()->poolId() == node.ed) {
                        boundary = hit;
                        break;
                    }
                }
            }
            assert(boundary != nullptr);
            if (boundary == nullptr) {
                continue;
                ;
            }
            std::unordered_map<int, std::vector<Node>>& neighbour_nodes = graph->getNode(neighbourId);
            for (auto& [key, nds] : neighbour_nodes) {
                if (key == groupId) {
                    for (auto& nd : nds) {
                        if (nd.st == node.stop) {
                            nd.setVisiable();
                            break;
                        }
                    }
                }
            }
            int stopId = node.stop;
            // NEW_boundary 可能有问题
            int cnt = 0;
            do {
                MCGAL::Halfedge* nxt = MCGAL::next_boundary(neighbourId, boundary);
                if (boundaryRemovableInEdgeCollapse(groupId, neighbourId, boundary)) {
                    // MCGAL::Point p = (boundary->vertex()->point() + boundary->end_vertex()->point()) / 2;
                    MCGAL::Point p = boundary->vertex()->point();
                    MCGAL::Vertex* v = mesh->halfedge_collapse(boundary, p);
                    for (MCGAL::Halfedge* h : v->halfedges()) {
                        if (!h->isBoundary() && h->face()->groupId() != h->opposite()->face()->groupId()) {
                            h->setBoundary();
                            h->opposite()->setBoundary();
                        }
                        if (h->isBoundary() && h->face()->groupId() == h->opposite()->face()->groupId()) {
                            h->setNotBoundary();
                            h->opposite()->setNotBoundary();
                        }
                    }
                }
                if (boundary->end_vertex()->poolId() == stopId) {
                    break;
                }
                boundary = nxt;
                // std::string path1 = "./" + std::to_string(cnt) + "_" + std::to_string(groupId) + ".off";
                // std::string path2 = "./" + std::to_string(cnt) + "_" + std::to_string(neighbourId) + ".off";
                // mesh->submesh_dumpto_oldtype(path1, groupId);
                // mesh->submesh_dumpto_oldtype(path2, neighbourId);

                assert(boundary != nullptr);
                // if (boundary == nullptr)
                //     break;
                cnt++;
            } while (boundary->end_vertex()->poolId() != stopId);
        }
    }
}

bool EdgeCollapseFastQuardicEncoder::boundaryRemovableInEdgeCollapse(int inner, int outer, MCGAL::Halfedge* hit) {
    // assert(hit->isBoundary());
    if (triPoints->count(hit->vertex()) || triPoints->count(hit->end_vertex())) {
        return false;
    }
    // if (hit->isBoundary() && hit->next()->isBoundary()) {
    //     if (hit->hasRemovedVertex() || hit->next()->hasRemovedVertex()) {
    //         return false;
    //     }
    // }
    if (hit->isRemoved() || hit->vertex()->isRemoved() || hit->end_vertex()->isRemoved()) {
        return false;
    }
    if (MCGAL::Mesh::is_collapse_ok(hit) && MCGAL::Mesh::is_collapse_ok(hit->opposite())) {
        return true;
    }

    return false;
}