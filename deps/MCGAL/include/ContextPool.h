#ifndef CONTEXTPOOL_H
#define CONTEXTPOOL_H
#include "Configuration.h"
#include "Facet.h"
#include "Halfedge.h"
#include "Vertex.h"
#include "openmesh/Property.hh"
#include "openmesh/PropertyContainer.hh"
#include <deque>
#include <mutex>
#include <vector>

namespace MCGAL {

class ContextPool {
  private:
    int poolIndex = 0;

  public:
    std::vector<int> vindexs;
    std::vector<int> hindexs;
    std::vector<int> findexs;

    std::vector<int> vpoolSizes;
    std::vector<int> hpoolSizes;
    std::vector<int> fpoolSizes;

    std::vector<MCGAL::Vertex*> vpools;
    std::vector<MCGAL::Halfedge*> hpools;
    std::vector<MCGAL::Facet*> fpools;

    std::vector<OpenMesh::PropertyContainer> vprops_;
    std::vector<OpenMesh::PropertyContainer> hprops_;
    std::vector<OpenMesh::PropertyContainer> eprops_;
    std::vector<OpenMesh::PropertyContainer> fprops_;
    std::vector<OpenMesh::PropertyContainer> mprops_;

    ContextPool();

  public:
    ~ContextPool();

    static ContextPool& getInstance() {
        static ContextPool contextPool;
        return contextPool;
    }

    void initPoolSize(int size);

    int registerPool(int estimateVSize, int estimateHSize, int estimateFSize);
    void releasePool(int index);

    ContextPool(const ContextPool&) = delete;
    ContextPool& operator=(const ContextPool&) = delete;

    void reset() {}

    template <class T> void add_property(int index, OpenMesh::VPropHandleT<T>& _ph, const std::string& _name = "<vprop>") {
        _ph = OpenMesh::VPropHandleT<T>(vprops_[index].add(T(), _name));
        vprops_[index].resize(vpoolSizes[index]);
    }

    template <class T> void add_property(int index, OpenMesh::HPropHandleT<T>& _ph, const std::string& _name = "<eprop>") {
        _ph = OpenMesh::HPropHandleT<T>(hprops_[index].add(T(), _name));
        hprops_[index].resize(hpoolSizes[index]);
    }

    template <class T> void add_property(int index, OpenMesh::FPropHandleT<T>& _ph, const std::string& _name = "<fprop>") {
        _ph = OpenMesh::FPropHandleT<T>(fprops_[index].add(T(), _name));
        fprops_[index].resize(fpoolSizes[index]);
    }

    template <class T> typename OpenMesh::VPropHandleT<T>::reference property(int index, OpenMesh::VPropHandleT<T> _ph, MCGAL::Vertex* _vh) {
        return vprops_[index].property(_ph)[_vh->poolId()];
    }

    template <class T> typename OpenMesh::VPropHandleT<T>::const_reference property(int index, OpenMesh::VPropHandleT<T> _ph, MCGAL::Vertex* _vh) const {
        return vprops_[index].property(_ph)[_vh->poolId()];
    }

    template <class T> typename OpenMesh::HPropHandleT<T>::reference property(int index, OpenMesh::HPropHandleT<T> _ph, MCGAL::Halfedge* _hh) {
        return hprops_[index].property(_ph)[_hh->poolId()];
    }

    template <class T> typename OpenMesh::HPropHandleT<T>::const_reference property(int index, OpenMesh::HPropHandleT<T> _ph, MCGAL::Halfedge* _hh) const {
        return hprops_[index].property(_ph)[_hh->poolId()];
    }

    template <class T> typename OpenMesh::FPropHandleT<T>::reference property(int index, OpenMesh::FPropHandleT<T> _ph, MCGAL::Facet* _fh) {
        return fprops_[index].property(_ph)[_fh->poolId()];
    }

    template <class T> typename OpenMesh::FPropHandleT<T>::const_reference property(int index, OpenMesh::FPropHandleT<T> _ph, MCGAL::Facet* _fh) const {
        return fprops_[index].property(_ph)[_fh->poolId()];
    }

    template <class T> typename OpenMesh::VPropHandleT<T>::reference property(OpenMesh::VPropHandleT<T> _ph, MCGAL::Vertex* _vh) {
        return vprops_[DEFAULT_MESH_ID].property(_ph)[_vh->poolId()];
    }

    template <class T> typename OpenMesh::VPropHandleT<T>::const_reference property(OpenMesh::VPropHandleT<T> _ph, MCGAL::Vertex* _vh) const {
        return vprops_[DEFAULT_MESH_ID].property(_ph)[_vh->poolId()];
    }

    template <class T> typename OpenMesh::HPropHandleT<T>::reference property(OpenMesh::HPropHandleT<T> _ph, MCGAL::Halfedge* _hh) {
        return hprops_[DEFAULT_MESH_ID].property(_ph)[_hh->poolId()];
    }

    template <class T> typename OpenMesh::HPropHandleT<T>::const_reference property(OpenMesh::HPropHandleT<T> _ph, MCGAL::Halfedge* _hh) const {
        return hprops_[DEFAULT_MESH_ID].property(_ph)[_hh->poolId()];
    }

    template <class T> typename OpenMesh::FPropHandleT<T>::reference property(OpenMesh::FPropHandleT<T> _ph, MCGAL::Facet* _fh) {
        return fprops_[DEFAULT_MESH_ID].property(_ph)[_fh->poolId()];
    }

    template <class T> typename OpenMesh::FPropHandleT<T>::const_reference property(OpenMesh::FPropHandleT<T> _ph, MCGAL::Facet* _fh) const {
        return fprops_[DEFAULT_MESH_ID].property(_ph)[_fh->poolId()];
    }

    inline int getFindex(int index) {
        return findexs[index];
    }

    inline int getHindex(int index) {
        return hindexs[index];
    }

    inline int getVindex(int index) {
        return vindexs[index];
    }

    inline MCGAL::Vertex* getVertexPoolByIndex(int groupId) {
        return vpools[groupId];
    }

    inline MCGAL::Halfedge* getHalfedgePoolByIndex(int groupId) {
        return hpools[groupId];
    }

    inline MCGAL::Facet* getFacetPoolByIndex(int groupId) {
        return fpools[groupId];
    }

    inline MCGAL::Vertex* getVertexByIndexInSubPool(int groupId, int offset) {
        return &vpools[groupId][offset];
    }

    inline MCGAL::Halfedge* getHalfedgeByIndexInSubPool(int groupId, int offset) {
        return &hpools[groupId][offset];
    }

    inline MCGAL::Facet* getFacetByIndexInSubPool(int groupId, int offset) {
        return &fpools[groupId][offset];
    }

    inline MCGAL::Vertex* getVertexByIndex(int index, int offset) {
        return &vpools[index][offset];
    }

    inline MCGAL::Halfedge* getHalfedgeByIndex(int index, int offset) {
        return &hpools[index][offset];
    }

    inline MCGAL::Facet* getFacetByIndex(int index, int offset) {
        return &fpools[index][offset];
    }

    inline MCGAL::Vertex* allocateVertexFromPool(int index) {
        MCGAL::Vertex* v = &vpools[index][vindexs[index]++];
        return v;
    }

    inline MCGAL::Vertex* allocateVertexFromPool(int index, float x, float y, float z) {
        MCGAL::Vertex* v = &vpools[index][vindexs[index]++];
        v->setPoint(x, y, z);
        return v;
    }

    inline MCGAL::Vertex* allocateVertexFromPool(int index, MCGAL::Point p) {
        MCGAL::Vertex* v = &vpools[index][vindexs[index]++];
        v->setPoint(p);
        return v;
    }

    inline MCGAL::Halfedge* allocateHalfedgeFromPool(int index) {
        return &hpools[index][hindexs[index]++];
    }

    inline MCGAL::Halfedge* allocateHalfedgeFromPoolWithOutInit(int index, MCGAL::Vertex* v1, MCGAL::Vertex* v2) {
        MCGAL::Halfedge* h = &hpools[index][hindexs[index]++];
        h->reset_without_init(v1, v2);
        h->setMeshId(index);
        return h;
    }

    inline MCGAL::Halfedge* allocateHalfedgeFromPool(int index, MCGAL::Vertex* v1, MCGAL::Vertex* v2) {
        MCGAL::Halfedge* h = &hpools[index][hindexs[index]++];
        h->reset(v1, v2);
        h->setMeshId(index);
        return h;
    }

    inline MCGAL::Facet* allocateFaceFromPool(int index) {
        return &fpools[index][findexs[index]++];
    }

    inline MCGAL::Facet* allocateFaceFromPool(int index, std::vector<MCGAL::Vertex*> vts) {
        MCGAL::Facet* f = &fpools[index][findexs[index]++];
        f->setMeshId(index);
        f->reset(vts);
        return f;
    }

    inline MCGAL::Facet* allocateFaceFromPool(int index, MCGAL::Halfedge* halfedge) {
        MCGAL::Facet* f = &fpools[index][findexs[index]++];
        if(findexs[index] == 90551) {
            int i = 0;
        }
        f->setMeshId(index);
        f->reset_without_init(halfedge);
        return f;
    }

    inline int preAllocVertex(int index, int size) {
        int ret = vindexs[index];
        vindexs[index] += size;
        return ret;
    }

    inline int preAllocHalfedge(int index, int size) {
        int ret = vindexs[index];
        vindexs[index] += size;
        return ret;
    }

    inline int preAllocFace(int index, int size) {
        int ret = vindexs[index];
        vindexs[index] += size;
        return ret;
    }
};

}  // namespace MCGAL
#endif