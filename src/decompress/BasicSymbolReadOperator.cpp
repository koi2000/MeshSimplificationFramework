/*
 * @Author: koi
 * @Date: 2025-08-31 22:33:59
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-08-29 21:13:56
 * @Description:
 */

#include "BasicSymbolReadOperator.h"
#include "../common/BufferUtils.h"
#include "BFSVersionManager.h"
#include "Configuration.h"
#include "Facet.h"
#include "MeshUtils.h"
#include "core.h"
#include <deque>

void BasicSymbolReadOperator::collect(MCGAL::Halfedge* seed,
                                      char* buffer,
                                      int& dataOffset,
                                      std::vector<MCGAL::Vertex*>& vertices,
                                      std::vector<MCGAL::Halfedge*>& halfedge,
                                      std::vector<MCGAL::Facet*>& facets) {
    std::deque<char> facetSymbol;
    std::deque<char> edgeSymbol;
    std::deque<MCGAL::Point> points;
    std::queue<int> gateQueue;
    gateQueue.push(seed->poolId());
    int current_version = MCGAL::BfsVersionMananger::current_version++;

    while (!gateQueue.empty()) {
        int hid = gateQueue.front();
        MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndexInSubPool(DEFAULT_MESH_ID, hid);
        gateQueue.pop();
        if (h->isVisited(current_version) || h->isRemoved()) {
            continue;
        }
        h->setVisited(current_version);

        if (!h->face()->isVisited(current_version)) {
            MCGAL::Facet* f = h->face();
            char symbol = readChar(buffer, dataOffset);
            facetSymbol.push_back(symbol);
            f->setVisited(current_version);
            if (symbol) {
                f->setSplittable();
                MCGAL::Point point = readPoint(buffer, dataOffset);
                f->setRemovedVertexPos(point);
                facets.push_back(f);
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

    current_version = MCGAL::BfsVersionMananger::current_version++;
    gateQueue.push(seed->poolId());

    while (!gateQueue.empty()) {
        int hid = gateQueue.front();
        MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndexInSubPool(DEFAULT_MESH_ID, hid);
        gateQueue.pop();
        if (h->isVisited(current_version) || h->isRemoved()) {
            continue;
        }
        h->setVisited(current_version);
        char symbol = readChar(buffer, dataOffset);
        edgeSymbol.push_back(symbol);
        if (symbol) {
            h->setAdded();
            halfedge.push_back(h);
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
    int i = 9;
}