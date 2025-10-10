/*
 * @Author: koi
 * @Date: 2025-09-18 17:31:27
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-08-29 21:13:56
 * @Description:
 */

#include "SegmentationSymbolCollectOperator.h"
#include "../common/BufferUtils.h"
#include "BFSVersionManager.h"
#include "Facet.h"
#include "Halfedge.h"
#include "Point.h"
#include <deque>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>

void SegmentationSymbolCollectOperator::collect(MCGAL::Halfedge* seed) {
    std::deque<char> facetSym;
    std::deque<char> edgeSym;
    std::deque<MCGAL::Point> points;
    std::deque<MCGAL::PointInt> ipoints;
    std::queue<int> gateQueue;
    gateQueue.push(seed->poolId());
    int current_version = MCGAL::BfsVersionMananger::current_version++;
    
    std::vector<MCGAL::Facet*> faces;
    std::vector<MCGAL::Halfedge*> halfedges;
    while (!gateQueue.empty()) {
        int hid = gateQueue.front();
        MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndexInSubPool(mesh_->meshId(), hid);
        gateQueue.pop();
        if (h->isVisited(current_version) || h->isRemoved()) {
            continue;
        }
        h->setVisited(current_version);

        if (!h->face()->isVisited(current_version)) {
            MCGAL::Facet* f = h->face();
            f->setVisited(current_version);
            unsigned sym = f->isSplittable();
            facetSym.push_back(sym);
            if (sym) {
                MCGAL::Point rmved = f->getRemovedVertexPos();
                points.push_back(rmved);
                ipoints.push_back(f->getRemovedVertexPosInt());
                fcount++;
                faces.push_back(f);
            }
        }
        MCGAL::Halfedge* hIt = h;
        do {
            MCGAL::Halfedge* hOpp = hIt->opposite();
            if (!hOpp->isVisited(current_version) && hIt->vertex()->groupId() == hIt->end_vertex()->groupId()) {
                gateQueue.push(hOpp->poolId());
            }
            if (!hIt->isVisited(current_version) && hIt->vertex()->groupId() == hIt->end_vertex()->groupId()) {
                gateQueue.push(hIt->poolId());
            }
            hIt = hIt->opposite()->next();
        } while (hIt != h);
    }
    gateQueue.push(seed->poolId());
    current_version = MCGAL::BfsVersionMananger::current_version++;
    // int ecount = 0;
    while (!gateQueue.empty()) {
        int hid = gateQueue.front();
        MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndexInSubPool(mesh_->meshId(), hid);
        gateQueue.pop();
        if (h->isVisited(current_version) || h->isRemoved()) {
            continue;
        }
        h->setVisited(current_version);
        // if (!h->opposite()->isVisited(current_version)) {
        if (h->isAdded()) {
            edgeSym.push_back(1);
            halfedges.push_back(h);
            ecount++;
        } else {
            edgeSym.push_back(0);
        }
        // }
        MCGAL::Halfedge* hIt = h;
        do {
            MCGAL::Halfedge* hOpp = hIt->opposite();
            if (!hOpp->isVisited(current_version) && hIt->vertex()->groupId() == hIt->end_vertex()->groupId()) {
                gateQueue.push(hOpp->poolId());
            }
            if (!hIt->isVisited(current_version) && hIt->vertex()->groupId() == hIt->end_vertex()->groupId()) {
                gateQueue.push(hIt->poolId());
            }
            hIt = hIt->opposite()->next();
        } while (hIt != h);
    }
    segmentFacetSymbolQueues[groupIdx].push_back(facetSym);
    segmentEdgeSymbolQueues[groupIdx].push_back(edgeSym);
    segmentIpointQueues[groupIdx].push_back(ipoints);
    segmentPointQueues[groupIdx].push_back(points);
}

void SegmentationSymbolCollectOperator::collect(std::vector<MCGAL::Halfedge*> seeds) {
    if (!inited) {
        inited = true;
        segmentFacetSymbolQueues.resize(seeds.size());
        segmentEdgeSymbolQueues.resize(seeds.size());
        segmentIpointQueues.resize(seeds.size());
        segmentPointQueues.resize(seeds.size());
    }
    fcount = 0;
    ecount = 0;
    groupIdx = 0;
    for (MCGAL::Halfedge* seed : seeds) {
        collect(seed);
        groupIdx++;
    }
    std::cout << "ecount: " << ecount << std::endl;
    std::cout << "fcount: " << fcount << std::endl;
}

int SegmentationSymbolCollectOperator::exportToBuffer(char* buffer, bool isEnableQuantization) {
    int beforeOffset = 0;
    int offset = groupIdx * sizeof(int);
    char* outBlock = buffer;
// #ifndef DEBUG
    std::vector<int> sizes = {0};
// #endif
    for (int gid = 0; gid < groupIdx; gid++) {
        int now = offset;
        for (int j = segmentFacetSymbolQueues[gid].size() - 1; j >= 0; j--) {
            unsigned i_bitOffset = 0;
            std::deque<char> facetSym = segmentFacetSymbolQueues[gid][j];
            std::deque<char> edgeSym = segmentEdgeSymbolQueues[gid][j];
            std::deque<MCGAL::Point> points = segmentPointQueues[gid][j];
            while (!facetSym.empty()) {
                char sym = facetSym.front();
                facetSym.pop_front();
                writeChar(buffer, offset, sym);
                // writeBits(sym, 1, buffer, i_bitOffset, offset);
                if (sym) {
                    MCGAL::Point point = points.front();
                    points.pop_front();
                    if (isEnableQuantization) {
                        MCGAL::PointInt pi = mesh_->floatPosToInt(point);
                        for (int i = 0; i < 3; i++) {
                            writeBits((uint32_t)pi[i], mesh_->i_nbQuantBits, buffer, i_bitOffset, offset);
                        }
                    } else {
                        writePoint(buffer, offset, point);
                    }
                }
            }
            while (!edgeSym.empty()) {
                char sym = edgeSym.front();
                edgeSym.pop_front();
                writeChar(buffer, offset, sym);
                // writeBits(sym, 1, buffer, i_bitOffset, offset);
            }
            // offset++;
        }
        int size = offset - now;
// #ifndef DEBUG
        sizes.push_back(size);
// #endif
//         writeInt(buffer, beforeOffset, size);
    }
    for (size_t i = 1; i < sizes.size(); ++i) {
        sizes[i] += sizes[i - 1];
    }
    for (int i = 0; i < groupIdx; ++i) {
        // std::memcpy(buffer + i * sizeof(int), &sizes[i], sizeof(int));
        writeInt(buffer, beforeOffset, sizes[i]);
    }

    return offset;
}