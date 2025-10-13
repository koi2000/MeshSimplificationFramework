// A standalone selection module for choosing the next halfedge to collapse.
// It maintains an internal min-priority queue ordered by user-provided error metrics.
//
// Usage:
//   - Provide either a halfedge error accessor or a vertex error accessor (or both).
//   - When using vertex-based errors, the halfedge error is computed by combining
//     the two endpoint vertex errors via the provided combiner (default: min).
//   - Push or build from a mesh, then call popNext() to get the next best halfedge.

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "../operator/IsRemovableOperator.h"
#include "ErrorSource.h"
#include "Halfedge.h"
#include "Mesh.h"
#include "Vertex.h"

struct HalfedgeErrorEntry {
    MCGAL::Halfedge* edge;
    MCGAL::Point p;
    float error;
    std::uint64_t version;
};

struct VertexErrorEntry {
    MCGAL::Vertex* vertex;
    float error;
    std::uint64_t version;
};

struct MinErrorComparator {
    bool operator()(const HalfedgeErrorEntry& a, const HalfedgeErrorEntry& b) const {
        // priority_queue puts the "largest" first; we invert to get min-heap behavior
        return a.error > b.error;
    }
};

class HalfedgeSelectionQueue {
  public:
    HalfedgeSelectionQueue() = default;
    HalfedgeSelectionQueue(ErrorSource source,
                           std::function<float(MCGAL::Halfedge*, MCGAL::Point& p)> halfedgeErrorAccessor = nullptr,
                           std::function<float(MCGAL::Vertex*)> vertexErrorAccessor = nullptr,
                           std::vector<std::shared_ptr<IsRemovableOperator>> operators = {})
        : source_(source), halfedgeErrorAccessor_(std::move(halfedgeErrorAccessor)), vertexErrorAccessor_(std::move(vertexErrorAccessor)),
          operators_(operators) {
        if (!isValidEdgePredicate_) {
            isValidEdgePredicate_ = [](MCGAL::Halfedge* e) { return e != nullptr && !e->isRemoved(); };
        }
    }

    void clear() {
        currentVersions_.clear();
        while (!heap_.empty())
            heap_.pop();
        globalVersionCounter_ = 0;
    }

    bool empty() const {
        return currentVersions_.empty();
    }

    std::size_t size() const {
        return currentVersions_.size();
    }

    void reserve(std::size_t n) {
        currentVersions_.reserve(n);
    }

    void addIsRemovableOperator(std::shared_ptr<IsRemovableOperator> op) {
        operators_.push_back(op);
    }

    bool isRemovable(MCGAL::Halfedge* e) {
        for (auto& op : operators_) {
            if (!op->isRemovable(e)) {
                return false;
            }
        }
        return true;
    }

    bool isValid(MCGAL::Halfedge* e) {
        return isValidEdgePredicate_(e) && isRemovable(e);
    }

    // Build queue from all halfedges incident to vertices of the mesh.
    // This avoids coupling with any specific mesh traversal elsewhere in the project.
    void buildFromMesh(std::shared_ptr<MCGAL::Mesh>& mesh) {
        clear();
        if (source_ == ErrorSource::Halfedge) {
            auto& vertices = mesh->vertices();
            for (MCGAL::Vertex* v : vertices) {
                if (v == nullptr)
                    continue;
                auto& halfedges = v->halfedges();
                for (MCGAL::Halfedge* h : halfedges) {
                    if (h == nullptr || h->isRemoved())
                        continue;
                    // Only index each undirected edge once (by convention: original flag or lower poolId)
                    // if (!isValid(h))
                    //     continue;
                    // if (h->opposite() && h->poolId() > h->opposite()->poolId())
                    //     continue;
                    pushOrUpdate(h);
                }
            }
        }
        int i = heap_.size();
        int j = 0;
    }

    // Insert or update the error for a given halfedge.
    void pushOrUpdate(MCGAL::Halfedge* edge) {
        if (edge == nullptr)
            return;
        // if (!isValid(edge))
        //     return;
        MCGAL::Point p;
        float err = computeError(edge, p);
        std::uint64_t version = ++globalVersionCounter_;
        currentVersions_[edge] = version;
        heap_.push(HalfedgeErrorEntry{edge, p, err, version});
    }

    // Remove an edge from consideration (lazy remove).
    void remove(MCGAL::Halfedge* edge) {
        if (edge == nullptr)
            return;
        currentVersions_.erase(edge);
    }

    // Pop the current best edge according to the error ordering.
    // Skips any stale entries lazily.
    std::pair<MCGAL::Halfedge*, MCGAL::Point> popNext() {
        while (!heap_.empty()) {
            const HalfedgeErrorEntry top = heap_.top();
            heap_.pop();
            auto it = currentVersions_.find(top.edge);
            if (it == currentVersions_.end()) {
                continue;  // removed or superseded
            }
            if (it->second != top.version) {
                continue;  // superseded by a newer update
            }
            if (!isValid(top.edge)) {
                currentVersions_.erase(it);
                continue;
            }
            return {top.edge, top.p};
        }
        return {nullptr, MCGAL::Point()};
    }

    // Recompute error for a specific edge and update its priority.
    void recompute(MCGAL::Halfedge* edge) {
        pushOrUpdate(edge);
    }

    float peekMinError() const {
        // Not strictly exact due to lazy removal; used for diagnostics.
        if (heap_.empty())
            return std::numeric_limits<float>::infinity();
        return heap_.top().error;
    }

  private:
    float computeError(MCGAL::Halfedge* edge, MCGAL::Point& p) const {
        if (source_ == ErrorSource::Halfedge) {
            if (halfedgeErrorAccessor_)
                return halfedgeErrorAccessor_(edge, p);
        } else {
            if (vertexErrorAccessor_) {
                if (edge->end_vertex())
                    return vertexErrorAccessor_(edge->end_vertex());
            }
        }
        // 如果不传入，就默认不使用error计算 返回0
        return 0;
    }

  private:
    ErrorSource source_;
    std::function<float(MCGAL::Halfedge*, MCGAL::Point&)> halfedgeErrorAccessor_;
    std::function<float(MCGAL::Vertex*)> vertexErrorAccessor_;
    std::vector<std::shared_ptr<IsRemovableOperator>> operators_;
    std::function<bool(MCGAL::Halfedge*)> isValidEdgePredicate_;

    std::priority_queue<HalfedgeErrorEntry, std::vector<HalfedgeErrorEntry>, MinErrorComparator> heap_;
    std::unordered_map<MCGAL::Halfedge*, std::uint64_t> currentVersions_;
    std::uint64_t globalVersionCounter_ = 0;
};
