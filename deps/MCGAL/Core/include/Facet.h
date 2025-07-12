#ifndef FACET_H
#define FACET_H
#include "Halfedge.h"
#include "Point.h"
#include <assert.h>
#include <set>
#include <vector>
namespace MCGAL {
class Point;
class Vertex;
class Halfedge;
class Facet {
  private:
    enum ProcessedFlag { NotProcessed, Processed };
    enum RemovedFlag { NotRemoved, Removed };
    enum SplittableFlag { Unknown = 0, Splittable = 1, Unsplittable = 2 };

  public:
    class halfedge_iterator {
      private:
        Halfedge* current_;
        Halfedge* start_;
        bool is_first_iteration_;

      public:
        // Constructor
        halfedge_iterator(Halfedge* start = nullptr) : current_(start), start_(start), is_first_iteration_(true) {}

        // Dereference operator
        Halfedge* operator*() const {
            return current_;
        }

        // Pre-increment operator
        halfedge_iterator& operator++() {
            if (current_ != nullptr) {
                if (is_first_iteration_) {
                    is_first_iteration_ = false;
                } else {
                    // Stop if we've completed the loop and returned to start
                    if (current_->next() == start_) {
                        current_ = nullptr;
                        return *this;
                    }
                }
                current_ = current_->next();
            }
            return *this;
        }

        // Post-increment operator
        halfedge_iterator operator++(int) {
            halfedge_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // Equality comparison
        bool operator==(const halfedge_iterator& other) const {
            return current_ == other.current_;
        }

        // Inequality comparison
        bool operator!=(const halfedge_iterator& other) const {
            return !(*this == other);
        }
    };

    // Iterator begin method - returns iterator pointing to the first halfedge
    halfedge_iterator halfedges_begin() const {
        return halfedge_iterator(proxyHalfedge_);
    }

    // Iterator end method - returns iterator representing past-the-end
    halfedge_iterator halfedges_end() const {
        return halfedge_iterator(nullptr);
    }

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
        splittableFlag_ = Unknown;
    }

    inline void resetProcessedFlag() {
        processedFlag_ = NotProcessed;
    }

    inline void setProcessedFlag(ProcessedFlag flag) {
        processedFlag_ = flag;
    }

    inline ProcessedFlag processedFlag() {
        return processedFlag_;
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

    inline bool isConquered() const {
        return (splittableFlag_ == Splittable || splittableFlag_ == Unsplittable || removedFlag_ == Removed);
    }

    inline bool isSplittable() const {
        return (splittableFlag_ == Splittable);
    }

    inline bool isUnsplittable() const {
        return (splittableFlag_ == Unsplittable);
    }

    inline void setSplittable() {
        assert(splittableFlag_ == Unknown);
        splittableFlag_ = Splittable;
    }

    inline void setUnsplittable() {
        assert(splittableFlag_ == Unknown || splittableFlag_ == Unsplittable);
        splittableFlag_ = Unsplittable;
    }

    Point getRemovedVertexPos() const;

    void setRemovedVertexPos(Point p);

    int facet_degree();

    inline bool isVisited(int current_version) const {
        return visited_version == current_version;
    }

    inline void setVisited(int current_version) {
        visited_version = current_version;
    }

  private:
    SplittableFlag splittableFlag_ = Unknown;
    ProcessedFlag processedFlag_ = NotProcessed;
    RemovedFlag removedFlag_ = NotRemoved;

    int poolId_ = -1;
    int groupId_ = -1;
    int meshId_ = -1;
    MCGAL::Halfedge* proxyHalfedge_ = nullptr;
    MCGAL::Point removedVertexPos;
    std::vector<Vertex*> vertices_;
    std::vector<Halfedge*> halfedges_;
    int facet_degree_ = 0;
    int visited_version = 0;
};

}  // namespace MCGAL
#endif