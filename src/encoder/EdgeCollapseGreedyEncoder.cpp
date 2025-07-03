#include "EdgeCollapseGreedyEncoder.h"
#include "MeshUtils.h"


void EdgeCollapseGreedyEncoder::encode(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed, Graph* graph, std::set<MCGAL::Vertex*>* triPoints, int groupId, bool encode_boundry) {
    this->mesh = mesh;
    meshId = mesh->meshId();
    this->seed = seed;
    this->graph = graph;
    this->triPoints = triPoints;
    // if (groupId == 0 || groupId == 37) {
    //     std::string path = "./before" + std::to_string(groupId) + ".off";
    //     mesh->submesh_dumpto_oldtype(path, groupId);
    // }
    encodeInsideOp(mesh, seed);
    // if (groupId == 0 || groupId == 37) {
    //     std::string path = "./after" + std::to_string(groupId) + ".off";
    //     mesh->submesh_dumpto_oldtype(path, groupId);
    // }
    if (encode_boundry) {
        encodeBoundaryOp(groupId);
    }
}

bool EdgeCollapseGreedyEncoder::encodeInsideOp(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed) {
    std::queue<int> gateQueue;
    gateQueue.push(seed->poolId());
    int removedCount = 0;
    std::set<int> boundaryIds;
    while (!gateQueue.empty()) {
        int hid = gateQueue.front();
        MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndex(meshId, hid);
        gateQueue.pop();

        if (h->isProcessed() || h->isRemoved()) {
            continue;
        }
        h->setProcessed();

        if (isRemovable(h) && !h->isBoundary() && !h->opposite()->isBoundary()) {
            removedCount++;
            edgeCut(mesh, boundaryIds, gateQueue, h);
        } else {
            // MCGAL::Halfedge* hh = h;
            // do {
            //     // hh->vertex()->setConquered();
            //     MCGAL::Halfedge* hOpp = hh->opposite();
            //     if (hOpp != nullptr && hOpp->isProcessed() && !hOpp->isBoundary() && !hh->isBoundary()) {
            //         gateQueue.push(hOpp->poolId());
            //     }
            // } while ((hh = hh->next()) != h);
            for (MCGAL::Halfedge* h : h->end_vertex()->halfedges()) {
                if (!h->isProcessed()) {
                    gateQueue.push(h->poolId());
                }
            }
        }
    }
    resetBfsState();
    return true;
}

void EdgeCollapseGreedyEncoder::encodeBoundaryOp(int groupId) {
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

bool EdgeCollapseGreedyEncoder::isRemovable(MCGAL::Halfedge* hit) {
    if (hit->isRemoved() || hit->vertex()->isRemoved() || hit->end_vertex()->isRemoved() || hit->vertex()->isProcessed() || hit->end_vertex()->isProcessed()) {
        return false;
    }
    if(hit->vertex()->isBoundary() || hit->end_vertex()->isBoundary()) {
        return false;
    }
    if (hit->isBoundary() || hit->next()->isBoundary() || hit->next()->next()->isBoundary()) {
        return false;
    }
    if (!isFlipped(hit, hit->vertex()->point()) && !isFlipped(hit->opposite(), hit->end_vertex()->point()) && MCGAL::Mesh::is_collapse_ok(hit) && MCGAL::Mesh::is_collapse_ok(hit->opposite())) {
        return true;
    }
    return false;
}

MCGAL::Halfedge* EdgeCollapseGreedyEncoder::edgeCut(MCGAL::Mesh* mesh, std::set<int>& boundaryIds, std::queue<int>& gateQueue, MCGAL::Halfedge* hit) {
    MCGAL::Vertex* v = mesh->halfedge_collapse(hit, hit->vertex()->point());
    for (MCGAL::Halfedge* h : v->halfedges()) {
        if (!h->isProcessed()) {
            gateQueue.push(h->poolId());
        }
    }
    return nullptr;
}

bool EdgeCollapseGreedyEncoder::isFlipped(MCGAL::Halfedge* edge, const MCGAL::Point& ptTarget) {
    MCGAL::Vertex* v0 = edge->vertex();
    MCGAL::Vertex* v1 = edge->end_vertex();
    for (MCGAL::Halfedge* h : v0->halfedges()) {
        if (h->face()->isRemoved()) {
            continue;
        }
        std::vector<MCGAL::Vertex*> vs;
        MCGAL::Halfedge* st = h->face()->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        do {
            vs.push_back(st->vertex());
            st = st->next();
        } while (st != ed);
        if (vs[0] == v1 || vs[1] == v1 || vs[2] == v1) {
            continue;
        }
        int idxV0 = 0;
        for (int i = 0; i < vs.size(); i++) {
            MCGAL::Vertex* v = vs[i];
            if (vs[i] == v0) {
                idxV0 = i;
            }
        }
        MCGAL::Point pt0 = v0->point();
        MCGAL::Point pt1 = vs[(idxV0 + 1) % 3]->point();
        MCGAL::Point pt2 = vs[(idxV0 + 2) % 3]->point();

        MCGAL::Point dir1 = pt1 - ptTarget;
        dir1.normalize();
        MCGAL::Point dir2 = pt2 - ptTarget;
        dir2.normalize();

        // The angle below can be adjusted, but now is enough
        // if the angle between dir1 and dir2 small than 2.6 angle, return true
        if (fabs(dir1.dot(dir2)) > 0.999)
            return true;
        MCGAL::Point normold;
        try {
            normold = h->face()->computeNormal();
        } catch (std::string exp) { return true; }
        MCGAL::Point norm = dir1.cross(dir2);
        norm.normalize();

        // if the angle between normold and norm large than 78.5 angle, return true
        if ((normold.dot(norm)) < 0.2)
            return true;
    }
    return false;
}

bool EdgeCollapseGreedyEncoder::boundaryRemovableInEdgeCollapse(int inner, int outer, MCGAL::Halfedge* hit) {
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

void EdgeCollapseGreedyEncoder::resetBfsState() {}

void EdgeCollapseGreedyEncoder::resetState() {}