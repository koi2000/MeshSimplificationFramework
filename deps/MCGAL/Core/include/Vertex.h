#ifndef VERTEX_H
#define VERTEX_H
#include "Point.h"
#include "VertexSplitNode.h"
#include <assert.h>
#include <list>
#include <stdexcept>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
namespace MCGAL {

class Halfedge;
class Facet;
class Vertex;

#define VERTEX_BUCKET_SIZE 3

class Vertex : public Point {
    enum RemovedFlag { NotRemoved, Removed };
    enum ProcessedFlag { NotProcessed, Processed };
    enum CollapsedFlag { NotCollapsed, Collapsed };

  public:
    Vertex() : Point() {}
    Vertex(const Point& p) : Point(p) {}
    Vertex(float v1, float v2, float v3) : Point(v1, v2, v3) {}

    int poolId() {
        return poolId_;
    }

    int groupId() {
        return groupId_;
    }

    void setPoolId(int poolId) {
        this->poolId_ = poolId;
    }

    void setMeshId(int meshId) {
        this->meshId_ = meshId;
    }

    int vertex_degree() {
        return halfedges_.size();
    }

    void print() {
        printf("%f %f %f\n", v[0], v[1], v[2]);
    }

    void setVid(int id) {
        this->vid_ = id;
    }

    void setGroupId(int id) {
        this->groupId_ = id;
    }

    int vid() {
        return vid_;
    }

    float x() const {
        return v[0];
    }

    float y() const {
        return v[1];
    }

    float z() const {
        return v[2];
    }

    int vid() const {
        return vid_;
    }

    Point point() {
        return Point(this->v[0], this->v[1], this->v[2]);
    }

    void setPoint(const Point& p) {
        this->v[0] = p.x();
        this->v[1] = p.y();
        this->v[2] = p.z();
    }

    void setPoint(float x, float y, float z) {
        this->v[0] = x;
        this->v[1] = y;
        this->v[2] = z;
    }

    void setPoint(float x, float y, float z, int id) {
        this->v[0] = x;
        this->v[1] = y;
        this->v[2] = z;
    }

    virtual void resetState() {
        this->processedFlag_ = NotProcessed;
    }

    inline void setRemoved() {
        removedFlag_ = Removed;
    }

    inline bool isRemoved() {
        return removedFlag_ == Removed;
    }

    inline void setCollapsed() {
        collapsedFlag_ = Collapsed;
    }

    inline bool isCollapsed() {
        return collapsedFlag_ == Collapsed;
    }

    inline void setProcessed() {
        processedFlag_ = Processed;
    }

    inline bool isProcessed() {
        return processedFlag_ == Processed;
    }

    std::vector<Halfedge*>& halfedges() {
        return halfedges_;
    };

    void eraseHalfedgeByPointer(Halfedge* halfedge) {
        for (auto it = halfedges_.begin(); it != halfedges_.end();) {
            if ((*it) == halfedge) {
                halfedges_.erase(it);
                break;
            } else {
                it++;
            }
        }
    }

    void setVertexSplitNode(VertexSplitNode* node) {
        this->vsplitNode_ = node;
    }

    VertexSplitNode* vsplitNode() {
        return this->vsplitNode_;
    }

  private:
    int vid_ = 0;
    int poolId_ = -1;
    int meshId_ = -1;
    int groupId_ = -1;
    std::vector<Halfedge*> halfedges_;
    RemovedFlag removedFlag_ = NotRemoved;
    ProcessedFlag processedFlag_ = NotProcessed;
    CollapsedFlag collapsedFlag_ = NotCollapsed;

    VertexSplitNode* vsplitNode_;
};
}  // namespace MCGAL

#endif