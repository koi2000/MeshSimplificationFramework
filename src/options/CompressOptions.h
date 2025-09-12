/*
 * @Author: koi
 * @Date: 2025-08-27 21:18:59
 * @Description: 存储用于压缩所使用的各种option
 */

#ifndef COMPRESSION_OPTIONS_H
#define COMPRESSION_OPTIONS_H

#include "../elimination/EliminateOperator.h"
#include "../selection/SelectOperator.h"
#include <memory>
#include <string>

class CompressOptions {
  public:
    CompressOptions() = default;
    ~CompressOptions() = default;

    class Builder {
      public:
        Builder() : round_(0), enablePrediction_(false), enableQuantization_(false) {}

        Builder& setRound(const int r) {
            round_ = r;
            return *this;
        }

        Builder& setPath(const std::string& p) {
            path_ = p;
            return *this;
        }
        Builder& setOutputPath(const std::string& p) {
            outputPath_ = p;
            return *this;
        }
        Builder& setSelect(const std::shared_ptr<SelectOperator>& s) {
            select_ = s;
            return *this;
        }
        Builder& setEliminate(const std::shared_ptr<EliminateOperator>& e) {
            eliminate_ = e;
            return *this;
        }
        Builder& setEnableCompress(bool enable) {
            enableCompress_ = enable;
            return *this;
        }
        Builder& setEnablePrediction(bool enable) {
            enablePrediction_ = enable;
            return *this;
        }
        Builder& setEnableQuantization(bool enable) {
            enableQuantization_ = enable;
            return *this;
        }
        CompressOptions build() {
            CompressOptions options;
            options.round = round_;
            options.path = path_;
            options.outputPath = outputPath_;
            options.select = select_;
            options.eliminate = eliminate_;
            options.enableCompress = enableCompress_;
            options.enablePrediction = enablePrediction_;
            options.enableQuantization = enableQuantization_;
            return options;
        }

      private:
        int round_;
        std::string path_;
        std::string outputPath_;
        bool enableCompress_;
        bool enablePrediction_;
        bool enableQuantization_;
        std::shared_ptr<SelectOperator> select_;
        std::shared_ptr<EliminateOperator> eliminate_;
    };

    const std::string& getPath() const {
        return path;
    }

    const std::string& getOutputPath() const {
        return outputPath;
    }
    const int getRound() const {
        return round;
    }
    const std::shared_ptr<SelectOperator>& getSelect() const {
        return select;
    }
    const std::shared_ptr<EliminateOperator>& getEliminate() const {
        return eliminate;
    }
    bool isEnableCompress() const {
        return enableCompress;
    }
    bool isEnablePrediction() const {
        return enablePrediction;
    }
    bool isEnableQuantization() const {
        return enableQuantization;
    }

  private:
    bool enableCompress = false;
    bool enablePrediction = false;
    bool enableQuantization = false;

    std::string path;
    std::string outputPath;
    int round = 0;
    std::shared_ptr<SelectOperator> select;
    std::shared_ptr<EliminateOperator> eliminate;
};

#endif