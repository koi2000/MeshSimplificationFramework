/*
 * @Author: koi
 * @Date: 2025-09-13 23:10:21
 * @Description:
 */
#include "BasicSegmentOperator.h"
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

#define RANDOM_SEED 345

void BasicSegmentOperator::segment(std::shared_ptr<MCGAL::Mesh>& mesh) {
    this->mesh = mesh;
    markBoundary(mesh);
    buildGraph();
}

void BasicSegmentOperator::markBoundary(std::shared_ptr<MCGAL::Mesh>& mesh) {
    int fsize = mesh->size_of_facets();
    int sampleNumber = number;
    std::vector<int> targetNumber(sampleNumber, fsize / sampleNumber);
    targetNumber[sampleNumber - 1] = fsize - (sampleNumber - 1) * (fsize / sampleNumber);
    std::vector<int> tmpNumber(sampleNumber, 0);
    std::set<int> allSet;
    for (MCGAL::Facet* fit : mesh->faces()) {
        allSet.insert(fit->poolId());
    }
    int idx = 0;

    std::map<int, int> group2cnt;
    std::mt19937 gen(RANDOM_SEED);
    while (!allSet.empty()) {
        std::set<int> group;
        std::uniform_int_distribution<> dis(0, allSet.size() - 1);
        auto it = allSet.begin();
        std::advance(it, dis(gen));
        MCGAL::Facet* fit = MCGAL::contextPool.getFacetByIndex(mesh->meshId(), *it);
        fit->setGroupId(idx);
        // int cnt = 1;

        std::queue<int> gateQueue;
        gateQueue.push(fit->poolId());
        std::set<int> neighbours;
        while (!gateQueue.empty()) {
            int fid = gateQueue.front();
            gateQueue.pop();
            MCGAL::Facet* f = MCGAL::contextPool.getFacetByIndex(mesh->meshId(), fid);
            if (f->isProcessed()) {
                continue;
            }
            f->setProcessedFlag();
            group.insert(f->poolId());
            int cnt = group.size();
            if (cnt == targetNumber[idx % targetNumber.size()]) {
                continue;
            }
            MCGAL::Halfedge* st = f->proxyHalfedge();
            MCGAL::Halfedge* ed = st;
            MCGAL::Vertex* minV = st->vertex();
            MCGAL::Halfedge* hIt = st;
            do {
                if (MCGAL::compareVertex(minV, st->vertex())) {
                    minV = st->vertex();
                    hIt = st;
                }
                st = st->next();
            } while (st != ed);

            MCGAL::Halfedge* h = hIt;
            do {
                MCGAL::Halfedge* hOpp = hIt->opposite();
                if (!hOpp->face()->isProcessed()) {
                    hOpp->face()->setGroupId(f->groupId());
                    gateQueue.push(hOpp->face()->poolId());
                    // if (!group.count(hOpp->face->poolId)) {
                    //     cnt++;
                    // }
                    group.insert(hOpp->face()->poolId());
                    int cnt = group.size();
                    if (cnt == targetNumber[idx % targetNumber.size()]) {
                        break;
                    }
                } else if (hOpp->face()->groupId() != f->groupId() && hOpp->face()->groupId() != -1) {
                    neighbours.insert(hOpp->face()->groupId());
                }
                hIt = hIt->next();
            } while (hIt != h);
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
            for (int fid : group) {
                fit = MCGAL::contextPool.getFacetByIndex(mesh->meshId(), fid);
                // fit->groupId = resId;
                fit->setGroupId(resId);
            }
            continue;
        }
        seeds.push_back(fit->proxyHalfedge());
        idx++;
    }
    for (MCGAL::Facet* fit : mesh->faces()) {
        for (auto it = fit->halfedges_begin(); it != fit->halfedges_end(); it++) {
            if ((*it)->face()->groupId() != (*it)->opposite()->face()->groupId()) {
                (*it)->setBoundary();
                (*it)->opposite()->setBoundary();
            }
        }
    }
}

void BasicSegmentOperator::buildGraph() {
    std::set<MCGAL::Vertex*> triPoints;
    for (MCGAL::Vertex* vit : mesh->vertices()) {
        std::set<int> cnt;
        for (MCGAL::Halfedge* hit : vit->halfedges()) {
            cnt.insert(hit->face()->groupId());
        }
        if (cnt.size() >= 3) {
            triPoints.insert(vit);
        }
    }
    for (MCGAL::Vertex* vit : triPoints) {
        for (MCGAL::Halfedge* hit : vit->halfedges()) {
            if (hit->isBoundary()) {
                // if (triPoints.count(hit->vertex) && triPoints.count(hit->end_vertex)) {
                //     g.addHEdge(hit->face->groupId, {hit->vertex->id, hit->end_vertex->id, hit->opposite->face->groupId, hit->end_vertex->id});
                //     continue;
                // }
                MCGAL::Halfedge* st = hit;
                int inner = st->face()->groupId();
                int outer = st->opposite()->face()->groupId();
                int dis = 0;
                do {
                    assert(st->isBoundary());
                    MCGAL::Halfedge* ed = st;
                    if (triPoints.count(st->end_vertex())) {
                        int i = 0;
                    }
                    st = next_boundary(inner, outer, st);

                    if (st == nullptr) {
                        g.addHEdge(inner, outer, {hit->vertex()->poolId(), hit->end_vertex()->poolId(), ed->end_vertex()->poolId()});
                    }
                    dis++;
                } while (st != nullptr);
                // g.addEdge({hit->vertex->id, hit->end_vertex->id, hit->opposite->face->groupId, st->end_vertex->id}, {st->end_vertex->id, st->vertex->id, hit->face->groupId, hit->vertex->id});
            }
        }
    }
    g.setTriPoints(triPoints);
}

std::vector<MCGAL::Halfedge*> BasicSegmentOperator::exportSeeds() {
    return seeds;
}

Graph BasicSegmentOperator::exportGraph() {
    return g;
}