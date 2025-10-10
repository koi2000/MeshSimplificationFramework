/*
 * @Author: koi
 * @Date: 2025-09-01 10:29:43
 * @Description: 
 */
#ifndef BASIC_PMSF_HEADER_H
#define BASIC_PMSF_HEADER_H

#include "../common/BufferUtils.h"
#include "header/PMSFHeader.h"
#include <string>

class BasicPMSFHeader : public PMSFHeader {
    int round;

    bool enableCompress = false;
    bool enableQuantization = false;
    bool enableSegmentation = false;

    void serialize(char* buffer, int& offset) {
        writeInt(buffer, offset, round);
    }
};

#endif