#include "include/SimpleEncoder.h"
#include "BufferUtils.h"
#include "VertexSplitNode.h"
#include "biops.h"
#include <fstream>
#include <unordered_set>

SimpleEncoder::SimpleEncoder(std::string path) {
    MCGAL::contextPool.initPoolSize(1);
    mesh.setMeshId(0);
    mesh.loadOFF(path);

    MCGAL::contextPool.add_property(mesh.meshId(), m_vpQuadrics);
    MCGAL::contextPool.add_property(mesh.meshId(), m_epError);
    MCGAL::contextPool.add_property(mesh.meshId(), m_epTargetPoints);
    MCGAL::contextPool.add_property(mesh.meshId(), m_epDirty);
    seed = mesh.faces()[0]->proxyHalfedge();
    buffer = new char[BUFFER_SIZE];
}

SimpleEncoder::~SimpleEncoder() {}

void SimpleEncoder::encodeVertexSymbol() {
    mesh.garbage_collection();
    std::deque<char> vertexSym;
    std::deque<MCGAL::VertexSplitNode*> nodes;
    std::queue<int> gateQueue;
    gateQueue.push(seed->face()->poolId());
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
    vertexSymbols.push_back(vertexSym);
    vertexSplitNodes.push_back(nodes);
}

void SimpleEncoder::fast_quardic_encode() {}

void SimpleEncoder::simple_encode() {
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