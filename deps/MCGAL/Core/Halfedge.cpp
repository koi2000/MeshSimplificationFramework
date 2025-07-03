#include "include/Halfedge.h"
#include "include/Facet.h"
#include "include/Vertex.h"

namespace MCGAL {

// create a new half edge, setup the opposite of this half edge if needed
Halfedge::Halfedge(Vertex* v1, Vertex* v2) {
    vertex_ = v1;
    end_vertex_ = v2;
    vertex_->halfedges().push_back(this);
    for (Halfedge* h : v2->halfedges()) {
        if (h->end_vertex_ == v1) {
            if (h->opposite()) {
                printf("create half edge:\n");
                v1->print();
                v2->print();
                h->opposite()->face()->print_off();
            }
            assert(h->opposite() == NULL);
            h->setOpposite(this);
            this->setOpposite(h);
            break;
        }
    }
}

void Halfedge::reset_without_init(Vertex* v1, Vertex* v2) {
    vertex_ = v1;
    end_vertex_ = v2;
}

void Halfedge::reset(Vertex* v1, Vertex* v2) {
    vertex_ = v1;
    end_vertex_ = v2;
    vertex_->halfedges().push_back(this);
    // end_vertex_->opposite_half_edges.insert(this);
    // in case this is the second half edge
    for (Halfedge* h : v2->halfedges()) {
        if (h->end_vertex_ == v1) {
            if (h->opposite()) {
                printf("create half edge:\n");
                v1->print();
                v2->print();
                h->opposite()->face()->print_off();
                int idx = 0;
                for (Halfedge* hit : v1->halfedges()) {
                    if (hit->face() != nullptr) {
                        hit->face()->dumpTo("./submesh1/v1_" + std::to_string(idx++) + ".off");
                    }
                }
                idx = 0;
                for (Halfedge* hit : v2->halfedges()) {
                    if (hit->face() != nullptr) {
                        hit->face()->dumpTo("./submesh1/v2_" + std::to_string(idx++) + ".off");
                    }
                }
            }
            assert(h->opposite() == NULL);
            h->setOpposite(this);
            this->setOpposite(h);
            break;
        }
    }
}

Halfedge* Halfedge::find_prev() {
    MCGAL::Halfedge* g = this;
    while (g->next() != this) {
        g = g->next();
    }
    return g;
}

Halfedge* Halfedge::next_boundary(int ogroupId) {
    MCGAL::Halfedge* boundary = this;
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

void Halfedge::setVertex(Vertex* v1, Vertex* v2) {
    vertex_ = v1;
    end_vertex_ = v2;
}

float Halfedge::length() {
    return (vertex_->point() - end_vertex_->point()).length();
}

Halfedge::~Halfedge() {}
}  // namespace MCGAL
