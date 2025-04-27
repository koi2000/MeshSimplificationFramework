#ifndef FACET_H
#define FACET_H
#include "Point.h"
#include <assert.h>
#include <set>
#include <vector>
namespace MCGAL {
class Point;
class Vertex;
class Halfedge;
class Facet {
  public:
    ~Facet();

    Facet() {};
    Facet(const Facet& face);
    Facet(Halfedge* hit);
    Facet(std::vector<Vertex*>& vs);

    void setMeshId(int meshId) {
        this->meshId_ = meshId;
    }

    void setGroupId(int gid) {
        this->groupId_ = gid;
    }
    bool isDegenerate();
    Facet* clone();
    void reset(Halfedge* h);
    void reset(std::vector<Halfedge*>& hs);
    void reset(std::vector<Vertex*>& vs);
    void reset(std::vector<Vertex*>& vs, int meshId);
    void reset_without_init(Halfedge* h);

    void remove(Halfedge* h);
    MCGAL::Point computeNormal() const;

    // override
    bool equal(const Facet& rhs) const;
    bool operator==(const Facet& rhs) const;

    // to_string method
    void print();
    void print_off();

    void dumpTo(std::string path);

    inline void resetBfsFlag() {
        processedFlag_ = NotProcessed;
    }

    inline void resetState() {
        processedFlag_ = NotProcessed;
    }

    inline void resetProcessedFlag() {
        processedFlag_ = NotProcessed;
    }

    inline void setProcessedFlag() {
        processedFlag_ = Processed;
    }

    inline void setUnProcessed() {
        processedFlag_ = NotProcessed;
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

    inline int poolId() const {
        return poolId_;
    }

    inline void setPoolId(int id) {
        poolId_ = id;
    }
    inline int groupId() const {
        return groupId_;
    }

    inline void groupId(int id) {
        groupId_ = id;
    }

    inline int meshId() const {
        return meshId_;
    }

    inline void meshId(int id) {
        meshId_ = id;
    }

    inline Halfedge* proxyHalfedge() const {
        return proxyHalfedge_;
    }

    inline void proxyHalfedge(Halfedge* h) {
        proxyHalfedge_ = h;
    }

    std::vector<Vertex*> getVertices();

    int facet_degree();
    
  private:
    enum ProcessedFlag { NotProcessed, Processed };
    enum RemovedFlag { NotRemoved, Removed };

    ProcessedFlag processedFlag_ = NotProcessed;
    RemovedFlag removedFlag_ = NotRemoved;

    int poolId_ = -1;
    int groupId_ = -1;
    int meshId_ = -1;
    MCGAL::Halfedge* proxyHalfedge_ = nullptr;
    std::vector<Vertex*> vertices_;
    std::vector<Halfedge*> halfedges_;
    int facet_degree_ = 0;
};

}  // namespace MCGAL
#endif