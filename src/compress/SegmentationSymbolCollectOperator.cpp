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
#include "../rangecoder/qsmodel.h"
#include "../rangecoder/rangecod.h"
#include "BFSVersionManager.h"
#include "Facet.h"
#include "Halfedge.h"
#include "MeshUtils.h"
#include "Point.h"
#include "core.h"
#include <algorithm>
#include <vector>

void SegmentationSymbolCollectOperator::collect(MCGAL::Halfedge* seed) {
    std::deque<char> facetSym;
    std::deque<char> edgeSym;
    std::deque<MCGAL::Point> points;
    std::deque<MCGAL::PointInt> ipoints;
    std::queue<int> gateQueue;
    gateQueue.push(seed->poolId());
    int current_version = MCGAL::BfsVersionMananger::current_version++;
    int fcount = 0;
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
            if (!hOpp->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary()) {
                gateQueue.push(hOpp->poolId());
            }
            if (!hIt->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary()) {
                gateQueue.push(hIt->poolId());
            }
            hIt = hIt->next();
        } while (hIt != h);
    }

    gateQueue.push(seed->poolId());
    current_version = MCGAL::BfsVersionMananger::current_version++;
    int ecount = 0;
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
            if (!hOpp->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary()) {
                gateQueue.push(hOpp->poolId());
            }
            if (!hIt->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary()) {
                gateQueue.push(hIt->poolId());
            }
            hIt = hIt->next();
        } while (hIt != h);
    }
    std::cout << "ecount: " << ecount << std::endl;
    std::cout << "fcount: " << fcount << std::endl;
    // facetSymbolQueues.push_back(facetSym);
    // edgeSymbolQueues.push_back(edgeSym);
    // ipointQueues.push_back(ipoints);
    // pointQueues.push_back(points);
}

void SegmentationSymbolCollectOperator::collect(std::vector<MCGAL::Halfedge*> seeds) {
    for (MCGAL::Halfedge* seed : seeds) {
        // collect(seed);
    }
}

int SegmentationSymbolCollectOperator::exportToBuffer(char* buffer, bool isEnableQuantization) {
    int offset = 0;
    return offset;
}