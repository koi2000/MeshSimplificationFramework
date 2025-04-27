#ifndef HALFEDGE_H
#define HALFEDGE_H
#include "Point.h"
#include <assert.h>
#include <atomic>
namespace MCGAL {
class Point;
class Vertex;
class Facet;
class Halfedge {
  public:
    Halfedge() {};
    Halfedge(Vertex* v1, Vertex* v2);
    ~Halfedge();

    void setVertex(Vertex* v1, Vertex* v2);

    void reset(Vertex* v1, Vertex* v2);

    void reset_without_init(Vertex* v1, Vertex* v2);

    void reset_without_pushback(Vertex* v1, Vertex* v2);

    inline void resetBfsFlag() {
        processedFlag_ = NotProcessed;
    }

    inline void resetState() {
        processedFlag_ = NotProcessed;
    }

    Point getRemovedVertexPos() const;

    /* Processed flag */
    inline void resetProcessedFlag() {
        processedFlag_ = NotProcessed;
    }

    inline void setProcessed() {
        processedFlag_ = Processed;
    }

    inline void setUnProcessed() {
        processedFlag_ = NotProcessed;
    }

    inline bool isBoundary() const {
        return boundaryFlag_ == IsBoundary;
    }

    inline void setBoundary() {
        boundaryFlag_ = IsBoundary;
    }

    inline void setNotBoundary() {
        boundaryFlag_ = NotBoundary;
    }

    inline bool isProcessed() const {
        return processedFlag_ == Processed;
    }

    inline void setRemoved() {
        removedFlag_ = Removed;
    }

    inline bool isRemoved() {
        return removedFlag_ == Removed;
    }

    // 自定义的比较函数，只比较名字
    bool operator==(const MCGAL::Halfedge& other) const {
        return poolId_ == other.poolId_;
    }

    inline Vertex* vertex() const {
        return vertex_;
    }

    inline void setVertex(Vertex* v) {
        this->vertex_ = v;
    }

    inline Vertex* end_vertex() const {
        return end_vertex_;
    }

    inline void setEndVertex(Vertex* v) {
        this->end_vertex_ = v;
    }

    inline Facet* face() const {
        return face_;
    }

    inline void setFace(Facet* f) {
        face_ = f;
    }

    inline Halfedge* next() const {
        return next_;
    }

    inline void setNext(Halfedge* h) {
        next_ = h;
    }

    inline Halfedge* opposite() const {
        return opposite_;
    }

    inline void setOpposite(Halfedge* h) {
        opposite_ = h;
    }

    inline int poolId() const {
        return poolId_;
    }

    inline void setPoolId(int id) {
        poolId_ = id;
    }

    inline int meshId() const {
        return meshId_;
    }

    inline void setMeshId(int id) {
        meshId_ = id;
    }

    inline int groupId() const {
        return groupId_;
    }

    inline void setGroupId(int id) {
        groupId_ = id;
    }

    float length();

  private:
    enum ProcessedFlag { NotProcessed, Processed };
    enum RemovedFlag { NotRemoved, Removed };
    enum BoundaryFlag { NotBoundary, IsBoundary };

    ProcessedFlag processedFlag_ = NotProcessed;
    RemovedFlag removedFlag_ = NotRemoved;
    BoundaryFlag boundaryFlag_ = NotBoundary;

    Vertex* vertex_ = nullptr;
    Vertex* end_vertex_ = nullptr;
    Facet* face_ = nullptr;
    Halfedge* next_ = nullptr;
    Halfedge* opposite_ = nullptr;

    int poolId_ = -1;
    int meshId_ = -1;
    int groupId_ = -1;
};
}  // namespace MCGAL

#endif