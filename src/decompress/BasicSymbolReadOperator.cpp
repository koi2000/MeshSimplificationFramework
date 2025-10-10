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
#include "../rangecoder/qsmodel.h"
#include "../rangecoder/rangecod.h"
#include "BFSVersionManager.h"
#include "Configuration.h"
#include "Facet.h"
#include "MeshUtils.h"
#include "core.h"
#include <deque>
#include <sys/types.h>

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
    unsigned i_bitOffset = 0;
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
            // char symbol = readBits(1, buffer, i_bitOffset, dataOffset);
            facetSymbol.push_back(symbol);
            f->setVisited(current_version);
            if (symbol) {
                f->setSplittable();
                MCGAL::Point point;
                if (enableQuantization_) {
                    int p[3];
                    for (int i = 0; i < 3; i++) {
                        p[i] = readBits(12, buffer, i_bitOffset, dataOffset);
                    }
                    MCGAL::PointInt pi = MCGAL::PointInt(p[0], p[1], p[2]);
                    point[0] = bboxMin_[0] + (p[0] + 0.5) * f_quantStep_;
                    point[1] = bboxMin_[1] + (p[1] + 0.5) * f_quantStep_;
                    point[2] = bboxMin_[2] + (p[2] + 0.5) * f_quantStep_;
                    // point = MCGAL::intPosToFloat(MCGAL::PointInt(p[0], p[1], p[2]), bboxMin_, f_quantStep_);
                } else {
                    point = readPoint(buffer, dataOffset);
                }
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
        // char symbol = readBits(1, buffer, i_bitOffset, dataOffset);
        edgeSymbol.push_back(symbol);
        if (symbol) {
            h->setAdded();
            halfedge.push_back(h);
        }
        // if (!h->opposite()->isVisited(current_version)) {
        //     char symbol = readBits(1, buffer, i_bitOffset, dataOffset);
        //     if (h->isAdded()) {
        //         h->setAdded();
        //         halfedge.push_back(h);
        //     }
        //     edgeSymbol.push_back(symbol);
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
    // dataOffset++;
    int i = 9;
}

void BasicSymbolReadOperator::collect(std::vector<MCGAL::Halfedge*> seeds,
                                      char* buffer,
                                      int& dataOffset,
                                      std::vector<MCGAL::Vertex*>& vertices,
                                      std::vector<MCGAL::Halfedge*>& halfedge,
                                      std::vector<MCGAL::Facet*>& facets) {}

// void BasicSymbolReadOperator::collect(MCGAL::Halfedge* seed,
//                                       char* buffer,
//                                       int& dataOffset,
//                                       std::vector<MCGAL::Vertex*>& vertices,
//                                       std::vector<MCGAL::Halfedge*>& halfedge,
//                                       std::vector<MCGAL::Facet*>& facets) {
//     // 1) 读取符号位流的压缩块，并在本地指针上解码位流
//     char* bitBuf = readCompressedBlock(buffer, dataOffset);
//     int bitBufOffset = 0;
//     unsigned i_bitOffset = 0;
//     // 2) 读取 PointInt 的“按轮次”头与各轮的两组数据块，逐轮解码
//     int numRounds = readInt(buffer, dataOffset);
//     struct RoundData {
//         std::vector<int> g1;
//         std::vector<int> g2;
//         int points;
//     };
//     std::vector<RoundData> rounds;
//     rounds.reserve(std::max(0, numRounds));
//     for (int r = 0; r < numRounds; ++r) {
//         RoundData rd;
//         rd.points = readInt(buffer, dataOffset);
//         int g1Min = readInt(buffer, dataOffset);
//         unsigned char g1Bits = readChar(buffer, dataOffset);
//         int g2Min = readInt(buffer, dataOffset);
//         unsigned char g2Bits = readChar(buffer, dataOffset);
//         // 解码 Group1 of this round
//         {
//             char* rcPayload = readCompressedBlock(buffer, dataOffset);
//             if (g1Bits > 0 && rd.points > 0 && rcPayload) {
//                 size_t rcOff = 0;
//                 rangecoder rc;
//                 rc.p_data = rcPayload;
//                 rc.p_dataOffset = &rcOff;
//                 start_decoding(&rc);
//                 int alphabetSize = 1 << g1Bits;
//                 int lg_totf = g1Bits;
//                 qsmodel model;
//                 initqsmodel(&model, alphabetSize, lg_totf, 1 << (std::max(0, lg_totf - 1)), nullptr, 0);
//                 int count = rd.points * 2;
//                 rd.g1.reserve(count);
//                 for (int i = 0; i < count; ++i) {
//                     freq cf = decode_culshift(&rc, (freq)lg_totf);
//                     int sym = qsgetsym(&model, (int)cf);
//                     decode_update(&rc, (freq)(model.cf[sym + 1] - model.cf[sym]), (freq)model.cf[sym], (freq)1 << lg_totf);
//                     qsupdate(&model, sym);
//                     rd.g1.push_back(g1Min + sym);
//                 }
//                 done_decoding(&rc);
//                 deleteqsmodel(&model);
//             }
//             if (rcPayload)
//                 delete[] rcPayload;
//         }
//         // 解码 Group2 of this round
//         {
//             char* rcPayload = readCompressedBlock(buffer, dataOffset);
//             if (g2Bits > 0 && rd.points > 0 && rcPayload) {
//                 size_t rcOff = 0;
//                 rangecoder rc;
//                 rc.p_data = rcPayload;
//                 rc.p_dataOffset = &rcOff;
//                 start_decoding(&rc);
//                 int alphabetSize = 1 << g2Bits;
//                 int lg_totf = g2Bits;
//                 qsmodel model;
//                 initqsmodel(&model, alphabetSize, lg_totf, 1 << (std::max(0, lg_totf - 1)), nullptr, 0);
//                 int count = rd.points;
//                 rd.g2.reserve(count);
//                 for (int i = 0; i < count; ++i) {
//                     freq cf = decode_culshift(&rc, (freq)lg_totf);
//                     int sym = qsgetsym(&model, (int)cf);
//                     decode_update(&rc, (freq)(model.cf[sym + 1] - model.cf[sym]), (freq)model.cf[sym], (freq)1 << lg_totf);
//                     qsupdate(&model, sym);
//                     rd.g2.push_back(g2Min + sym);
//                 }
//                 done_decoding(&rc);
//                 deleteqsmodel(&model);
//             }
//             if (rcPayload)
//                 delete[] rcPayload;
//         }
//         rounds.push_back(std::move(rd));
//     }
//     std::deque<char> facetSymbol;
//     std::deque<char> edgeSymbol;
//     std::deque<MCGAL::Point> points;
//     std::queue<int> gateQueue;
//     gateQueue.push(seed->poolId());
//     int current_version = MCGAL::BfsVersionMananger::current_version++;
//     // 注意：位读取来自 bitBuf
//     while (!gateQueue.empty()) {
//         int hid = gateQueue.front();
//         MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndexInSubPool(DEFAULT_MESH_ID, hid);
//         gateQueue.pop();
//         if (h->isVisited(current_version) || h->isRemoved()) {
//             continue;
//         }
//         h->setVisited(current_version);
//         if (!h->face()->isVisited(current_version)) {
//             MCGAL::Facet* f = h->face();
//             // 从位流读取一个 facet 符号
//             char symbol = readBits(1, bitBuf, i_bitOffset, bitBufOffset);
//             facetSymbol.push_back(symbol);
//             f->setVisited(current_version);
//             if (symbol) {
//                 f->setSplittable();
//                 MCGAL::Point point;
//                 if (enableQuantization_) {
//                     // 从当前轮数据重建 (x,y,z)，按写端顺序消费
//                     static int roundCursor = 0;  // 顺序与写端相同：从最后一轮到第一轮
//                     if (roundCursor >= (int)rounds.size())
//                         roundCursor = (int)rounds.size() - 1;
//                     auto& rd = rounds[rounds.size() - 1];
//                     int p[3] = {0, 0, 0};
//                     if (!rd.g1.empty()) {
//                         p[0] = rd.g1.front();
//                         rd.g1.erase(rd.g1.begin());
//                     }
//                     if (!rd.g1.empty()) {
//                         p[1] = rd.g1.front();
//                         rd.g1.erase(rd.g1.begin());
//                     }
//                     if (!rd.g2.empty()) {
//                         p[2] = rd.g2.front();
//                         rd.g2.erase(rd.g2.begin());
//                     }
//                     if (rd.g1.empty() && rd.g2.empty() && rounds.size() > 0) {
//                         rounds.erase(rounds.end() - 1);
//                     }
//                     point[0] = bboxMin_[0] + (p[0] + 0.5) * f_quantStep_;
//                     point[1] = bboxMin_[1] + (p[1] + 0.5) * f_quantStep_;
//                     point[2] = bboxMin_[2] + (p[2] + 0.5) * f_quantStep_;
//                 } else {
//                     point = readPoint(buffer, dataOffset);
//                 }
//                 f->setRemovedVertexPos(point);
//                 facets.push_back(f);
//             }
//         }
//         MCGAL::Halfedge* hIt = h;
//         do {
//             MCGAL::Halfedge* hOpp = hIt->opposite();
//             if (!hOpp->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary()) {
//                 gateQueue.push(hOpp->poolId());
//             }
//             if (!hIt->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary()) {
//                 gateQueue.push(hIt->poolId());
//             }
//             hIt = hIt->next();
//         } while (hIt != h);
//     }
//     current_version = MCGAL::BfsVersionMananger::current_version++;
//     gateQueue.push(seed->poolId());
//     while (!gateQueue.empty()) {
//         int hid = gateQueue.front();
//         MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndexInSubPool(DEFAULT_MESH_ID, hid);
//         gateQueue.pop();
//         if (h->isVisited(current_version) || h->isRemoved()) {
//             continue;
//         }
//         h->setVisited(current_version);
//         // 从位流读取一个 edge 符号
//         char symbol = readBits(1, bitBuf, i_bitOffset, bitBufOffset);
//         edgeSymbol.push_back(symbol);
//         if (symbol) {
//             h->setAdded();
//             halfedge.push_back(h);
//         }
//         // if (!h->opposite()->isVisited(current_version)) {
//         //     char symbol = readBits(1, buffer, i_bitOffset, dataOffset);
//         //     if (h->isAdded()) {
//         //         h->setAdded();
//         //         halfedge.push_back(h);
//         //     }
//         //     edgeSymbol.push_back(symbol);
//         // }
//         MCGAL::Halfedge* hIt = h;
//         do {
//             MCGAL::Halfedge* hOpp = hIt->opposite();
//             if (!hOpp->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary()) {
//                 gateQueue.push(hOpp->poolId());
//             }
//             if (!hIt->isVisited(current_version) && !hIt->isBoundary() && !hOpp->isBoundary()) {
//                 gateQueue.push(hIt->poolId());
//             }
//             hIt = hIt->next();
//         } while (hIt != h);
//     }
//     // 位流按字节对齐步进（与写端一致）
//     bitBufOffset++;
//     if (bitBuf)
//         delete[] bitBuf;
// }