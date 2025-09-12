/*
 * @Author: koi
 * @Date: 2025-08-29 17:49:34
 * @Description:
 */
#ifndef SerializeOperator_H
#define SerializeOperator_H

#include "Halfedge.h"
#include "Mesh.h"
#include "compress/SymbolCollectOperator.h"
#include <memory>
#include <string>

/**
 * @brief 序列化操作符
 * @details 这里的序列化类型有很多区别
 * 首先header就有区别，然后就是序列化每一轮的symbol
 */
class SerializeOperator {
  public:
    SerializeOperator(std::shared_ptr<SymbolCollectOperator> collector, bool enablePrediction = false, bool enableQuantization = false, bool enableCompress = false)
        : collector_(collector), enablePrediction_(enablePrediction), enableQuantization_(enableQuantization), enableCompress_(enableCompress) {}

    virtual void initBuffer(int size) = 0;

    virtual void serialize(std::string path) = 0;

    virtual void serializeInt(int val) = 0;

    virtual void serializeBaseMesh(std::shared_ptr<MCGAL::Mesh> mesh) = 0;

    virtual void serializeBaseMeshWithSeed(std::shared_ptr<MCGAL::Mesh> mesh, MCGAL::Halfedge* seed) = 0;

    // virtual void serializeCharPointer(char* val, int size) = 0;

    bool enablePrediction_;
    bool enableCompress_;
    bool enableQuantization_;
    std::shared_ptr<SymbolCollectOperator> collector_;
    char* buffer_;
    int dataOffset_ = 0;
};
#endif