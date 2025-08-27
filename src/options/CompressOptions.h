/*
 * @Author: koi
 * @Date: 2025-08-27 21:18:59
 * @Description: 存储用于压缩所使用的各种option
 */

#pragma

#include <string>

class CompressOptions {
  public:
    CompressOptions() = default;
    ~CompressOptions() = default;

  private:
    std::string path;
};
