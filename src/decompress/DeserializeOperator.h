/*
 * @Author: koi
 * @Date: 2025-08-29 22:44:22
 * @Description:
 */
#ifndef DeserializeOperator_H
#define DeserializeOperator_H

#include "Mesh.h"
#include "../header/PMSFHeader.h"
#include "decompress/SymbolReadOperator.h"
#include "reconstruct/ReconstructOpertator.h"
#include <memory>

/**
 * @brief 压缩操作符
 * @details 压缩操作符是用于压缩网格的抽象类，有一个困难点在于很多东西不知道怎么去存，不知道哪些要存，不知道怎么去存
 * 比如在现有的统一化设计方案里，需要先去三角化然后插入中心点
 * 是否可以尝试将信息全部存储在halfedge上，第一个是isAdded，来标识是否为新增，第二个为isSplittable，同时记录点的位置信息，但这样存在一个缺点，
 * 半边的数量是远大于面片的数量的，这样会导致文件大小增加，最好的方式是：
 * 将isAdded信息attach到边而不是半边上，这样会让bit数量减少一半
 */
class DeserializeOperator {
  public:
    DeserializeOperator(std::shared_ptr<SymbolReadOperator> symbolReadOperator, std::shared_ptr<ReconstructOperator> reconstructOperator)
        : symbolReadOperator_(symbolReadOperator), reconstructOperator_(reconstructOperator) {}

    void init(std::string path);

    void deserializeInt(int& val);

    virtual void deserializeHeader(PMSFHeader& header) {
      header.deserialize(buffer_, dataOffset_);
      header_ = header;
    };

    virtual std::shared_ptr<MCGAL::Mesh> deserializeBaseMesh() = 0;

    virtual void deserializeOneRound() = 0;

    virtual std::shared_ptr<MCGAL::Mesh> exportMesh() = 0;

    char* buffer_;
    int dataOffset_ = 0;
    std::shared_ptr<SymbolReadOperator> symbolReadOperator_;
    std::shared_ptr<ReconstructOperator> reconstructOperator_;
    std::shared_ptr<MCGAL::Mesh> mesh_;
    PMSFHeader header_;

  private:
    void readBuffer(std::string path);
};
#endif