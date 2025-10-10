/*
 * @Author: koi
 * @Date: 2025-10-05 11:51:01
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-09-13 23:10:21
 * @Description:
 */
#include "VertexSegmentOperator.h"
#include "Global.h"
#include "Halfedge.h"
#include "Mesh.h"
#include "MeshUtils.h"
#include "Vertex.h"
#include "common/Graph.h"
#include <map>
#include <memory>
#include <random>
#include <vector>

#define RANDOM_SEED 3945

// 辅助函数：找到下一个边界边（基于顶点的版本）
static MCGAL::Halfedge* next_boundary_vertex(int inner, int outer, MCGAL::Halfedge* boundary) {
    MCGAL::Halfedge* nxt = boundary->next();
    if (nxt->isBoundary()) {
        if (nxt->vertex()->groupId() == inner && nxt->end_vertex()->groupId() == outer) {
            return nxt;
        }
    }
    nxt = boundary->next()->opposite()->next();
    if (nxt->isBoundary()) {
        if (nxt->vertex()->groupId() == inner && nxt->end_vertex()->groupId() == outer) {
            return nxt;
        }
    }
    for (MCGAL::Halfedge* hit : boundary->end_vertex()->halfedges()) {
        if (hit->opposite()->isBoundary() && hit->isBoundary() && hit->vertex()->groupId() == inner && hit->end_vertex()->groupId() == outer) {
            return hit;
        }
    }
    return nullptr;
}

void VertexSegmentOperator::segment(std::shared_ptr<MCGAL::Mesh>& mesh) {
    this->mesh = mesh;
    markBoundary(mesh);
    buildGraph();
}

void VertexSegmentOperator::markBoundary(std::shared_ptr<MCGAL::Mesh>& mesh) {
    int vsize = mesh->size_of_vertices();
    int sampleNumber = number;
    std::vector<int> targetNumber(sampleNumber, vsize / sampleNumber);
    targetNumber[sampleNumber - 1] = vsize - (sampleNumber - 1) * (vsize / sampleNumber);
    std::vector<int> tmpNumber(sampleNumber, 0);
    std::set<int> allSet;
    for (MCGAL::Vertex* vit : mesh->vertices()) {
        allSet.insert(vit->poolId());
    }
    int idx = 0;
    std::map<int, int> group2cnt;
    std::mt19937 gen(RANDOM_SEED);
    while (!allSet.empty()) {
        std::set<int> group;
        std::uniform_int_distribution<> dis(0, allSet.size() - 1);
        auto it = allSet.begin();
        std::advance(it, dis(gen));
        MCGAL::Vertex* vit = MCGAL::contextPool.getVertexByIndex(mesh->meshId(), *it);
        vit->setGroupId(idx);
        std::queue<int> gateQueue;
        gateQueue.push(vit->poolId());
        std::set<int> neighbours;
        while (!gateQueue.empty()) {
            int vid = gateQueue.front();
            gateQueue.pop();
            MCGAL::Vertex* v = MCGAL::contextPool.getVertexByIndex(mesh->meshId(), vid);
            if (v->isProcessed()) {
                continue;
            }
            v->setProcessed();
            group.insert(v->poolId());
            int cnt = group.size();
            if (cnt == targetNumber[idx % targetNumber.size()]) {
                continue;
            }
            // 遍历当前顶点的所有邻接顶点
            for (MCGAL::Halfedge* hit : v->halfedges()) {
                MCGAL::Vertex* neighbor = hit->end_vertex();
                if (!neighbor->isProcessed()) {
                    neighbor->setGroupId(v->groupId());
                    gateQueue.push(neighbor->poolId());
                    group.insert(neighbor->poolId());
                    int cnt = group.size();
                    if (cnt == targetNumber[idx % targetNumber.size()]) {
                        break;
                    }
                } else if (neighbor->groupId() != v->groupId() && neighbor->groupId() != -1) {
                    neighbours.insert(neighbor->groupId());
                }
            }
        }
        int cnt = group.size();
        group2cnt[idx] = cnt;
        std::set<int> result;
        std::set_difference(allSet.begin(), allSet.end(), group.begin(), group.end(), std::inserter(result, result.begin()));
        allSet = std::move(result);
        if (cnt < targetNumber[idx % targetNumber.size()] / 2 || idx > sampleNumber) {
            int mincnt = INT_MAX;
            int resId = -1;
            for (int nid : neighbours) {
                int cnt = group2cnt[nid];
                if (cnt < mincnt) {
                    resId = nid;
                    mincnt = cnt;
                }
            }
            group2cnt[resId] += cnt;
            for (int vid : group) {
                vit = MCGAL::contextPool.getVertexByIndex(mesh->meshId(), vid);
                vit->setGroupId(resId);
            }
            continue;
        }
        // 为顶点选择种子边（选择该顶点的第一条边作为种子）
        if (!vit->halfedges().empty()) {
            for (MCGAL::Halfedge* hit : vit->halfedges()) {
                if (hit->vertex()->groupId() == hit->end_vertex()->groupId()) {
                    seeds.push_back(hit);
                    break;
                }
            }
        }
        idx++;
    }
    // 标记边界边：连接不同组的顶点的边
    // for (MCGAL::Vertex* vit : mesh->vertices()) {
    //     for (MCGAL::Halfedge* hit : vit->halfedges()) {
    //         if (hit->vertex()->groupId() != hit->end_vertex()->groupId()) {
    //             hit->setBoundary();
    //             hit->opposite()->setBoundary();
    //         }
    //     }
    // }
}

// void VertexSegmentOperator::markBoundary(std::shared_ptr<MCGAL::Mesh>& mesh) {
//     int fsize = mesh->size_of_facets();
//     int sampleNumber = number;
//     std::vector<int> targetNumber(sampleNumber, fsize / sampleNumber);
//     targetNumber[sampleNumber - 1] = fsize - (sampleNumber - 1) * (fsize / sampleNumber);
//     std::vector<int> tmpNumber(sampleNumber, 0);
//     std::set<int> allSet;
//     for (MCGAL::Facet* fit : mesh->faces()) {
//         allSet.insert(fit->poolId());
//     }
//     int idx = 0;

//     std::map<int, int> group2cnt;
//     std::mt19937 gen(RANDOM_SEED);
//     while (!allSet.empty()) {
//         std::set<int> group;
//         std::uniform_int_distribution<> dis(0, allSet.size() - 1);
//         auto it = allSet.begin();
//         std::advance(it, dis(gen));
//         MCGAL::Facet* fit = MCGAL::contextPool.getFacetByIndex(mesh->meshId(), *it);
//         fit->setGroupId(idx);
//         // int cnt = 1;

//         std::queue<int> gateQueue;
//         gateQueue.push(fit->poolId());
//         std::set<int> neighbours;
//         while (!gateQueue.empty()) {
//             int fid = gateQueue.front();
//             gateQueue.pop();
//             MCGAL::Facet* f = MCGAL::contextPool.getFacetByIndex(mesh->meshId(), fid);
//             if (f->isProcessed()) {
//                 continue;
//             }
//             f->setProcessedFlag();
//             group.insert(f->poolId());
//             int cnt = group.size();
//             if (cnt == targetNumber[idx % targetNumber.size()]) {
//                 continue;
//             }
//             MCGAL::Halfedge* st = f->proxyHalfedge();
//             MCGAL::Halfedge* ed = st;
//             MCGAL::Vertex* minV = st->vertex();
//             MCGAL::Halfedge* hIt = st;
//             do {
//                 if (MCGAL::compareVertex(minV, st->vertex())) {
//                     minV = st->vertex();
//                     hIt = st;
//                 }
//                 st = st->next();
//             } while (st != ed);

//             MCGAL::Halfedge* h = hIt;
//             do {
//                 MCGAL::Halfedge* hOpp = hIt->opposite();
//                 if (!hOpp->face()->isProcessed()) {
//                     hOpp->face()->setGroupId(f->groupId());
//                     gateQueue.push(hOpp->face()->poolId());
//                     group.insert(hOpp->face()->poolId());
//                     int cnt = group.size();
//                     if (cnt == targetNumber[idx % targetNumber.size()]) {
//                         break;
//                     }
//                 } else if (hOpp->face()->groupId() != f->groupId() && hOpp->face()->groupId() != -1) {
//                     neighbours.insert(hOpp->face()->groupId());
//                 }
//                 hIt = hIt->next();
//             } while (hIt != h);
//         }
//         int cnt = group.size();
//         group2cnt[idx] = cnt;
//         std::set<int> result;
//         std::set_difference(allSet.begin(), allSet.end(), group.begin(), group.end(), std::inserter(result, result.begin()));
//         allSet = std::move(result);
//         // if (cnt < targetNumber[idx % targetNumber.size()] / 2 || idx > sampleNumber) {
//         //     int mincnt = INT_MAX;
//         //     int resId = -1;
//         //     for (int nid : neighbours) {
//         //         int cnt = group2cnt[nid];
//         //         if (cnt < mincnt) {
//         //             resId = nid;
//         //             mincnt = cnt;
//         //         }
//         //     }
//         //     group2cnt[resId] += cnt;
//         //     for (int fid : group) {
//         //         fit = MCGAL::contextPool.getFacetByIndex(mesh->meshId(), fid);
//         //         // fit->groupId = resId;
//         //         fit->setGroupId(resId);
//         //     }
//         //     continue;
//         // }
//         seeds.push_back(fit->proxyHalfedge());
//         idx++;
//     }
//     for (MCGAL::Facet* fit : mesh->faces()) {
//         for (auto it = fit->halfedges_begin(); it != fit->halfedges_end(); it++) {
//             // if ((*it)->face()->groupId() != (*it)->opposite()->face()->groupId()) {
//             //     // (*it)->setBoundary();
//             //     // (*it)->opposite()->setBoundary();
//             // } else {
//                 (*it)->vertex()->setGroupId(fit->groupId());
//                 (*it)->end_vertex()->setGroupId(fit->groupId());
//             // }
//         }
//     }
// }

void VertexSegmentOperator::buildGraph() {
    std::set<MCGAL::Vertex*> triPoints;
    for (MCGAL::Vertex* vit : mesh->vertices()) {
        std::set<int> cnt;
        for (MCGAL::Halfedge* hit : vit->halfedges()) {
            cnt.insert(hit->end_vertex()->groupId());
        }
        if (cnt.size() >= 3) {
            triPoints.insert(vit);
        }
    }
    for (MCGAL::Vertex* vit : triPoints) {
        for (MCGAL::Halfedge* hit : vit->halfedges()) {
            if (hit->isBoundary()) {
                MCGAL::Halfedge* st = hit;
                int inner = st->vertex()->groupId();
                int outer = st->end_vertex()->groupId();
                int dis = 0;
                do {
                    assert(st->isBoundary());
                    MCGAL::Halfedge* ed = st;
                    if (triPoints.count(st->end_vertex())) {
                        int i = 0;
                    }
                    st = next_boundary_vertex(inner, outer, st);

                    if (st == nullptr) {
                        g.addHEdge(inner, outer, {hit->vertex()->poolId(), hit->end_vertex()->poolId(), ed->end_vertex()->poolId()});
                    }
                    dis++;
                } while (st != nullptr);
            }
        }
    }
    g.setTriPoints(triPoints);
}

std::vector<MCGAL::Halfedge*> VertexSegmentOperator::exportSeeds() {
    return seeds;
}

Graph VertexSegmentOperator::exportGraph() {
    return g;
}