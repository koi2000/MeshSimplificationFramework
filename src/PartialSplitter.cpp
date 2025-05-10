#include "./include/PartialSplitter.h"
#include <algorithm>
#include <bitset>
#include <map>

inline MCGAL::Halfedge* next_boundary(int inner, int outer, MCGAL::Halfedge* boundary) {
    MCGAL::Halfedge* nxt = boundary->next();
    if (nxt->isBoundary()) {
        if (nxt->face()->groupId() == inner && nxt->opposite()->face()->groupId() == outer) {
            return nxt;
        }
    }
    nxt = boundary->next()->opposite()->next();
    if (nxt->isBoundary()) {
        if (nxt->face()->groupId() == inner && nxt->opposite()->face()->groupId() == outer) {
            return nxt;
        }
    }
    for (MCGAL::Halfedge* hit : boundary->end_vertex()->halfedges()) {
        if (hit->opposite()->isBoundary() && hit->isBoundary() && hit->face()->groupId() == inner && hit->opposite()->face()->groupId() == outer) {
            return hit;
        }
    }
    return nullptr;
}

PartialSplitter::PartialSplitter(std::string filename) {
    mesh = new MCGAL::Mesh();
    mesh->loadOFF(filename);
}

PartialSplitter::PartialSplitter(MCGAL::Mesh* mesh_, bool skipMarkBoundary) {
    this->mesh = mesh_;
}

void PartialSplitter::loadMesh(MCGAL::Mesh* mesh_, bool skipMarkBoundary) {
    this->mesh = mesh_;
}

std::vector<MCGAL::Halfedge*>& PartialSplitter::exportSeeds() {
    return seeds;
}

std::set<MCGAL::Vertex*>& PartialSplitter::exportTriPoints() {
    return triPoints;
}

std::unordered_map<int, int> PartialSplitter::exportDup2Origin() {
    return std::move(dup2origin);
}

std::unordered_map<int, std::unordered_map<int, int>> PartialSplitter::exportOrigin2Dup() {
    return std::move(origin2dup);
}

Graph PartialSplitter::exportGraph() {
    return std::move(g);
}

std::vector<MCGAL::Mesh>& PartialSplitter::exportSubMeshes() {
    return subMeshes;
}

/**
 * 使用copy_if直接拷贝过去
 * 对point需要进行dup
 */
int PartialSplitter::split(int groupNumber) {
    groupNumber = markBoundry();
    // for (int i = 0; i < 50; i++) {
    //     std::string path = "./submesh5/round" + std::to_string(1) + "_group" + std::to_string(i) + ".off";
    //     mesh->dumptoInEncoder(i, path);
    // }
    buildGraph();
    return groupNumber;
}

/***
 * 思路，找到所有的tripoint 作为起点，
 * 只有vertex为起始点的边才是有效边，通过该方法确定了方向
 * 从起始点出发，到另一个点为止，是一个边界，通过该方法可以构建图
 */
// 有些有两段连接
void PartialSplitter::buildGraph() {
    for (int i = 0; i < MCGAL::contextPool.getVindex(mesh->meshId()); i++) {
        MCGAL::Vertex* vit = MCGAL::contextPool.getVertexByIndex(mesh->meshId(), i);
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
                MCGAL::Halfedge* st = hit;
                int inner = st->face()->groupId();
                int outer = st->opposite()->face()->groupId();
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
                } while (st != nullptr);
            }
        }
    }
}

void PartialSplitter::dumpSubMesh(std::string path, int groupId) {
    // assert(groupId < subMeshes.size());
    // subMeshes[groupId].dumpto(path);
}

void PartialSplitter::neAllocEdges(std::set<int> allSet, std::set<int>& coreSet, std::set<int>& boundarySet, std::set<int>& part, int x, int number) {
    coreSet.insert(x);
    boundarySet.insert(x);
    // 获取x的boundary
    MCGAL::Facet* fit = MCGAL::contextPool.getFacetByIndex(mesh->meshId(), x);
    std::set<int> neighbour;
    std::vector<int> diff;
    std::set_difference(neighbour.begin(), neighbour.end(), boundarySet.begin(), boundarySet.end(), std::inserter(diff, diff.begin()));
    for (int y : diff) {
        boundarySet.insert(y);
    }
}

std::set<int> PartialSplitter::neExpand(std::set<int>& allSet, int number) {
    int partNumber = 0;
    int meshId = mesh->meshId();
    std::set<int> part;
    std::set<int> coreSet;
    std::set<int> boundarySet;
    std::mt19937 gen(RANDOM_SEED);
    bool inited = false;
    while (boundarySet.size() < number) {
        std::vector<int> diff;
        std::set_difference(boundarySet.begin(), boundarySet.end(), coreSet.begin(), coreSet.end(), std::inserter(diff, diff.begin()));
        int x = -1;
        if (!inited && diff.empty()) {
            std::uniform_int_distribution<> dis(0, allSet.size() - 1);
            auto it = allSet.begin();
            std::advance(it, dis(gen));
            x = *it;
            inited = true;
        } else {
            int rscore = INT_MAX;
            for (int v : diff) {
                MCGAL::Facet* fit = MCGAL::contextPool.getFacetByIndex(meshId, v);
                std::set<int> neighbour;

                for (MCGAL::Facet::halfedge_iterator it = fit->halfedges_begin(); it != fit->halfedges_end(); ++it) {
                    MCGAL::Halfedge* hit = *it;
                    if (!allSet.count(hit->opposite()->face()->poolId())) {
                        continue;
                    }
                    neighbour.insert(hit->opposite()->face()->poolId());
                }
                std::vector<int> score;
                std::set_difference(neighbour.begin(), neighbour.end(), boundarySet.begin(), boundarySet.end(), std::inserter(score, score.begin()));
                if (score.size() < rscore) {
                    x = v;
                    rscore = score.size();
                }
            }
        }
        if (x != -1) {
            neAllocEdges(allSet, coreSet, boundarySet, part, x, number);
        } else {
            break;
        }
    }
    // allSet.erase(std::remove_if(allSet.begin(), allSet.end(), [&boundarySet](const int& value) { return boundarySet.find(value) != boundarySet.end(); }), allSet.end());
    std::set<int> result;
    std::set_difference(allSet.begin(), allSet.end(), boundarySet.begin(), boundarySet.end(), std::inserter(result, result.begin()));

    // 将结果赋给 set1
    allSet = std::move(result);
    return boundarySet;
}

int PartialSplitter::markBoundry() {
    int meshId = mesh->meshId();
    int fsize = MCGAL::contextPool.getFindex(mesh->meshId());
    int sampleNumber = 50;
    std::vector<int> targetNumber(sampleNumber, fsize / sampleNumber);
    targetNumber[sampleNumber - 1] = fsize - (sampleNumber - 1) * (fsize / sampleNumber);
    std::vector<int> tmpNumber(sampleNumber, 0);
    std::set<int> allSet;

    for (int i = 0; i < fsize; i++) {
        allSet.insert(i);
    }
    int idx = 0;

    std::map<int, int> group2cnt;
    std::mt19937 gen(RANDOM_SEED);
    while (!allSet.empty()) {
        std::set<int> group;
        std::uniform_int_distribution<> dis(0, allSet.size() - 1);
        auto it = allSet.begin();
        std::advance(it, dis(gen));
        MCGAL::Facet* fit = MCGAL::contextPool.getFacetByIndex(meshId, *it);
        fit->setGroupId(idx);

        std::queue<int> gateQueue;
        gateQueue.push(fit->poolId());
        std::set<int> neighbours;
        while (!gateQueue.empty()) {
            int fid = gateQueue.front();
            gateQueue.pop();
            MCGAL::Facet* f = MCGAL::contextPool.getFacetByIndex(meshId, fid);
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
            MCGAL::Halfedge* ed = f->proxyHalfedge();
            MCGAL::Halfedge* hIt = st;
            MCGAL::Vertex* minV = st->vertex();
            do {
                if (st->vertex() > minV) {
                    hIt = st;
                }
                st->resetState();
                st = st->next();
            } while (st != ed);

            MCGAL::Halfedge* h = hIt;
            do {
                MCGAL::Halfedge* hOpp = hIt->opposite();
                if (!hOpp->face()->isProcessed()) {
                    hOpp->face()->setGroupId(f->groupId());
                    gateQueue.push(hOpp->face()->poolId());
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
                fit = MCGAL::contextPool.getFacetByIndex(meshId, fid);
                fit->setGroupId(resId);
            }
            continue;
        }
        seeds.push_back(fit->proxyHalfedge());
        idx++;
    }

    for (int i = 0; i < MCGAL::contextPool.getHindex(meshId); i++) {
        MCGAL::Halfedge* hit = MCGAL::contextPool.getHalfedgeByIndex(meshId, i);
        if (hit->face()->groupId() != hit->opposite()->face()->groupId()) {
            hit->setBoundary();
            hit->opposite()->setBoundary();
        }
    }
    return idx;
}