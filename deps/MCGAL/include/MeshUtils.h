#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include "BufferUtils.h"
#include "Matrix.h"
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

// static MCGAL::Halfedge* next_boundary(int ogroupId, MCGAL::Halfedge* boundary) {
//     MCGAL::Halfedge* nxt = boundary->next();
//     if (nxt->isBoundary()) {
//         return nxt;
//     }
//     nxt = boundary->next()->opposite()->next();
//     if (nxt->isBoundary()) {
//         return nxt;
//     }
//     for (MCGAL::Halfedge* hit : boundary->end_vertex()->halfedges()) {
//         if (hit->opposite() != boundary && hit->isBoundary() && hit->opposite()->face()->groupId() == ogroupId) {
//             return hit;
//         }
//         // if (hit->face()->groupId() != ogroupId && hit->opposite()->face()->groupId() == ogroupId) {
//         //     return hit;
//         // }
//     }
//     return nullptr;
// }

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
        int num_face_vertices = 3;
        std::vector<MCGAL::Vertex*> vts;
        for (int j = 0; j < num_face_vertices; ++j) {
            int vertex_index = ptr[j];
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
        // int nv = readInt(buffer, dataOffset);
        int nv = 3;
        uint32_t* f = new uint32_t[nv];
        // f[0] = nv;
        for (unsigned j = 0; j < nv; ++j) {
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

static std::vector<int> charPtrToVectorInt(char* data, int& dataOffset, int size) {
    std::vector<int> result(size);
    std::memcpy(result.data(), data + dataOffset, size * sizeof(int));
    dataOffset += size * sizeof(int);
    return result;
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

static MCGAL::Vector3 calculateFaceNormal(const MCGAL::Facet* face) {
    MCGAL::Vector3 normal = face->computeNormal();
    return normal;
}

// Matrix product.
static Matrix matProd(Matrix const& m, Matrix const& n) {
    float mT[3][3];
    mT[0][0] = m.r0()[0];
    mT[0][1] = m.r0()[1];
    mT[0][2] = m.r0()[2];
    mT[1][0] = m.r1()[0];
    mT[1][1] = m.r1()[1];
    mT[1][2] = m.r1()[2];
    mT[2][0] = m.r2()[0];
    mT[2][1] = m.r2()[1];
    mT[2][2] = m.r2()[2];

    float nT[3][3];
    nT[0][0] = n.r0()[0];
    nT[0][1] = n.r0()[1];
    nT[0][2] = n.r0()[2];
    nT[1][0] = n.r1()[0];
    nT[1][1] = n.r1()[1];
    nT[1][2] = n.r1()[2];
    nT[2][0] = n.r2()[0];
    nT[2][1] = n.r2()[1];
    nT[2][2] = n.r2()[2];

    float C[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++)
                C[i][j] += mT[i][k] * nT[k][j];
        }
    }

    return Matrix(C[0][0], C[0][1], C[0][2], C[1][0], C[1][1], C[1][2], C[2][0], C[2][1], C[2][2]);
}

static inline int signe(const float x) {
    return (x < 0) ? -1 : 1;
}

// Description : To find a bijection through a rotation transformation in 3D with only integer coordinates.
static std::vector<Matrix> compRotationMat(Vector3& T1, Vector3& T2, Vector3& normal) {
    Matrix rotMat(T1[0], T2[0], normal[0], T1[1], T2[1], normal[1], T1[2], T2[2], normal[2]);
    Matrix M = rotMat;

    Matrix D1(1, 0, 0, 0, 1, 0, 0, 0, 1);
    Matrix D2(0, 1, 0, 1, 0, 0, 0, 0, 1);
    Matrix D3(-1, 0, 0, 0, -1, 0, 0, 0, 1);
    Matrix D4(1, 0, 0, 0, 0, 1, 0, 1, 0);
    Matrix D5(1, 0, 0, 0, -1, 0, 0, 0, -1);

    std::vector<Matrix> S(15);

    // Verify in order to find the smallest rotation angle.
    if (abs((int)M.r0()[2]) > abs((int)M.r1()[2]))
        S[0] = D2;
    else
        S[0] = D1;

    M = matProd(S[0], M);

    if (M.r1()[2] < 0)
        S[1] = D3;
    else
        S[1] = D1;

    M = matProd(S[1], M);

    // Determine first rotation angle phi.
    float len = (M.r0()[2] * M.r0()[2]) + (M.r1()[2] * M.r1()[2]);
    float phi = len == 0 ? 0 : signe(-1 * M.r0()[2]) * acos((float)(M.r1()[2] / sqrt(len)));

    S[2] = Matrix(1, -tan(phi / 2), 0, 0, 1, 0, 0, 0, 1);
    S[3] = Matrix(1, 0, 0, sin(phi), 1, 0, 0, 0, 1);
    S[4] = S[2];

    Matrix R1inv(cos(phi), sin(phi), 0, -sin(phi), cos(phi), 0, 0, 0, 1);

    M = matProd(R1inv, M);

    if (abs((int)M.r1()[2]) > abs((int)M.r2()[2]))
        S[5] = D4;
    else
        S[5] = D1;

    M = matProd(S[5], M);

    if (M.r2()[2] < 0)
        S[6] = D5;
    else
        S[6] = D1;

    M = matProd(S[6], M);

    // Determine second rotation angle psi.
    len = (M.r1()[2] * M.r1()[2]) + (M.r2()[2] * M.r2()[2]);
    float psi = len == 0 ? 0 : signe(-1 * M.r1()[2]) * acos((float)(M.r2()[2] / sqrt(len)));

    S[7] = Matrix(1, 0, 0, 0, 1, -tan(psi / 2), 0, 0, 1);
    S[8] = Matrix(1, 0, 0, 0, 1, 0, 0, sin(psi), 1);
    S[9] = S[7];

    Matrix R2inv(1, 0, 0, 0, cos(psi), sin(psi), 0, -sin(psi), cos(psi));

    M = matProd(R2inv, M);

    if (abs((int)M.r0()[1]) > abs((int)M.r1()[1]))
        S[10] = D2;
    else
        S[10] = D1;

    M = matProd(S[10], M);

    if (M.r1()[1] < 0)
        S[11] = D3;
    else
        S[11] = D1;

    M = matProd(S[11], M);

    // Determine last rotation angle theta.
    len = (M.r0()[1] * M.r0()[1]) + (M.r1()[1] * M.r1()[1]);
    float theta = len == 0 ? 0 : signe(-1 * M.r0()[1]) * acos((float)(M.r1()[1] / sqrt(len)));

    S[12] = Matrix(1, -tan(theta / 2), 0, 0, 1, 0, 0, 0, 1);
    S[13] = Matrix(1, 0, 0, sin(theta), 1, 0, 0, 0, 1);
    S[14] = S[12];

    return S;
}

static void determineFrenetFrame(MCGAL::Vector3 v1, const MCGAL::Vector3 v2, MCGAL::Vector3& normal, MCGAL::Vector3& t1, MCGAL::Vector3& t2) {
    MCGAL::Vector3 gateVector = v1 - v2;
    // Determine t1.
    t1 = gateVector - normal * (gateVector | normal);
    float f_t1Length = sqrt(t1 | t1);
    if (f_t1Length != 0)
        t1 = t1 / f_t1Length;

    // Determine t2;
    t2 = normal % t1;
}

static Vector3i frenetRotation(Vector3i& Dist, Vector3& T1, Vector3& T2, Vector3& normal) {
    std::vector<Matrix> S = compRotationMat(T1, T2, normal);

    Vector3 u(Dist[0], Dist[1], Dist[2]);
    Matrix m_inter;

    // Procedure of the bijection.
    for (int i = 0; i < 15; i++) {
        if (i == 0 || i == 1 || i == 5 || i == 6 || i == 10 || i == 11)
            m_inter = S[i];
        else
            m_inter = Matrix(S[i].r0()[0], -S[i].r0()[1], -S[i].r0()[2], -S[i].r1()[0], S[i].r1()[1], -S[i].r1()[2], -S[i].r2()[0], -S[i].r2()[1],
                             S[i].r2()[2]);
        u = m_inter * u;

        int x = round(u[0]);
        int y = round(u[1]);
        int z = round(u[2]);

        u = Vector3(x, y, z);
    }

    return Vector3i(u[0], u[1], u[2]);
}

static Vector3i invFrenetRotation(Vector3i& Frenet, Vector3& T1, Vector3& T2, Vector3& normal) {
    std::vector<Matrix> S = compRotationMat(T1, T2, normal);

    Vector3 u(Frenet[0], Frenet[1], Frenet[2]);
    Matrix m_inter;

    for (int i = 14; i > -1; i--) {
        m_inter = S[i];
        u = m_inter * u;

        int x = round(u[0]);
        int y = round(u[1]);
        int z = round(u[2]);

        u = Vector3(x, y, z);
    }

    return Vector3i(u[0], u[1], u[2]);
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
        if (hit->opposite() != boundary && hit->isBoundary()) {
            return hit;
        }
    }
    return nullptr;
}

static MCGAL::Halfedge* next_boundary(int inner, int outer, MCGAL::Halfedge* boundary) {
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

static bool isPointInTriangle(const MCGAL::Point& A, const MCGAL::Point& B, const MCGAL::Point& C, const MCGAL::Point& P) {
    // 计算三个向量叉积
    MCGAL::Vector3 cross1 = crossProduct(A, B, P);
    MCGAL::Vector3 cross2 = crossProduct(B, C, P);
    MCGAL::Vector3 cross3 = crossProduct(C, A, P);

    // 检查是否所有叉积的z分量符号相同
    bool hasNeg = (cross1.z() < 0) || (cross2.z() < 0) || (cross3.z() < 0);
    bool hasPos = (cross1.z() > 0) || (cross2.z() > 0) || (cross3.z() > 0);

    return !(hasNeg && hasPos);  // 如果叉积符号不一致，返回false
}

}  // namespace MCGAL

#endif