/*
 * @Author: koi
 * @Date: 2025-09-13 22:25:20
 * @Description: 
 */
/*
 * @Author: koi
 * @Date: 2025-08-26 22:22:37
 * @Description: 
 */
#ifndef SEGMENT_OPERATOR_H
#define SEGMENT_OPERATOR_H

#include "core.h"
#include <memory>

/**
 * 传入一个网格，然后可以自行操作如何进行segment，可以使用很多现有的分区方法
 * 返回临界图
*/
class SegmentOperator {
public:
    SegmentOperator();
    virtual ~SegmentOperator() = default;

    virtual void segment(std::shared_ptr<MCGAL::Mesh> mesh) = 0;
};

#endif