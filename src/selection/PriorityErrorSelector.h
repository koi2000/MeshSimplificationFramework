/*
 * @Author: koi
 * @Date: 2025-08-27 11:35:42
 * @Description: 根据error来进行选择的selector
 */
#ifndef PRIORITY_ERROR_SELECTOR_H
#define PRIORITY_ERROR_SELECTOR_H

#include "SelectOperator.h"
#include "selection/HalfedgeSelectionQueue.h"

class PriorityErrorSelector : public SelectOperator {
  public:
    PriorityErrorSelector(std::shared_ptr<MCGAL::Mesh> mesh, SelectOptions options);
    PriorityErrorSelector();
    ~PriorityErrorSelector();
    bool select(MCGAL::Halfedge* halfedge) override;

  private:
    HalfedgeSelectionQueue queue;
};

#endif