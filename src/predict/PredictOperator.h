/*
 * @Author: koi
 * @Date: 2025-09-09 22:30:26
 * @Description: 
 */
#ifndef PREDICT_OPERATOR_H
#define PREDICT_OPERATOR_H
#include "Halfedge.h"

class PredictOperator {
public:
    PredictOperator() = default;
    ~PredictOperator() = default;
    
    void predict(MCGAL::Halfedge* edge);
};

#endif