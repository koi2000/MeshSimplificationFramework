#include "VertexRemovalGreedyEncoder.h"
#include "../common/BufferUtils.h"
#include "../common/Logutil.h"
#include "BFSVersionManager.h"
#include "MeshUtils.h"
#include "biops.h"
#include <fstream>
#include <iostream>
#include <map>
#define DEBUG_GROUPID 20

VertexRemovalGreedyEncoder::VertexRemovalGreedyEncoder() {}

void VertexRemovalGreedyEncoder::encode(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed, Graph* graph, std::set<MCGAL::Vertex*>* triPoints, int groupId, bool encode_boundry) {
    meshId = mesh->meshId();
    this->seed = seed;
    this->graph = graph;
    this->triPoints = triPoints;
    this->groupId = groupId;
    encodeInsideOp(mesh, seed);
    if (encode_boundry) {
        encodeBoundaryOp(groupId);
    }
}

bool VertexRemovalGreedyEncoder::encodeInsideOp(MCGAL::Mesh* mesh, MCGAL::Halfedge* seed) {
    std::queue<int> gateQueue;
    gateQueue.push(seed->poolId());
    int removedCount = 0;
    std::set<int> boundaryIds;
    while (!gateQueue.empty()) {
        int hid = gateQueue.front();
        MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndex(meshId, hid);
        gateQueue.pop();
        MCGAL::Facet* f = h->face();
        if (f->isConquered()) {
            continue;
        }
        bool hasRemovable = false;
        MCGAL::Halfedge* unconqueredVertexHE;
        for (MCGAL::Halfedge* hh = h->next(); hh != h; hh = hh->next()) {
            if (isRemovable(hh->end_vertex()) && !hh->isBoundary() && !hh->opposite()->isBoundary()) {
                hasRemovable = true;
                unconqueredVertexHE = hh;
                break;
            }
        }
        if (!hasRemovable) {
            f->setUnsplittable();
            MCGAL::Halfedge* hh = h;
            do {
                // hh->vertex()->setConquered();
                MCGAL::Halfedge* hOpp = hh->opposite();
                if (hOpp != nullptr && !hOpp->isBoundary() && !hh->isBoundary()) {
                    gateQueue.push(hOpp->poolId());
                }
            } while ((hh = hh->next()) != h);
        } else {
            removedCount++;
            vertexCut(mesh, boundaryIds, gateQueue, unconqueredVertexHE);
        }
    }
    encodeSymbolOp();
    return true;
}

// void VertexRemovalGreedyEncoder::encodeBoundaryOp(int groupId) {
//     std::unordered_map<int, std::vector<Node>>& nodes = graph->getNode(groupId);
//     for (auto& [neighbourId, nodeArr] : nodes) {
//         for (auto& node : nodeArr) {
//             if (node.isVisiable()) {
//                 continue;
//             }
//             node.setVisiable();
//             int poolId = node.st;
//             MCGAL::Vertex* vt = MCGAL::contextPool.getVertexByIndex(meshId, poolId);
//             MCGAL::Halfedge* boundary = nullptr;
//             for (MCGAL::Halfedge* hit : vt->halfedges()) {
//                 if (hit->isBoundary() && hit->end_vertex()->poolId() == node.ed) {
//                     boundary = hit;
//                     break;
//                 }
//             }
//             std::unordered_map<int, std::vector<Node>>& neighbour_nodes = graph->getNode(neighbourId);
//             for (auto& [key, nds] : neighbour_nodes) {
//                 if (key == groupId) {
//                     for (auto& nd : nds) {
//                         if (nd.st == node.stop) {
//                             nd.setVisiable();
//                             break;
//                         }
//                     }
//                 }
//             }
//             int stopId = node.stop;
//             do {
//                 if (boundaryRemovableInVertexRemoval(groupId, neighbourId, boundary)) {
//                     if (boundary->next()->isBoundary()) {
//                         int oriRemovedVid = 0;
//                         MCGAL::Vertex* v1 = nullptr;
//                         MCGAL::Vertex* v2 = nullptr;
//                         MCGAL::Halfedge* new_boundary = nullptr;
//                         if (boundary->face()->facet_degree() == 3) {
//                             // 将这个面的groupId设置为opposite的groupId
//                             boundary->face()->setGroupId(boundary->opposite()->face()->groupId());
//                             // 取消旧边界
//                             boundary->setNotBoundary();
//                             boundary->opposite()->setNotBoundary();
//                             boundary->next()->setNotBoundary();
//                             boundary->next()->opposite()->setNotBoundary();
//                             // 设置新边界
//                             new_boundary = boundary->next()->next()->opposite();
//                             new_boundary->setRemovedVertex(boundary->end_vertex()->pointptr());
//                             new_boundary->setBoundary();
//                             new_boundary->opposite()->setBoundary();
//                         } else if (boundary->face()->facet_degree() > 3) {
//                             // } else if (false) {
//                             // 新建一个面
//                             new_boundary = MCGAL::contextPool.allocateHalfedgeFromPool(meshId, boundary->vertex(), boundary->next()->end_vertex());
//                             MCGAL::Halfedge* new_boundary_opposite = MCGAL::contextPool.allocateHalfedgeFromPool(meshId, boundary->next()->end_vertex(), boundary->vertex());
//                             // 取消旧边界
//                             boundary->setNotBoundary();
//                             boundary->opposite()->setNotBoundary();
//                             boundary->next()->setNotBoundary();
//                             boundary->next()->opposite()->setNotBoundary();
//                             MCGAL::Halfedge* boundary_prev = find_prev(boundary);
//                             boundary_prev->setNext(new_boundary);
//                             new_boundary->setNext(boundary->next()->next());
//                             new_boundary_opposite->setNext(boundary);
//                             boundary->next()->setNext(new_boundary_opposite);
//                             // 申请新面
//                             MCGAL::Facet* new_face = MCGAL::contextPool.allocateFaceFromPool(meshId, new_boundary_opposite);
//                             new_face->setGroupId(boundary->opposite()->face()->groupId());
//                             // 从大的多边形中移除小的多边形
//                             new_boundary->setRemovedVertex(boundary->end_vertex()->pointptr());
//                             new_boundary->setBoundary();
//                             new_boundary->opposite()->setBoundary();
//                             // 减除一个三角形
//                             boundary_prev->face()->reset(boundary_prev);
//                         }
//                         // 获得对面的groupId
//                         MCGAL::Halfedge* h = boundary->opposite();
//                         MCGAL::Halfedge* end(h);
//                         int removed = 0;
//                         do {
//                             MCGAL::Facet* f = h->face();
//                             assert(!f->isConquered() && !f->isRemoved());
//                             if (f->facet_degree() > 3) {
//                                 MCGAL::Halfedge* hSplit(h->next());
//                                 for (; hSplit->next()->next() != h; hSplit = hSplit->next())
//                                     ;
//                                 MCGAL::Halfedge* hCorner = mesh->split_facet_non_meshId(h, hSplit);
//                                 // hCorner->setBoundaryAdded();
//                             }
//                             h->end_vertex()->setConquered();
//                             removed++;
//                         } while ((h = h->opposite()->next()) != end);
//                         MCGAL::Halfedge* newH = mesh->erase_center_vertex(boundary);
//                         newH->face()->setGroupId(neighbourId);
//                         boundary = new_boundary;
//                     }
//                 }
//                 if (boundary->end_vertex()->poolId() == stopId) {
//                     break;
//                 }
//                 boundary = next_boundary(neighbourId, boundary);
//             } while (boundary->end_vertex()->poolId() != stopId);
//         }
//     }
// }

void VertexRemovalGreedyEncoder::encodeBoundaryOp(int groupId) {
    // vertex removal
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
            do {
                if (boundaryRemovableInVertexRemoval(groupId, neighbourId, boundary)) {
                    MCGAL::Halfedge* next_boundary = MCGAL::next_boundary(neighbourId, boundary);
                    // try use split facet to triangulation and use vertex removal to compress
                    MCGAL::Vertex* v = boundary->end_vertex();
                    assert(v->vertex_degree() > 2);
                    MCGAL::Halfedge* h = boundary->opposite();
                    MCGAL::Halfedge* end(h);
                    // 遍历找到两个点之间的距离
                    MCGAL::Vertex* v1 = boundary->vertex();
                    MCGAL::Vertex* v2 = next_boundary->end_vertex();
                    MCGAL::Halfedge* st = boundary->opposite();
                    MCGAL::Halfedge* ed = st;
                    int opposite_number = 0;
                    int number = 0;
                    while (st->end_vertex() != v2) {
                        opposite_number++;
                        st = MCGAL::find_prev(st->opposite());
                    }
                    number = st->vertex()->halfedges().size() - opposite_number;

                    // 尝试将周围的所有大于等于四的面片提前处理
                    do {
                        MCGAL::Facet* f = h->face();
                        if (f->facet_degree() > 3) {
                            MCGAL::Halfedge* hSplit(h->next());
                            for (; hSplit->next()->next() != h; hSplit = hSplit->next())
                                ;
                            MCGAL::Halfedge* hCorner = mesh->split_facet_non_meshId(h, hSplit);
                            hCorner->setAdded();
                        }
                    } while ((h = h->opposite()->next()) != end);
                    MCGAL::Point vPos = boundary->end_vertex()->point();
                    MCGAL::Halfedge* hNewFace = mesh->erase_center_vertex(boundary);
                    MCGAL::Facet* added_face = hNewFace->face();
                    added_face->setGroupId(groupId);
                    // added_face->setSplittable();
                    // added_face->setRemovedVertexPos(vPos);
                }
                if (boundary->end_vertex()->poolId() == stopId) {
                    break;
                }
                boundary = next_boundary(neighbourId, boundary);
            } while (boundary->end_vertex()->poolId() != stopId);
        }
    }
}

void VertexRemovalGreedyEncoder::encodeSymbolOp() {
    std::deque<char> facetSym;
    std::deque<char> halfedgeSym;
    std::deque<MCGAL::Point> points;
    std::queue<int> gateQueue;
    gateQueue.push(seed->face()->poolId());
    int current_version = MCGAL::BfsVersionMananger::current_version++;
    while (!gateQueue.empty()) {
        int fid = gateQueue.front();
        MCGAL::Facet* f = MCGAL::contextPool.getFacetByIndex(mesh->meshId(), fid);
        gateQueue.pop();
        if (f->isVisited(current_version) || f->isRemoved()) {
            continue;
        }
        f->setVisited(current_version);
        int vpoolId = -1;
        // int minVid = -1;
        MCGAL::Vertex* minV = (*f->halfedges_begin())->vertex();
        MCGAL::Halfedge* hIt = (*f->halfedges_begin());
        for (auto it = f->halfedges_begin(); it != f->halfedges_end(); it++) {
            MCGAL::Vertex* v = (*it)->vertex();
            if (v > minV) {
                minV = v;
                hIt = (*it);
            }
        }
        MCGAL::Halfedge* h = hIt;
        unsigned sym = f->isSplittable();
        facetSym.push_back(sym);

        if (sym) {
            MCGAL::Point rmved = f->getRemovedVertexPos();
            points.push_back(rmved);
        }
        do {
            unsigned sym;
            if (hIt->isOriginal()) {
                sym = 0;
            } else {
                sym = 1;
            }
            halfedgeSym.push_back(sym);
            MCGAL::Halfedge* hOpp = hIt->opposite();
            if (hOpp == nullptr) {
                hIt = hIt->next();
                continue;
            }
            // 对方没有被处理，且该边不是边界
            if (!hOpp->face()->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary() && hOpp->face()->groupId() == groupId) {
                gateQueue.push(hOpp->face()->poolId());
            }
            hIt = hIt->next();
        } while (hIt != h);
    }
    connectFaceSyms[groupId].push_back(facetSym);
    connectEdgeSyms[groupId].push_back(halfedgeSym);
    facetPointSyms[groupId].push_back(points);
}

void VertexRemovalGreedyEncoder::encodeBoundarySymbolOp() {}

MCGAL::Halfedge* VertexRemovalGreedyEncoder::vertexCut(MCGAL::Mesh* submesh, std::set<int>& boundaryIds, std::queue<int>& gateQueue, MCGAL::Halfedge* startH) {
    MCGAL::Vertex* v = startH->end_vertex();
    // assert(!v->isConquered());
    assert(v->vertex_degree() > 2);

    MCGAL::Halfedge* h = startH->opposite();
    MCGAL::Halfedge* end(h);
    int removed = 0;
    do {
        MCGAL::Facet* f = h->face();
        // assert(!f->isConquered() && !f->isRemoved());

        if (f->facet_degree() > 3) {
            MCGAL::Halfedge* hSplit(h->next());
            for (; hSplit->next()->next() != h; hSplit = hSplit->next())
                ;
            MCGAL::Halfedge* hCorner = submesh->split_facet_non_meshId(h, hSplit);
            hCorner->setAdded();
        }
        // h->end_vertex()->setConquered();
        removed++;
    } while ((h = h->opposite()->next()) != end);
    MCGAL::Point vPos = startH->end_vertex()->point();
    MCGAL::Halfedge* hNewFace = submesh->erase_center_vertex(startH);
    MCGAL::Facet* added_face = hNewFace->face();
    added_face->setSplittable();
    added_face->setRemovedVertexPos(vPos);
    h = hNewFace;
    do {
        MCGAL::Halfedge* hOpp = h->opposite();
        if (h->isBoundary()) {
            boundaryIds.insert(h->poolId());
        }
        if (hOpp == nullptr) {
            continue;
        }
        if (!hOpp->face()->isConquered() && !hOpp->isBoundary() && !h->isBoundary()) {
            gateQueue.push(hOpp->poolId());
        }
    } while ((h = h->next()) != hNewFace);
    return hNewFace;
}

bool VertexRemovalGreedyEncoder::boundaryRemovableInVertexRemoval(int inner, int outer, MCGAL::Halfedge* hit) {
    assert(hit->isBoundary());
    // 边界点不能被压缩
    if (triPoints->count(hit->vertex()) || triPoints->count(hit->end_vertex())) {
        return false;
    }
    // 被压缩过的不能再次被压缩
    if (hit->isBoundary() && hit->next()->isBoundary()) {
        if (hit->hasRemovedVertex() || hit->next()->hasRemovedVertex()) {
            return false;
        }
    }
    MCGAL::Vertex* v = hit->end_vertex();
    // seed其实顶点不能被压缩
    if (v->poolId() == seed->vertex()->poolId() || v->poolId() == seed->end_vertex()->poolId()) {
        return false;
    }
    // 大于等于8度的顶点不能被压缩
    if (!v->isConquered() && v->vertex_degree() > 2 && v->vertex_degree() < 8) {
        std::vector<MCGAL::Point> vh_oneRing;
        std::vector<MCGAL::Halfedge*> heh_oneRing;
        heh_oneRing.reserve(v->vertex_degree());
        for (MCGAL::Halfedge* hit : v->halfedges()) {
            if (hit == seed || (hit->opposite() != nullptr && hit->opposite() == seed)) {
                return false;
            }

            vh_oneRing.push_back(hit->opposite()->vertex()->point());
            heh_oneRing.push_back(hit);
        }
        bool removable = !willViolateManifold(heh_oneRing);  //  && arePointsCoplanar(vh_oneRing);
        // if (removable) {
        //     return checkCompetition(v);
        // }
        return removable;
    }
    return false;
}

bool VertexRemovalGreedyEncoder::isBoundaryRemovable(MCGAL::Halfedge* h) {
    bool res = false;
    if (seed->poolId() == h->poolId()) {
        return false;
    }

    std::set<int> poolIds;
    for (MCGAL::Facet::halfedge_iterator hit = h->face()->halfedges_begin(); hit != h->face()->halfedges_end(); ++hit) {
        if (poolIds.count((*hit)->opposite()->face()->poolId())) {
            return false;
        }
        poolIds.insert((*hit)->opposite()->face()->poolId());
    }
    poolIds.clear();
    for (MCGAL::Facet::halfedge_iterator hit = h->opposite()->face()->halfedges_begin(); hit != h->opposite()->face()->halfedges_end(); ++hit) {
        if (poolIds.count((*hit)->opposite()->face()->poolId())) {
            return false;
        }
        poolIds.insert((*hit)->opposite()->face()->poolId());
    }
    std::vector<MCGAL::Halfedge*> hs;
    poolIds.clear();
    for (MCGAL::Halfedge* hit : h->vertex()->halfedges()) {
        if (poolIds.count(hit->end_vertex()->poolId())) {
            continue;
        }
        hs.push_back(hit);
    }
    for (MCGAL::Halfedge* hit : h->end_vertex()->halfedges()) {
        if (poolIds.count(hit->end_vertex()->poolId())) {
            continue;
        }
        hs.push_back(hit);
    }
    // return h->canCollapse() && !h->isRemoved() && h->face()->facet_degree() == 3 && h->opposite()->face()->facet_degree() == 3 && !willViolateManifold(hs);
    return !h->isRemoved() && h->face()->facet_degree() == 3 && h->opposite()->face()->facet_degree() == 3 && !willViolateManifold(hs);
    // return false;
}

bool VertexRemovalGreedyEncoder::isRemovable(MCGAL::Vertex* v) {
    if (v->poolId() == seed->vertex()->poolId() || v->poolId() == seed->end_vertex()->poolId()) {
        return false;
    }

    if (!v->isConquered() && v->vertex_degree() > 2 && v->vertex_degree() < 8) {
        std::vector<MCGAL::Point> vh_oneRing;
        std::vector<MCGAL::Halfedge*> heh_oneRing;
        heh_oneRing.reserve(v->vertex_degree());
        for (MCGAL::Halfedge* hit : v->halfedges()) {
            if (hit->isBoundary() || hit->opposite()->isBoundary()) {
                return false;
            }

            if (hit == seed || (hit->opposite() != nullptr && hit->opposite() == seed)) {
                return false;
            }

            vh_oneRing.push_back(hit->opposite()->vertex()->point());
            heh_oneRing.push_back(hit);
        }
        bool removable = !willViolateManifold(heh_oneRing);  //  && arePointsCoplanar(vh_oneRing);
        // if (removable) {
        //     return checkCompetition(v);
        // }
        return removable;
    }
    return false;
}

bool VertexRemovalGreedyEncoder::arePointsCoplanar(std::vector<MCGAL::Point>& points) {
    if (points.size() < 4)
        return true;  // 三个点总是共面的

    // 选择前三个点，计算两个向量
    MCGAL::Vector3 v1 = points[1] - points[0];
    MCGAL::Vector3 v2 = points[2] - points[0];

    // 计算法向量
    MCGAL::Vector3 n = v1.cross(v2);

    // 检查剩余点是否在平面上
    for (size_t i = 3; i < points.size(); ++i) {
        MCGAL::Vector3 vi = points[i] - points[0];
        if (std::abs(n.dot(vi)) > 1e-8) {  // 如果点不在平面上
            return false;
        }
    }
    return true;
}

bool VertexRemovalGreedyEncoder::checkCompetition(MCGAL::Vertex* v) const {
    std::set<MCGAL::Facet*> fset;
    for (int i = 0; i < v->halfedges().size(); i++) {
        MCGAL::Halfedge* hit = v->halfedges()[i];
        fset.insert(hit->face());
    }
    for (MCGAL::Facet* fit : fset) {
        for (auto it = fit->halfedges_begin(); it != fit->halfedges_end(); it++) {
            if ((*it)->isAdded() || (*it)->opposite()->isAdded()) {
                return false;
            }
        }
    }
    return true;
}

MCGAL::Vector3 crossProduct(const MCGAL::Point& A, const MCGAL::Point& B, const MCGAL::Point& P) {
    // 向量 AB 和 AP
    MCGAL::Vector3 AB = B - A;
    MCGAL::Vector3 AP = P - A;

    // 叉积 AB × AP
    return AB.cross(AP);
}

// 判断点P是否在三角形ABC内部
bool isPointInTriangle(const MCGAL::Point& A, const MCGAL::Point& B, const MCGAL::Point& C, const MCGAL::Point& P) {
    // 计算三个向量叉积
    MCGAL::Vector3 cross1 = crossProduct(A, B, P);
    MCGAL::Vector3 cross2 = crossProduct(B, C, P);
    MCGAL::Vector3 cross3 = crossProduct(C, A, P);

    // 检查是否所有叉积的z分量符号相同
    bool hasNeg = (cross1.z() < 0) || (cross2.z() < 0) || (cross3.z() < 0);
    bool hasPos = (cross1.z() > 0) || (cross2.z() > 0) || (cross3.z() > 0);

    return !(hasNeg && hasPos);  // 如果叉积符号不一致，返回false
}

bool VertexRemovalGreedyEncoder::willViolateManifold(const std::vector<MCGAL::Halfedge*>& polygon) const {
    unsigned i_degree = polygon.size();
    for (unsigned i = 0; i < i_degree; ++i) {
        MCGAL::Halfedge* it = polygon[i];
        if (it->face()->facet_degree() == 3) {
            std::set<int> facecnt;
            MCGAL::Halfedge* st = it->face()->proxyHalfedge();
            MCGAL::Halfedge* ed = it->face()->proxyHalfedge();
            do {
                if (st->opposite() != nullptr) {
                    if (facecnt.count(st->opposite()->face()->poolId())) {
                        return true;
                    }
                    facecnt.insert(st->opposite()->face()->poolId());
                }
                st = st->next();
            } while (st != ed);
            continue;
        }

        for (int j = 0; j < i_degree; j++) {
            MCGAL::Halfedge* jt = polygon[j];
            if (i == j)
                continue;
            if (it->face() == jt->opposite()->face()) {
                MCGAL::Vertex* v1 = it->end_vertex();
                MCGAL::Vertex* v2 = jt->end_vertex();
                for (int k = 0; k < v1->halfedges().size(); k++) {
                    if (v1->halfedges()[k]->end_vertex() == v2) {
                        return true;
                    }
                }
                for (int k = 0; k < v2->halfedges().size(); k++) {
                    if (v2->halfedges()[k]->end_vertex() == v1) {
                        return true;
                    }
                }
                if (it->face()->facet_degree() > 3) {
                    if (it->next()->next()->next() == jt->opposite()) {
                        if (isPointInTriangle(it->vertex()->point(), it->end_vertex()->point(), jt->end_vertex()->point(), it->next()->end_vertex()->point())) {
                            return true;
                        }
                    }
                }
            } else if (it->face() == jt->face()) {
                for (int k = 0; k < it->end_vertex()->halfedges().size(); k++) {
                    if (it->end_vertex()->halfedges()[k]->end_vertex() == jt->end_vertex()) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void VertexRemovalGreedyEncoder::resetBfsState(MCGAL::Halfedge* seed) {
    // for (int i = 0; i < MCGAL::contextPool.getFindex(meshId); i++) {
    //     MCGAL::Facet* face = MCGAL::contextPool.getFacetByIndex(meshId, i);
    //     face->resetBfsFlag();
    // }
    // for (int i = 0; i < MCGAL::contextPool.getHindex(meshId); i++) {
    //     MCGAL::Halfedge* hit = MCGAL::contextPool.getHalfedgeByIndex(meshId, i);
    //     if (hit->opposite() == nullptr || hit->face()->groupId() != hit->opposite()->face()->groupId()) {
    //         hit->setBoundary();
    //     }
    //     hit->resetBfsFlag();
    // }
    std::deque<MCGAL::Halfedge*> gateQueue;
    gateQueue.push_back(seed);
    while (!gateQueue.empty()) {}
}

MCGAL::Halfedge* VertexRemovalGreedyEncoder::next_boundary(int ogroupId, MCGAL::Halfedge* boundary) {
    MCGAL::Halfedge* nxt = boundary->next();
    if (nxt->isBoundary()) {
        return nxt;
    }
    nxt = boundary->next()->opposite()->next();
    if (nxt->isBoundary()) {
        return nxt;
    }
    for (MCGAL::Halfedge* hit : boundary->end_vertex()->halfedges()) {
        if (hit->opposite() != boundary && hit->isBoundary()) {
            return hit;
        }
    }
    return nullptr;
}

MCGAL::Halfedge* VertexRemovalGreedyEncoder::find_prev(MCGAL::Halfedge* h) {
    MCGAL::Halfedge* g = h;
    while (g->next() != h) {
        g = g->next();
    }
    return g;
}
