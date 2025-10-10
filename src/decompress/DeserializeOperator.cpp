/*
 * @Author: koi
 * @Date: 2025-09-02 10:35:42
 * @Description:
 */
#include "DeserializeOperator.h"
#include "../common/BufferUtils.h"
#include "../common/EntropyCodec.h"
#include <cstring>
#include <fstream>
#include <memory>

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

    bool enableCompress = false;
    if (enableCompress) {
        std::unique_ptr<char[]> temp(new char[len2]);
        fin.read(temp.get(), len2);
        fin.close();
        int localOffset = 0;
        int origSize = readInt(temp.get(), localOffset);
        int compSize = readInt(temp.get(), localOffset);

        dataOffset_ = 0;
        if (origSize <= 0) {
            buffer_ = nullptr;
            return;
        }
        if (compSize == 0) {
            buffer_ = new char[origSize];
            std::memcpy(buffer_, temp.get() + localOffset, static_cast<size_t>(origSize));
            return;
        }
        char* outPtr = nullptr;
        int outSize = 0;
        bool ok = zlibDecompressAlloc(temp.get() + localOffset, compSize, origSize, outPtr, outSize);
        if (!ok || outSize != origSize) {
            buffer_ = nullptr;
            return;
        }
        buffer_ = outPtr;  // ownership transferred
    } else {
        buffer_ = new char[len2];
        memset(buffer_, 0, len2);
        fin.read(buffer_, len2);
    }
}
