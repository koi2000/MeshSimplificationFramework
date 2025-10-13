/*
 * @Author: koi
 * @Date: 2025-08-25 17:49:55
 * @Description:
 */
#ifndef PMSF_H
#define PMSF_H

#include "Options.h"
#include "core.h"
#include "handler/ProcessorRegistry.h"
#include "options/CompressOptions.h"
#include <cstddef>

class PMSF {
  public:
    PMSF() = default;
    ~PMSF() = default;

    void compress(CompressOptions& options);

    void segmentCompress(CompressOptions& options);

    void segmentDecompress(DecompressOptions& options);

    void decompress(DecompressOptions& options);

    void RegisterProperties();

    // template <class T> void registerPropertity(OpenMesh::VPropHandleT<T> v);
    // template <class T> void registerPropertity(OpenMesh::HPropHandleT<T> h);
    // template <class T> void registerPropertity(OpenMesh::FPropHandleT<T> f);

  private:
    MCGAL::Mesh mesh_;
    char* buffer_ = nullptr;
};

#endif