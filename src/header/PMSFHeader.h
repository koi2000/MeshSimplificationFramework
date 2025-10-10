/*
 * @Author: koi
 * @Date: 2025-09-01 10:04:28
 * @Description:
 */
#ifndef PMSF_HEADER_H
#define PMSF_HEADER_H

#include "../common/BufferUtils.h"

class PMSFHeader {
  protected:
    int round = 0;
    int groupNumber = 0;
    bool enableCompress = false;
    bool enableQuantization = false;
    bool enableSegmentation = false;

  public:
    PMSFHeader() = default;

    // Builder模式
    class Builder {
      private:
        int _round = 0;
        int _groupNumber = 0;
        bool _enableCompress = false;
        bool _enableQuantization = false;
        bool _enableSegmentation = false;

      public:
        Builder& setRound(int r) {
            _round = r;
            return *this;
        }
        Builder& setGroupNumber(int groupNumber) {
            _groupNumber = groupNumber;
            return *this;
        }
        Builder& setEnableCompress(bool v) {
            _enableCompress = v;
            return *this;
        }
        Builder& setEnableQuantization(bool v) {
            _enableQuantization = v;
            return *this;
        }
        Builder& setEnableSegmentation(bool v) {
            _enableSegmentation = v;
            return *this;
        }
        PMSFHeader build() {
            PMSFHeader header;
            header.round = _round;
            header.groupNumber = _groupNumber;
            header.enableCompress = _enableCompress;
            header.enableQuantization = _enableQuantization;
            header.enableSegmentation = _enableSegmentation;
            return header;
        }
    };

    // getter
    int getRound() const {
        return round;
    }
    int getGroupNumber() const {
        return groupNumber;
    }
    bool isEnableCompress() const {
        return enableCompress;
    }
    bool isEnableQuantization() const {
        return enableQuantization;
    }
    bool isEnableSegmentation() const {
        return enableSegmentation;
    }

    // setter
    void setRound(int r) {
        round = r;
    }
    void setGroupNumber(int g) {
        groupNumber = g;
    }
    void setEnableCompress(bool v) {
        enableCompress = v;
    }
    void setEnableQuantization(bool v) {
        enableQuantization = v;
    }
    void setEnableSegmentation(bool v) {
        enableSegmentation = v;
    }

    // 完整的序列化方法
    virtual void serialize(char* buffer, int& offset) const {
        writeInt(buffer, offset, round);
        unsigned bitOffset = 0;
        writeBool(buffer, bitOffset, offset, enableCompress);
        writeBool(buffer, bitOffset, offset, enableQuantization);
        writeBool(buffer, bitOffset, offset, enableSegmentation);
        offset += 1;
        if (enableSegmentation) {
            writeInt(buffer, offset, groupNumber);
        }
    }

    // 反序列化方法（可选）
    virtual void deserialize(char* buffer, int& offset) {
        round = readInt(buffer, offset);
        unsigned bitOffset = 0;
        enableCompress = readBool(buffer, bitOffset, offset);
        enableQuantization = readBool(buffer, bitOffset, offset);
        enableSegmentation = readBool(buffer, bitOffset, offset);
        offset += 1;
        if (enableSegmentation) {
            groupNumber = readInt(buffer, offset);
        }
    }
};

#endif