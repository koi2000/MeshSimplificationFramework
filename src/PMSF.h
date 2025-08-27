/*
 * @Author: koi
 * @Date: 2025-08-25 17:49:55
 * @Description:
 */
#ifndef PMSF_H
#define PMSF_H

#include "core.h"
#include "handler/ProcessorRegistry.h"
#include <cstddef>

class PMSF {
  public:
    PMSF() = default;
    ~PMSF() = default;

    void compress();

    void decompress();

    void RegisterProperties();

  private:
    MCGAL::Mesh mesh;
    char* buffer = nullptr;
};

#endif