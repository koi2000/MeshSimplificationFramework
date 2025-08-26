#ifndef VERTEX_MEMOVAL_DECODER_H
#define VERTEX_MEMOVAL_DECODER_H

#include <string>

/**
 * 无论是什么类型的encoder，都可以使用同一种decoder去做
 * 本质上decoder就是利用各种symbol来进行解压缩
 */
class VertexRemovalGreedyDecoder {
  public:
    VertexRemovalGreedyDecoder(std::string path);

    VertexRemovalGreedyDecoder(char* buffer);

    void decodeInside();

    void decodeBoundary();

    void readMeta();
    
  private:
    char* buffer_;
};

#endif