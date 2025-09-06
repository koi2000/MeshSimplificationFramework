/*
 * @Author: koi
 * @Date: 2025-09-02 10:35:42
 * @Description:
 */
#include "DeserializeOperator.h"
#include "../common/BufferUtils.h"
#include <cstring>
#include <fstream>

void DeserializeOperator::init(std::string path) {
    readBuffer(path);
}

void DeserializeOperator::deserializeInt(int& val) {
    val = readInt(buffer_, dataOffset_);
}

void DeserializeOperator::readBuffer(std::string path) {
    std::ifstream fin(path, std::ios::binary);
    int len2;
    fin.read((char*)&len2, sizeof(int));
    dataOffset_ = 0;
    buffer_ = new char[len2];
    memset(buffer_, 0, len2);
    fin.read(buffer_, len2);
}
