#include "EdgeCollapsePriorityErrorEncoder.h"
#include "BFSVersionManager.h"
#include "MeshUtils.h"
#include <queue>

void EdgeCollapsePriorityErrorEncoder::encode(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed, Graph* graph, std::set<MCGAL::Vertex*>* triPoints, int groupId, bool encode_boundry) {
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

bool EdgeCollapsePriorityErrorEncoder::encodeInsideOp(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed) {
    std::queue<int> gateQueue;
    gateQueue.push(seed->poolId());
    int current_version = MCGAL::BfsVersionMananger::current_version++;
    std::vector<MCGAL::Halfedge*> halfedges;
    halfedges.reserve(mesh->nb_vertices / 10);

    while (!gateQueue.empty()) {
        int hid = gateQueue.front();
        gateQueue.pop();
        MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndex(meshId, hid);
        if (h->isRemoved() || h->isVisited(current_version))
            continue;
        h->setVisited(current_version);
        halfedges.push_back(h);
        for (MCGAL::Halfedge* nh : h->end_vertex()->halfedges()) {
            if (!nh->isVisited(current_version)) {
                gateQueue.push(nh->poolId());
            }
        }
    }

    struct EdgeItem {
        MCGAL::Halfedge* edge;
        double error;
        bool operator<(const EdgeItem& other) const {
            return error > other.error;
        }
    };
    std::priority_queue<EdgeItem> edgeQueue;

    std::unordered_set<MCGAL::Halfedge*> seen;
    for (MCGAL::Halfedge* h : halfedges) {
        if (!h->isRemoved() && isRemovable(h)) {
            double err = MCGAL::contextPool.property(meshId, m_epError, h);
            edgeQueue.push({h, err});
            seen.insert(h);
        }
    }

    while (!edgeQueue.empty()) {
        EdgeItem item = edgeQueue.top();
        edgeQueue.pop();
        MCGAL::Halfedge* h = item.edge;

        if (h->isRemoved() || !isRemovable(h))
            continue;

        double currentErr = MCGAL::contextPool.property(meshId, m_epError, h);
        if (currentErr != item.error)
            continue;

        MCGAL::Point target = MCGAL::contextPool.property(meshId, m_epTargetPoints, h);
        if (MCGAL::IsFlipped(h, target) || IsFlipped(h->opposite(), target))
            continue;

        if (h == seed || h->next() == seed || h->next()->next() == seed || h->opposite() == seed || h->opposite()->next() == seed || h->opposite()->next()->next() == seed) {
            continue;
        }

        if (!mesh->is_collapse_ok(h))
            continue;

        MCGAL::Vertex* v = mesh->halfedge_collapse(h, target);
        if (!v)
            continue;

        h->vertex()->setProcessed();
        h->end_vertex()->setProcessed();
        v->setProcessed();

        MCGAL::Vertex* v0 = h->vertex();
        MCGAL::Vertex* v1 = h->vertex();
        auto& v0Quadric = MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, v0);
        auto& v1Quadric = MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, v1);
        MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, v1) += v0Quadric;
        MCGAL::contextPool.property(mesh->meshId(), m_vpQuadrics, v0) += v1Quadric;
        updatePropertiesOperator->updatePropertity(meshId, h);
        // 更新邻边误差
        for (MCGAL::Halfedge* nh : v->halfedges()) {
            if (!nh || nh->isRemoved())
                continue;
            if (isRemovable(nh)) {
                double newErr = MCGAL::contextPool.property(meshId, m_epError, nh);
                edgeQueue.push({nh, newErr});
            }
        }
    }

    return true;
}

bool EdgeCollapsePriorityErrorEncoder::isRemovable(MCGAL::Halfedge* hit) {
    if (hit->isRemoved() || hit->vertex()->isRemoved() || hit->end_vertex()->isRemoved() || hit->vertex()->isProcessed() || hit->end_vertex()->isProcessed()) {
        return false;
    }
    if (hit->vertex()->isBoundary() || hit->end_vertex()->isBoundary()) {
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

void EdgeCollapsePriorityErrorEncoder::encodeBoundaryOp(int groupId) {
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

bool EdgeCollapsePriorityErrorEncoder::boundaryRemovableInEdgeCollapse(int inner, int outer, MCGAL::Halfedge* hit) {
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