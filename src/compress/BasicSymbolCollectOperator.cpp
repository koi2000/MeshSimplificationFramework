/*
 * @Author: koi
 * @Date: 2025-08-29 21:13:56
 * @Description:
 */

#include "BasicSymbolCollectOperator.h"
#include "../common/BufferUtils.h"
#include "BFSVersionManager.h"
#include "Facet.h"
#include "Halfedge.h"
#include "MeshUtils.h"
#include "Point.h"
#include "core.h"
#include <vector>

void BasicSymbolCollectOperator::collect(MCGAL::Halfedge* seed) {
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
    facetSymbolQueues.push_back(facetSym);
    edgeSymbolQueues.push_back(edgeSym);
    ipointQueues.push_back(ipoints);
    pointQueues.push_back(points);
}

int BasicSymbolCollectOperator::exportToBuffer(char* buffer, bool isEnableQuantization) {
    int offset = 0;

    for (int i = facetSymbolQueues.size() - 1; i >= 0; i--) {
        unsigned i_bitOffset = 0;
        std::deque<char> facetSym = facetSymbolQueues[i];
        std::deque<char> edgeSym = edgeSymbolQueues[i];
        std::deque<MCGAL::Point> points = pointQueues[i];
        while (!facetSym.empty()) {
            char sym = facetSym.front();
            facetSym.pop_front();
            // writeChar(buffer, offset, sym);
            // 268 -> 230
            writeBits(sym, 1, buffer, i_bitOffset, offset);
        }
        // 268 -> 211
        while (!edgeSym.empty()) {
            char sym = edgeSym.front();
            edgeSym.pop_front();
            // writeChar(buffer, offset, sym);
            writeBits(sym, 1, buffer, i_bitOffset, offset);
        }
        offset++;
    }
    char* outBlock = buffer;
    int outSize = 0;
    offset = 0;
    serializeCharPointer(buffer, offset, outBlock, outSize);
    writeCharPointer(buffer, offset, outBlock, outSize);

    for (int i = facetSymbolQueues.size() - 1; i >= 0; i--) {
        unsigned i_bitOffset = 0;
        std::deque<char> facetSym = facetSymbolQueues[i];
        std::deque<char> edgeSym = edgeSymbolQueues[i];
        // std::deque<MCGAL::Point> points = pointQueues[i];
        std::deque<MCGAL::PointInt> ipoints = ipointQueues[i];
        while (!ipoints.empty()) {
            MCGAL::PointInt pi = ipoints.front();
            ipoints.pop_front();
            // MCGAL::PointInt pi = mesh_->floatPosToInt(point);
            if (isEnableQuantization) {
                for (int i = 0; i < 3; i++) {
                    writeBits((uint32_t)pi[i], mesh_->i_nbQuantBits, buffer, i_bitOffset, offset);
                }
            } else {
                // writePoint(buffer, offset, point);
            }
        }
    }
    char* noutBlock = buffer;
    int noutSize = 0;
    serializeCharPointer(buffer + outSize, offset, noutBlock, noutSize);
    writeCharPointer(buffer, outSize, noutBlock, noutSize);

    return offset;
}

// int BasicSymbolCollectOperator::exportToBuffer(char* buffer, bool isEnableQuantization) {
//     int offset = 0;
//     char* outBlock = buffer;
//     for (int i = facetSymbolQueues.size() - 1; i >= 0; i--) {
//         unsigned i_bitOffset = 0;
//         std::deque<char> facetSym = facetSymbolQueues[i];
//         std::deque<char> edgeSym = edgeSymbolQueues[i];
//         std::deque<MCGAL::Point> points = pointQueues[i];
//         while (!facetSym.empty()) {
//             char sym = facetSym.front();
//             facetSym.pop_front();
//             // writeChar(buffer, offset, sym);
//             // 268 -> 230
//             writeBits(sym, 1, buffer, i_bitOffset, offset);
//             if (sym) {
//                 MCGAL::Point point = points.front();
//                 points.pop_front();
//                 MCGAL::PointInt pi = mesh_->floatPosToInt(point);
//                 if (isEnableQuantization) {
//                     for (int i = 0; i < 3; i++) {
//                         writeBits((uint32_t)pi[i], mesh_->i_nbQuantBits, buffer, i_bitOffset, offset);
//                     }
//                 } else {
//                     writePoint(buffer, offset, point);
//                 }
//             }
//         }
//         // 268 -> 211
//         while (!edgeSym.empty()) {
//             char sym = edgeSym.front();
//             edgeSym.pop_front();
//             // writeChar(buffer, offset, sym);
//             writeBits(sym, 1, buffer, i_bitOffset, offset);
//         }
//         offset++;
//     }
//     return offset;
// }