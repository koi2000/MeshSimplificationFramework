/*
 * @Author: koi
 * @Date: 2025-08-27 21:18:59
 * @Description: 存储用于压缩所使用的各种option
 */

#include "decompress/DeserializeOperator.h"
#include <memory>
#pragma

#include <string>

class DecompressOptions {
  public:
    DecompressOptions() = default;
    ~DecompressOptions() = default;

    class Builder {
      public:
        Builder& setDeserializeOperator(const std::shared_ptr<DeserializeOperator>& op) {
            deserializeOperator_ = op;
            return *this;
        }
        Builder& setPath(const std::string& p) {
            path_ = p;
            return *this;
        }
        Builder& setOutputPath(const std::string& out) {
            outputPath_ = out;
            return *this;
        }
        DecompressOptions build() {
            DecompressOptions options;
            options.deserializeOperator_ = deserializeOperator_;
            options.path = path_;
            options.outputPath = outputPath_;
            return options;
        }
      private:
        std::shared_ptr<DeserializeOperator> deserializeOperator_;
        std::string path_;
        std::string outputPath_;
    };

    // Getter方法
    std::shared_ptr<DeserializeOperator> getDeserializeOperator() const {
        return deserializeOperator_;
    }
    std::string getPath() const {
        return path;
    }
    std::string getOutputPath() const {
        return outputPath;
    }

  private:
    std::shared_ptr<DeserializeOperator> deserializeOperator_;
    std::string path;
    std::string outputPath;
};
