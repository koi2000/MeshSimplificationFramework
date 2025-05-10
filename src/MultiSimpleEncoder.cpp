#include "include/MultiSimpleEncoder.h"
#include "BufferUtils.h"
#include "VertexSplitNode.h"
#include "biops.h"
#include <fstream>
#include <unordered_set>

MultiSimpleEncoder::MultiSimpleEncoder(std::string path) {
    MCGAL::contextPool.initPoolSize(1);
    mesh.setMeshId(0);
    mesh.loadOFF(path);
    splitter.loadMesh(&mesh);
    splitter.split(50);
    seeds = splitter.exportSeeds();

    MCGAL::contextPool.add_property(mesh.meshId(), m_vpQuadrics);
    MCGAL::contextPool.add_property(mesh.meshId(), m_epError);
    MCGAL::contextPool.add_property(mesh.meshId(), m_epTargetPoints);
    MCGAL::contextPool.add_property(mesh.meshId(), m_epDirty);
    buffer = new char[BUFFER_SIZE];
}

MultiSimpleEncoder::~MultiSimpleEncoder() {}

void MultiSimpleEncoder::encode_group(int groupId, size_t uiRemainedVertexNum) {
    SimplifyVertexTo(uiRemainedVertexNum, groupId, seeds[groupId]);
}

void MultiSimpleEncoder::encodeVertexSymbol(int groupId) {
    mesh.garbage_collection();
    std::deque<char> vertexSym;
    std::deque<MCGAL::VertexSplitNode*> nodes;
    std::queue<int> gateQueue;
    gateQueue.push(seeds[groupId]->face()->poolId());
    int count = 0;
    while (!gateQueue.empty()) {
        int fid = gateQueue.front();
        MCGAL::Facet* f = MCGAL::contextPool.getFacetByIndex(mesh.meshId(), fid);
        gateQueue.pop();
        if (f->isProcessed()) {
            continue;
        }
        int vpoolId = -1;
        f->setProcessedFlag();

        MCGAL::Halfedge* st = f->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        MCGAL::Vertex* minVid = st->vertex();
        MCGAL::Halfedge* hit = st;
        do {
            if (st->vertex()->point() > minVid->point()) {
                hit = st;
                minVid = st->vertex();
            }
            st = st->next();
        } while (st != ed);

        st = hit;
        ed = st;
        do {
            if (!st->vertex()->isProcessed() && st->vertex()->isCollapsed()) {
                vertexSym.push_back(1);
                st->vertex()->setProcessed();
                nodes.push_back(st->vertex()->vsplitNode());
            } else if (!st->vertex()->isProcessed()) {
                vertexSym.push_back(0);
                st->vertex()->setProcessed();
            }

            if (!st->opposite()->face()->isProcessed()) {
                gateQueue.push(st->opposite()->face()->poolId());
            }
            st = st->next();
        } while (st != ed);
    }
    multiVertexSymbols[groupId].push_back(vertexSym);
    multiVertexSplitNodes[groupId].push_back(nodes);
}

void MultiSimpleEncoder::encodeBoundary() {}

void MultiSimpleEncoder::encode() {}

// void MultiSimpleEncoder::encode_group(int groupId) {
//     std::queue<int> gateQueue;
//     gateQueue.push(seeds[groupId]->poolId());
//     int removedCount = 0;
//     while (!gateQueue.empty()) {
//         int hid = gateQueue.front();
//         MCGAL::Halfedge* h = MCGAL::contextPool.getHalfedgeByIndex(mesh.meshId(), hid);
//         gateQueue.pop();
//         if (h->isProcessed()) {
//             continue;
//         }
//         h->setProcessed();
//         if (isRemovable(h) && mesh.is_collapse_ok(h)) {
//             MCGAL::Vertex* v = mesh.halfedge_collapse(h);
//             for (MCGAL::Halfedge* h : v->halfedges()) {
//                 if (!h->isProcessed())
//                     gateQueue.push(h->poolId());
//             }
//         } else {
//             MCGAL::Vertex* v = h->vertex();
//             for (MCGAL::Halfedge* h : v->halfedges()) {
//                 if (!h->isProcessed())
//                     gateQueue.push(h->poolId());
//             }
//         }
//     }
// }

void MultiSimpleEncoder::simple_encode() {
    std::vector<MCGAL::Facet*>& faces = mesh.faces();
    int target = faces.size() / 50;
    int idx = 0;
    while (faces.size() > target) {
        if (idx > 20) {
            break;
        }
        for (MCGAL::Facet*& f : faces) {
            if (f->isRemoved()) {
                continue;
            }
            MCGAL::Halfedge* st = f->proxyHalfedge();
            MCGAL::Halfedge* ed = st;
            do {
                if (st->isRemoved() || st->vertex()->isRemoved() || st->end_vertex()->isRemoved() || st->vertex()->isProcessed() || st->end_vertex()->isProcessed()) {
                    // if (st->isRemoved() || st->vertex()->isRemoved() || st->end_vertex()->isRemoved()) {
                    break;
                }
                MCGAL::Vertex* v1 = st->vertex();
                MCGAL::Vertex* v2 = st->end_vertex();
                MCGAL::Point pTarget((v1->point().x() + v2->point().x()) / 2, (v1->point().y() + v2->point().y()) / 2, (v1->point().z() + v2->point().z()) / 2);
                if (mesh.is_collapse_ok(st) && mesh.is_collapse_ok(st->opposite()) && !IsFlipped(st, pTarget) && !IsFlipped(st->opposite(), pTarget)) {
                    // if (st->length() < 0.002) {
                    mesh.halfedge_collapse(st);
                    break;
                }
                st = st->next();
            } while (st != ed);
        }
        std::string path = "./res_" + std::to_string(idx++) + ".off";
        mesh.resetState();
        mesh.dumpto_oldtype(path);
    }
}