/*
 * @Author: koi
 * @Date: 2025-09-01 10:04:28
 * @Description:
 */
#ifndef PMSF_HEADER_H
#define PMSF_HEADER_H

#include "../common/BufferUtils.h"
#include <string>

class PMSFHeader {
    int round;

    void serialize(char* buffer, int& offset) {
        writeInt(buffer, offset, round);
    }
};

#endif