#include "include/Facet.h"
#include "include/Global.h"
#include "include/Halfedge.h"
#include "include/Vertex.h"
#include "stdio.h"
#include <fstream>
#include <iostream>

namespace MCGAL {

MCGAL::Point Facet::computeNormal() const {
    MCGAL::Halfedge* st = proxyHalfedge_;
    MCGAL::Halfedge* ed = st;
    std::vector<MCGAL::Point> points;
    do {
        points.push_back(st->vertex()->point());
        st = st->next();
    } while (st != ed);
    const Point& p1 = points[0];
    const Point& p2 = points[1];
    const Point& p3 = points[2];
    // 计算两个向量
    Point v1 = p2 - p1;
    Point v2 = p3 - p1;

    // 计算叉积得到法向量
    Point normal = v1.cross(v2);

    // 检查法向量是否为零（三点共线）
    float len = normal.length();
    if (len == 0) {
        throw std::string("The normal vector is zero, points are colinear or the face is degenerate");
    }

    // 归一化法向量
    normal.normalize();

    return normal;
}

Facet::Facet(const Facet& face) {
    this->proxyHalfedge_ = face.proxyHalfedge();
}

Facet::~Facet() {}

Facet::Facet(Halfedge* hit) {
    Halfedge* st(hit);
    Halfedge* ed(hit);
    std::vector<Halfedge*> edges;
    do {
        edges.push_back(st);
        st = st->next();
    } while (st != ed);
    this->reset(edges);
}

Facet* Facet::clone() {
    return new Facet(*this);
}

Facet::Facet(std::vector<Vertex*>& vs) {
    Halfedge* prev = nullptr;
    Halfedge* head = nullptr;
    for (int i = 0; i < vs.size(); i++) {
        vertices_.push_back(vs[i]);
        Vertex* nextv = vs[(i + 1) % vs.size()];
        Halfedge* hf = MCGAL::contextPool.allocateHalfedgeFromPool(DEFAULT_MESH_ID, vs[i], nextv);
        halfedges_.push_back(hf);
        hf->setFace(this);
        if (prev != NULL) {
            prev->setNext(hf);
        } else {
            head = hf;
        }
        if (i == vs.size() - 1) {
            hf->setNext(head);
        }
        prev = hf;
    }
}

void Facet::reset(std::vector<Vertex*>& vs) {
    Halfedge* prev = nullptr;
    Halfedge* head = nullptr;
    vertices_.clear();
    vertices_.shrink_to_fit();
    halfedges_.clear();
    halfedges_.shrink_to_fit();
    for (int i = 0; i < vs.size(); i++) {
        vertices_.push_back(vs[i]);
        Vertex* nextv = vs[(i + 1) % vs.size()];
        Halfedge* hf = MCGAL::contextPool.allocateHalfedgeFromPool(DEFAULT_MESH_ID, vs[i], nextv);
        halfedges_.push_back(hf);
        hf->setFace(this);
        if (prev != NULL) {
            prev->setNext(hf);
        } else {
            head = hf;
        }
        if (i == vs.size() - 1) {
            hf->setNext(head);
        }
        prev = hf;
    }
    this->proxyHalfedge_ = halfedges_[0];
}

void Facet::reset(std::vector<Vertex*>& vs, int meshId) {
    Halfedge* prev = nullptr;
    Halfedge* head = nullptr;
    for (int i = 0; i < vs.size(); i++) {
        vertices_.push_back(vs[i]);
        Vertex* nextv = vs[(i + 1) % vs.size()];
        Halfedge* hf = MCGAL::contextPool.allocateHalfedgeFromPool(meshId, vs[i], nextv);
        halfedges_.push_back(hf);
        hf->setFace(this);
        if (prev != NULL) {
            prev->setNext(hf);
        } else {
            head = hf;
        }
        if (i == vs.size() - 1) {
            hf->setNext(head);
        }
        prev = hf;
    }
    this->proxyHalfedge_ = halfedges_[0];
}

void Facet::reset(Halfedge* h) {
    Halfedge* st = h;
    Halfedge* ed = h;
    std::vector<Halfedge*> edges;
    this->halfedges_.clear();
    this->vertices_.clear();
    do {
        this->halfedges_.push_back(st);
        this->vertices_.push_back(st->vertex());
        st->setFace(this);
        st = st->next();
    } while (st != ed);
    this->proxyHalfedge_ = h;
}

/**
 * lazy init
 */
void Facet::reset_without_init(Halfedge* h) {
    Halfedge* st = h;
    Halfedge* ed = h;
    this->proxyHalfedge_ = h;

    do {
        st->setFace(this);
        st = st->next();
    } while (st != ed);
}

void Facet::reset(std::vector<Halfedge*>& hs) {
    this->halfedges_ = hs;
    this->vertices_.clear();
    this->vertices_.reserve(10);
    for (int i = 0; i < hs.size(); i++) {
        this->vertices_.push_back(hs[i]->vertex());
        hs[i]->setFace(this);
    }
}

std::vector<Vertex*> Facet::getVertices() {
    std::vector<Vertex*> vs;
    Halfedge* st = proxyHalfedge_;
    Halfedge* ed = proxyHalfedge_;
    do {
        vs.push_back(st->vertex());
        st = st->next();
    } while (st != ed);
    return vs;
}

void Facet::print() {
    printf("totally %ld vertices_:\n", vertices_.size());
    int idx = 0;
    for (Vertex* v : vertices_) {
        printf("%d:\t", idx++);
        v->print();
    }
}

void Facet::print_off() {
    printf("OFF\n%ld 1 0\n", vertices_.size());
    for (Vertex* v : vertices_) {
        v->print();
    }
    printf("%ld\t", vertices_.size());
    for (int i = 0; i < vertices_.size(); i++) {
        printf("%d ", i);
    }
    printf("\n");
}

void Facet::dumpTo(std::string path) {
    std::ofstream offFile(path);
    offFile << "OFF\n";
    offFile << vertices_.size() << " " << 1 << " 0\n";
    offFile << "\n";

    for (Vertex* v : vertices_) {
        offFile << v->x() << " " << v->y() << " " << v->z() << "\n";
    }
    offFile << vertices_.size() << " ";
    for (int i = 0; i < vertices_.size(); i++) {
        offFile << i << " ";
    }
    // printf("\n");
}

bool Facet::equal(const Facet& rhs) const {
    return this->poolId_ == rhs.poolId();
}
bool Facet::operator==(const Facet& rhs) const {
    return this->equal(rhs);
}

Point Facet::getRemovedVertexPos() const {
    return removedVertexPos;
};

void Facet::setRemovedVertexPos(Point p) {
    removedVertexPos = p;
};

int Facet::facet_degree() {
    return vertices_.size();
}

}  // namespace MCGAL
