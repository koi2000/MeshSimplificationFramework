/*
 * @Author: koi
 * @Date: 2025-08-29 21:13:56
 * @Description:
 */

#include "BasicSymbolCollectOperator.h"
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

void BasicSymbolCollectOperator::collect(std::vector<MCGAL::Halfedge*> seeds) {
    collect(seeds[0]);
}

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

// int BasicSymbolCollectOperator::exportToBuffer(char* buffer, bool isEnableQuantization) {
//     int offset = 0;
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
//     // 将前面写入的位流打包压缩
//     int bitDataSize = offset;
//     char* outBlock = buffer;
//     int outSize = 0;
//     serializeCharPointer(buffer, bitDataSize, outBlock, outSize);
//     offset = 0;
//     writeCharPointer(buffer, offset, outBlock, outSize);
//     // 使用 rangecoder 对 PointInt 进行“按轮次”分组熵编码：每轮独立的 Group1=(x,y)、Group2=(z)
//     if (isEnableQuantization) {
//         const int numRounds = (int)ipointQueues.size();
//         writeInt(buffer, offset, numRounds);
//         auto ceil_log2 = [](int v) -> int {
//             if (v <= 1)
//                 return 0;
//             int b = 0;
//             v -= 1;
//             while (v > 0) {
//                 b++;
//                 v >>= 1;
//             }
//             return b;
//         };
//         for (int ridx = (int)ipointQueues.size() - 1; ridx >= 0; --ridx) {
//             std::deque<MCGAL::PointInt> ipoints = ipointQueues[ridx];
//             const int numPoints = (int)ipoints.size();
//             // 计算两组统计信息
//             int g1Min = 0, g1Max = 0, g2Min = 0, g2Max = 0;
//             bool g1Init = false, g2Init = false;
//             std::vector<int> group1;
//             group1.reserve(std::max(0, numPoints * 2));
//             std::vector<int> group2;
//             group2.reserve(std::max(0, numPoints));
//             while (!ipoints.empty()) {
//                 MCGAL::PointInt p = ipoints.front();
//                 ipoints.pop_front();
//                 int vx = p.v[0], vy = p.v[1], vz = p.v[2];
//                 group1.push_back(vx);
//                 group1.push_back(vy);
//                 group2.push_back(vz);
//                 if (!g1Init) {
//                     g1Min = g1Max = vx;
//                     g1Init = true;
//                 }
//                 if (vx < g1Min)
//                     g1Min = vx;
//                 if (vx > g1Max)
//                     g1Max = vx;
//                 if (vy < g1Min)
//                     g1Min = vy;
//                 if (vy > g1Max)
//                     g1Max = vy;
//                 if (!g2Init) {
//                     g2Min = g2Max = vz;
//                     g2Init = true;
//                 }
//                 if (vz < g2Min)
//                     g2Min = vz;
//                 if (vz > g2Max)
//                     g2Max = vz;
//             }
//             int g1Bits = ceil_log2(g1Init ? (g1Max - g1Min + 1) : 0);
//             int g2Bits = ceil_log2(g2Init ? (g2Max - g2Min + 1) : 0);
//             g1Bits = std::min(g1Bits, 6);
//             g2Bits = std::min(g2Bits, 6);
//             // 写入每轮头部
//             writeInt(buffer, offset, numPoints);
//             writeInt(buffer, offset, g1Min);
//             writeChar(buffer, offset, (unsigned char)g1Bits);
//             writeInt(buffer, offset, g2Min);
//             writeChar(buffer, offset, (unsigned char)g2Bits);
//             // 编码 Group1
//             {
//                 const int g1Count = (int)group1.size();
//                 if (g1Bits == 0 || g1Count == 0) {
//                     char* emptyBlock = nullptr;
//                     int emptySize = 0;
//                     serializeCharPointer(nullptr, 0, emptyBlock, emptySize);
//                     writeCharPointer(buffer, offset, emptyBlock, emptySize);
//                 } else {
//                     const int alphabetSize = 1 << g1Bits;
//                     const int lg_totf = g1Bits;
//                     std::vector<int> symbols;
//                     symbols.reserve(g1Count);
//                     for (int v : group1)
//                         symbols.push_back(v - g1Min);
//                     size_t rcOffset = 0;
//                     std::vector<char> rcBuf(std::max<size_t>(symbols.size() * 2 + 1024, 4096));
//                     rangecoder rc;
//                     rc.p_data = rcBuf.data();
//                     rc.p_dataOffset = &rcOffset;
//                     start_encoding(&rc, 0, 0);
//                     qsmodel model;
//                     initqsmodel(&model, alphabetSize, lg_totf, 1 << (std::max(0, lg_totf - 1)), nullptr, 1);
//                     for (int sym : symbols) {
//                         if (sym < 0)
//                             sym = 0;
//                         if (sym >= alphabetSize)
//                             sym = alphabetSize - 1;
//                         int sy_f = 0, lt_f = 0;
//                         qsgetfreq(&model, sym, &sy_f, &lt_f);
//                         encode_shift(&rc, (freq)sy_f, (freq)lt_f, (freq)lg_totf);
//                         qsupdate(&model, sym);
//                     }
//                     done_encoding(&rc);
//                     deleteqsmodel(&model);
//                     char* rcBlock = nullptr;
//                     int rcBlockSize = 0;
//                     serializeCharPointer(rcBuf.data(), (int)rcOffset, rcBlock, rcBlockSize);
//                     writeCharPointer(buffer, offset, rcBlock, rcBlockSize);
//                 }
//             }
//             // 编码 Group2
//             {
//                 const int g2Count = (int)group2.size();
//                 if (g2Bits == 0 || g2Count == 0) {
//                     char* emptyBlock = nullptr;
//                     int emptySize = 0;
//                     serializeCharPointer(nullptr, 0, emptyBlock, emptySize);
//                     writeCharPointer(buffer, offset, emptyBlock, emptySize);
//                 } else {
//                     const int alphabetSize = 1 << g2Bits;
//                     const int lg_totf = g2Bits;
//                     std::vector<int> symbols;
//                     symbols.reserve(g2Count);
//                     for (int v : group2)
//                         symbols.push_back(v - g2Min);
//                     size_t rcOffset = 0;
//                     std::vector<char> rcBuf(std::max<size_t>(symbols.size() * 2 + 1024, 4096));
//                     rangecoder rc;
//                     rc.p_data = rcBuf.data();
//                     rc.p_dataOffset = &rcOffset;
//                     start_encoding(&rc, 0, 0);
//                     qsmodel model;
//                     initqsmodel(&model, alphabetSize, lg_totf, 1 << (std::max(0, lg_totf - 1)), nullptr, 1);
//                     for (int sym : symbols) {
//                         if (sym < 0)
//                             sym = 0;
//                         if (sym >= alphabetSize)
//                             sym = alphabetSize - 1;
//                         int sy_f = 0, lt_f = 0;
//                         qsgetfreq(&model, sym, &sy_f, &lt_f);
//                         encode_shift(&rc, (freq)sy_f, (freq)lt_f, (freq)lg_totf);
//                         qsupdate(&model, sym);
//                     }
//                     done_encoding(&rc);
//                     deleteqsmodel(&model);
//                     char* rcBlock = nullptr;
//                     int rcBlockSize = 0;
//                     serializeCharPointer(rcBuf.data(), (int)rcOffset, rcBlock, rcBlockSize);
//                     writeCharPointer(buffer, offset, rcBlock, rcBlockSize);
//                 }
//             }
//         }
//     } else {
//         writeInt(buffer, offset, 0);
//     }
//     return offset;
// }

int BasicSymbolCollectOperator::exportToBuffer(char* buffer, bool isEnableQuantization) {
    int offset = 0;
    char* outBlock = buffer;
    for (int i = facetSymbolQueues.size() - 1; i >= 0; i--) {
        unsigned i_bitOffset = 0;
        std::deque<char> facetSym = facetSymbolQueues[i];
        std::deque<char> edgeSym = edgeSymbolQueues[i];
        std::deque<MCGAL::Point> points = pointQueues[i];
        while (!facetSym.empty()) {
            char sym = facetSym.front();
            facetSym.pop_front();
            writeChar(buffer, offset, sym);
            // 268 -> 230
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
        // 268 -> 211
        while (!edgeSym.empty()) {
            char sym = edgeSym.front();
            edgeSym.pop_front();
            writeChar(buffer, offset, sym);
            // writeBits(sym, 1, buffer, i_bitOffset, offset);
        }
        // offset++;
    }
    return offset;
}