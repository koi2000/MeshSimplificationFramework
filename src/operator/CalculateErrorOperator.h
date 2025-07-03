#ifndef CALCULATE_ERROR_H
#define CALCULATE_ERROR_H

#include "core.h"

class CalculateErrorOperator {
public:
    CalculateErrorOperator();
    void calculateError(MCGAL::Halfedge* edge, MCGAL::Point& ptResult);
};  

#endif