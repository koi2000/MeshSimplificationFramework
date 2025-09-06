/*
 * @Author: koi
 * @Date: 2025-09-01 11:13:52
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-08-29 17:49:34
 * @Description:
 */
#ifndef BASIC_SERIALIZEOPERATOR_H
#define BASIC_SERIALIZEOPERATOR_H

#include "SerializeOperator.h"
#include <string>

/**
 * @brief 序列化操作符
 * @details 这里的序列化类型有很多区别
 * 首先header就有区别，然后就是序列化每一轮的symbol
 */
class BasicSerializeOperator : public SerializeOperator {
  public:
    BasicSerializeOperator(std::shared_ptr<SymbolCollectOperator> collector) : SerializeOperator(collector) {}

    void initBuffer(int size);

    void serializeInt(int val);

    void serializeBaseMesh(std::shared_ptr<MCGAL::Mesh> mesh);

    void serializeBaseMeshWithSeed(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed);

    void serializeCharPointer(char* val, int size);

    void serialize(std::string path);
};
#endif