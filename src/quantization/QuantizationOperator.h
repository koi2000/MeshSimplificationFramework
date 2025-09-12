/*
 * @Author: koi
 * @Date: 2025-09-09 22:31:33
 * @Description:
 */
#ifndef QUANTIZATION_OPERATOR_H
#define QUANTIZATION_OPERATOR_H

#include "Point.h"

/**
  * 只有在真正serialize的时候需要进行量化，这个很好做，然后就是predict，其实也就是把存的点变了一下  
  * 
*/
class QuantizationOperator {
  public:
    QuantizationOperator() = default;
    ~QuantizationOperator() = default;

    void quant(MCGAL::Point& pt);
};

#endif