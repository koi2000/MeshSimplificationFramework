#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include "BufferUtils.h"
#include "Mesh.h"
#include "Point.h"
#include "Vertex.h"
#include "core.h"
#include "symetric_matrix.h"
#include <cstdint>
#include <memory>

namespace MCGAL {

static double VertexError(const SymetricMatrix& q, double x, double y, double z) {
    return q[0] * x * x + 2 * q[1] * x * y + 2 * q[2] * x * z + 2 * q[3] * x + q[4] * y * y + 2 * q[5] * y * z + 2 * q[6] * y + q[7] * z * z +
           2 * q[8] * z + q[9];
}

static double CalculateError(SymetricMatrix v0m, SymetricMatrix v1m, MCGAL::Halfedge* edge, MCGAL::Point& ptResult) {
    MCGAL::Halfedge* h0 = edge;
    MCGAL::Vertex* v0 = h0->vertex();
    MCGAL::Vertex* v1 = h0->end_vertex();
    SymetricMatrix q = v0m + v1m;

    double dError = 0;
    // The number for Det, is the index in q
    double det = q.Det(0, 1, 2, 1, 4, 5, 2, 5, 7);
    if (fabs(det) > 1e-6 && !edge->isBoundary()) {
        ptResult[0] = -1 / det * (q.Det(1, 2, 3, 4, 5, 6, 5, 7, 8));  // vx = A41/det(q_delta)
        ptResult[1] = 1 / det * (q.Det(0, 2, 3, 1, 5, 6, 2, 7, 8));   // vy = A42/det(q_delta)
        ptResult[2] = -1 / det * (q.Det(0, 1, 3, 1, 4, 6, 2, 5, 8));  // vz = A43/det(q_delta)

        dError = VertexError(q, ptResult[0], ptResult[1], ptResult[2]);
    } else {
        // find if v0, or v1, or midpoint
        MCGAL::Point pt0 = v0->point();
        MCGAL::Point pt1 = v1->point();
        MCGAL::Point pt2 = (pt0 + pt1) / 2;
        double error0 = VertexError(q, pt0[0], pt0[1], pt0[2]);
        double error1 = VertexError(q, pt1[0], pt1[1], pt1[2]);
        double error2 = VertexError(q, pt2[0], pt2[1], pt2[2]);
        dError = std::min(error0, std::min(error1, error2));
        if (error0 == dError) {
            ptResult = pt0;
        }
        if (error1 == dError) {
            ptResult = pt1;
        }
        if (error2 == dError) {
            ptResult = pt2;
        }
    }

    return dError;
}

static bool IsFlipped(MCGAL::Halfedge* edge, const MCGAL::Point& ptTarget) {
    MCGAL::Vertex* v0 = edge->vertex();
    MCGAL::Vertex* v1 = edge->end_vertex();
    // 检查坍缩后是否会导致重叠面片或非流形网格
    // 坍缩后确保其他点的邻居数不能少于三个

    for (MCGAL::Halfedge* h : v0->halfedges()) {
        if (h->face()->isRemoved()) {
            continue;
        }
        std::vector<MCGAL::Vertex*> vs;
        MCGAL::Halfedge* st = h->face()->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        do {
            vs.push_back(st->vertex());
            st = st->next();
        } while (st != ed);
        if (vs[0] == v1 || vs[1] == v1 || vs[2] == v1) {
            continue;
        }
        int idxV0 = 0;
        for (int i = 0; i < vs.size(); i++) {
            MCGAL::Vertex* v = vs[i];
            if (vs[i] == v0) {
                idxV0 = i;
            }
        }
        MCGAL::Point pt0 = v0->point();
        MCGAL::Point pt1 = vs[(idxV0 + 1) % 3]->point();
        MCGAL::Point pt2 = vs[(idxV0 + 2) % 3]->point();

        MCGAL::Point dir1 = pt1 - ptTarget;
        dir1.normalize();
        MCGAL::Point dir2 = pt2 - ptTarget;
        dir2.normalize();

        // The angle below can be adjusted, but now is enough
        // if the angle between dir1 and dir2 small than 2.6 angle, return true
        if (fabs(dir1.dot(dir2)) > 0.999)
            return true;
        MCGAL::Point normold;
        try {
            normold = h->face()->computeNormal();
        } catch (std::string exp) { return true; }
        MCGAL::Point norm = dir1.cross(dir2);
        norm.normalize();

        // if the angle between normold and norm large than 78.5 angle, return true
        if ((normold.dot(norm)) < 0.2)
            return true;
    }
    return false;
}

static MCGAL::Halfedge* next_boundary(int ogroupId, MCGAL::Halfedge* boundary) {
    MCGAL::Halfedge* nxt = boundary->next();
    if (nxt->isBoundary()) {
        return nxt;
    }
    nxt = boundary->next()->opposite()->next();
    if (nxt->isBoundary()) {
        return nxt;
    }
    for (MCGAL::Halfedge* hit : boundary->end_vertex()->halfedges()) {
        if (hit->opposite() != boundary && hit->isBoundary() && hit->opposite()->face()->groupId() == ogroupId) {
            return hit;
        }
        // if (hit->face()->groupId() != ogroupId && hit->opposite()->face()->groupId() == ogroupId) {
        //     return hit;
        // }
    }
    return nullptr;
}

static Halfedge* find_prev(Halfedge* h) {
    Halfedge* g = h;
    while (g->next() != h) {
        g = g->next();
    }
    return g;
}

static MCGAL::Vector3 crossProduct(const MCGAL::Point& A, const MCGAL::Point& B, const MCGAL::Point& P) {
    MCGAL::Vector3 AB = B - A;
    MCGAL::Vector3 AP = P - A;
    return AB.cross(AP);
}

static bool compareVertex(MCGAL::Vertex* a, MCGAL::Vertex* b) {
    if (a->x() != b->x()) {
        return a->x() < b->x();
    }
    if (a->y() != b->y()) {
        return a->y() < b->y();
    }
    return a->z() < b->z();
}

static std::shared_ptr<MCGAL::Mesh> buildFromBuffer(int meshId, std::deque<MCGAL::Point>* p_pointDeque, std::deque<uint32_t*>* p_faceDeque) {
    std::shared_ptr<MCGAL::Mesh> mesh = std::make_shared<MCGAL::Mesh>();
    mesh->setMeshId(meshId);
    std::vector<MCGAL::Vertex*> vertices;
    for (std::size_t i = 0; i < p_pointDeque->size(); ++i) {
        float x, y, z;
        MCGAL::Point p = p_pointDeque->at(i);
        MCGAL::Vertex* vt = MCGAL::contextPool.allocateVertexFromPool(meshId, p);
        mesh->add_vertex(vt);
        vertices.push_back(vt);
    }
    for (int i = 0; i < p_faceDeque->size(); ++i) {
        uint32_t* ptr = p_faceDeque->at(i);
        int num_face_vertices = ptr[0];
        std::vector<MCGAL::Vertex*> vts;
        for (int j = 0; j < num_face_vertices; ++j) {
            int vertex_index = ptr[j + 1];
            vts.push_back(vertices[vertex_index]);
        }
        MCGAL::Facet* face = MCGAL::contextPool.allocateFaceFromPool(meshId, vts);
        mesh->add_face(face);
    }
    vertices.clear();
    return mesh;
}

static std::shared_ptr<MCGAL::Mesh> readBaseMesh(int meshId, char* buffer, int& dataOffset) {
    unsigned i_nbVerticesBaseMesh = readInt(buffer, dataOffset);
    unsigned i_nbFacesBaseMesh = readInt(buffer, dataOffset);
    MCGAL::contextPool.registerPool(i_nbVerticesBaseMesh * 5, i_nbFacesBaseMesh * 15, i_nbFacesBaseMesh * 5);
    std::deque<MCGAL::Point>* p_pointDeque = new std::deque<MCGAL::Point>();
    std::deque<uint32_t*>* p_faceDeque = new std::deque<uint32_t*>();
    for (unsigned i = 0; i < i_nbVerticesBaseMesh; ++i) {
        MCGAL::Point pos = readPoint(buffer, dataOffset);
        p_pointDeque->push_back(pos);
    }
    for (unsigned i = 0; i < i_nbFacesBaseMesh; ++i) {
        int nv = readInt(buffer, dataOffset);
        uint32_t* f = new uint32_t[nv + 1];
        f[0] = nv;
        for (unsigned j = 1; j < nv + 1; ++j) {
            f[j] = readInt(buffer, dataOffset);
        }
        p_faceDeque->push_back(f);
    }
    auto mesh = buildFromBuffer(meshId, p_pointDeque, p_faceDeque);

    for (unsigned i = 0; i < p_faceDeque->size(); ++i) {
        delete[] p_faceDeque->at(i);
    }
    delete p_faceDeque;
    delete p_pointDeque;
    return mesh;
}

static PointInt floatPosToInt(Point p, MCGAL::Point bboxMin, int f_quantStep) {
    return PointInt((p[0] - bboxMin[0]) / f_quantStep, (p[1] - bboxMin[1]) / f_quantStep, (p[2] - bboxMin[2]) / f_quantStep);
}

static inline MCGAL::Point intPosToFloat(PointInt p, MCGAL::Point bboxMin, int f_quantStep) {
    return Point(bboxMin[0] + (p[0] + 0.5) * f_quantStep, bboxMin[1] + (p[1] + 0.5) * f_quantStep, bboxMin[2] + (p[2] + 0.5) * f_quantStep);
}

static std::shared_ptr<MCGAL::Mesh> readBaseMeshWithQuantization(int meshId, char* buffer, int& dataOffset, bool enableQuantization = true) {
    char nbQuantBits;
    MCGAL::Point bboxMin;
    float f_quantStep;
    if (enableQuantization) {
        nbQuantBits = readChar(buffer, dataOffset);
        bboxMin = readPoint(buffer, dataOffset);
        f_quantStep = readFloat(buffer, dataOffset);
    }

    unsigned i_nbVerticesBaseMesh = readInt(buffer, dataOffset);
    unsigned i_nbFacesBaseMesh = readInt(buffer, dataOffset);
    MCGAL::contextPool.registerPool(i_nbVerticesBaseMesh * 5, i_nbFacesBaseMesh * 15, i_nbFacesBaseMesh * 5);
    std::deque<MCGAL::Point>* p_pointDeque = new std::deque<MCGAL::Point>();
    std::deque<uint32_t*>* p_faceDeque = new std::deque<uint32_t*>();

    unsigned i_bitOffset = 0;
    for (unsigned i = 0; i < i_nbVerticesBaseMesh; ++i) {
        MCGAL::Point pos;
        if (enableQuantization) {
            int p[3];
            for (int i = 0; i < 3; i++) {
                p[i] = readBits(nbQuantBits, buffer, i_bitOffset, dataOffset);
            }
            MCGAL::PointInt pi = PointInt(p[0], p[1], p[2]);
            pos[0] = bboxMin[0] + (p[0] + 0.5) * f_quantStep;
            pos[1] = bboxMin[1] + (p[1] + 0.5) * f_quantStep;
            pos[2] = bboxMin[2] + (p[2] + 0.5) * f_quantStep;
            // pos = intPosToFloat(pi, bboxMin, f_quantStep);
        } else {
            pos = readPoint(buffer, dataOffset);
        }

        p_pointDeque->push_back(pos);
    }
    i_bitOffset = 0;
    dataOffset++;
    
    char connBit = readChar(buffer, dataOffset);
    // uint32_t ddd = readBits(connBit, buffer, i_bitOffset, --dataOffset);
    for (unsigned i = 0; i < i_nbFacesBaseMesh; ++i) {
        // int nv = readInt(buffer, dataOffset);
        uint32_t* f = new uint32_t[3 + 1];
        f[0] = 3;
        for (unsigned j = 1; j < 3 + 1; ++j) {
            f[j] = readBits(connBit, buffer, i_bitOffset, dataOffset);
        }
        p_faceDeque->push_back(f);
    }
    auto mesh = buildFromBuffer(meshId, p_pointDeque, p_faceDeque);
    if (enableQuantization) {
        mesh->bboxMin = bboxMin;
        mesh->f_quantStep = f_quantStep;
    }
    for (unsigned i = 0; i < p_faceDeque->size(); ++i) {
        delete[] p_faceDeque->at(i);
    }
    delete p_faceDeque;
    delete p_pointDeque;
    return mesh;
}

static inline void remove_tip(Halfedge* h) {
    h->setNext(h->next()->opposite()->next());
}

/**
 * 还是顺序的问题 明天定位一下顺序的问题处在哪里
 */
static Halfedge* join_face(Halfedge* h) {
    if (h->poolId() == 73932) {
        int i = 0;
    }
    Halfedge* hprev = find_prev(h);
    Halfedge* gprev = find_prev(h->opposite());
    remove_tip(hprev);
    remove_tip(gprev);
    h->opposite()->setRemoved();
    h->setRemoved();
    if (gprev->face()->poolId() == 24644 || hprev->face()->poolId() == 24644) {
        int i = 0;
    }
    if (gprev->face()->isSplittable()) {
        hprev->face()->setRemoved();
        gprev->face()->reset_without_init(gprev);
    } else {
        gprev->face()->setRemoved();
        hprev->face()->reset_without_init(hprev);
    }
    return hprev;
}

}  // namespace MCGAL

#endif