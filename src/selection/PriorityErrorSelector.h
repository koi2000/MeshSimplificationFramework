/*
 * @Author: koi
 * @Date: 2025-08-27 11:35:42
 * @Description: 根据error来进行选择的selector
 */
#ifndef PRIORITY_ERROR_SELECTOR_H
#define PRIORITY_ERROR_SELECTOR_H

#include "Mesh.h"
#include "SelectOperator.h"
#include "selection/HalfedgeSelectionQueue.h"
#include <memory>

class PriorityErrorSelector : public SelectOperator {
  public:
    PriorityErrorSelector(SelectOptions options);
    void init(std::shared_ptr<MCGAL::Mesh> mesh) override;
    PriorityErrorSelector();
    ~PriorityErrorSelector();
    bool select(MCGAL::Halfedge*& halfedge) override;
    void reset() override;

  private:
    HalfedgeSelectionQueue queue;
};

#endif