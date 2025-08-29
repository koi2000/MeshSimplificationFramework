#include "VertexRemovalEliminateOperator.h"
#include "Facet.h"
#include "Global.h"
#include "Mesh.h"
#include "Vertex.h"
#include <vector>

void VertexRemovalEliminateOperator::init(std::shared_ptr<MCGAL::Mesh> mesh) {
    mesh_ = mesh;
}

void VertexRemovalEliminateOperator::eliminate(MCGAL::Halfedge* h) {
    if (h == nullptr)
        return;
    remove_point(h);
    triangulate(h);
    postprocess(h);
    encode_boundary(h);
}

void VertexRemovalEliminateOperator::remove_point(MCGAL::Halfedge* h) {
    if (!h)
        return;
    h->vertex()->setCollapsed();
    MCGAL::Vertex* removedVertex = h->end_vertex();
    if (!removedVertex || removedVertex->isRemoved())
        return;

    mesh_->erase_center_vertex(h);
}

void VertexRemovalEliminateOperator::triangulate(MCGAL::Halfedge* h) {
    if (!h)
        return;
    MCGAL::Facet* polyFace = h->face();
    if (!polyFace || polyFace->isRemoved())
        return;
    h = polyFace->proxyHalfedge();
    std::vector<MCGAL::Vertex*> polyVerts = polyFace->getVertices();
    std::vector<MCGAL::Halfedge*> polyHalfedges = polyFace->getHalfedges();
    int n = static_cast<int>(polyVerts.size());
    if (n < 4)
        return;
    for (int i = 0; i < n - 2; ++i) {
        if (i == 0) {
            MCGAL::Halfedge* h0 = polyHalfedges[0];
            MCGAL::Halfedge* h1 = polyHalfedges[1];
            MCGAL::Vertex* v1 = polyVerts[2];
            MCGAL::Vertex* v2 = polyVerts[0];
            MCGAL::Halfedge* hnew = MCGAL::contextPool.allocateHalfedgeFromPool(mesh_->meshId(), v1, v2);
            hnew->setAdded();
            h0->setNext(h1);
            h1->setNext(hnew);
            hnew->setNext(h0);
            polyFace->reset(h0);
        } else if (i == n - 3) {
            MCGAL::Halfedge* h0 = polyHalfedges[n - 2];
            MCGAL::Halfedge* h1 = polyHalfedges[n - 1];
            MCGAL::Vertex* v1 = polyVerts[0];
            MCGAL::Vertex* v2 = polyVerts[n - 2];
            MCGAL::Halfedge* hnew = MCGAL::contextPool.allocateHalfedgeFromPool(mesh_->meshId(), v1, v2);
            hnew->setAdded();
            h0->setNext(h1);
            h1->setNext(hnew);
            hnew->setNext(h0);
            MCGAL::Facet* newFace = MCGAL::contextPool.allocateFaceFromPool(mesh_->meshId(), h0);
            mesh_->add_face(newFace);
        } else {
            MCGAL::Vertex* v0 = polyVerts[0];
            MCGAL::Vertex* v1 = polyVerts[i + 1];
            MCGAL::Vertex* v2 = polyVerts[i + 2];
            MCGAL::Halfedge* h0 = polyHalfedges[i + 1];
            MCGAL::Halfedge* h1 = MCGAL::contextPool.allocateHalfedgeFromPool(mesh_->meshId(), v2, v0);
            MCGAL::Halfedge* h2 = MCGAL::contextPool.allocateHalfedgeFromPool(mesh_->meshId(), v0, v1);
            h1->setAdded();
            h2->setAdded();
            h0->setNext(h1);
            h1->setNext(h2);
            h2->setNext(h0);
            MCGAL::Facet* newFace = MCGAL::contextPool.allocateFaceFromPool(mesh_->meshId(), h0);
            mesh_->add_face(newFace);
        }
    }
    h->face()->setSplittable();
    h->face()->setRemovedVertexPos(h->end_vertex());
}

void VertexRemovalEliminateOperator::postprocess(MCGAL::Halfedge* h) {
    (void)h;
}

void VertexRemovalEliminateOperator::encode_boundary(MCGAL::Halfedge* h) {
    (void)h;
}
