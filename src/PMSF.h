#ifndef PMSF_H
#define PMSF_H

#include "core.h"
#include "handler/ProcessorRegistry.h"

class PMSF {
  public:
    PMSF() = default;
    ~PMSF() = default;

    void RegisterProperties();

  private:
    MCGAL::Mesh mesh;
};

#endif