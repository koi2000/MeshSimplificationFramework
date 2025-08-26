#ifndef OPTIONS_H
#define OPTIONS_H
#include "../operator/CalculateErrorOperator.h"
#include "../operator/IsRemovableOperator.h"
#include "../operator/RegisterPropertiesOperator.h"
#include "../operator/UpdatePropertiesOperator.h"
#include <string>

/**
 * options需要管理几个事项
 *
 * 可config的东西有
 * 1. isRemovable 这个在edge collapse和vertex removal里都有，在greedy和error based里都有
 * 2. 几种压缩方式fast quardic，priority error, greedy
 * 3. error的定义以及更新的hook需要暴露出去
 */

enum class CompressType { FAST_QUADRIC, PRIORITY_ERROR, GREEDY };

enum class LocalOperatorType { EDGE_COLLAPSE, VERTEX_REMOVAL };

class Options {
  private:
    std::string path;
    std::string outputPath;
    std::vector<std::pair<CompressType, LocalOperatorType>> level_type;
    std::vector<std::shared_ptr<IsRemovableOperator>> isRemovableOperators;
    std::shared_ptr<CalculateErrorOperator> calculateErrorOperator;
    std::shared_ptr<UpdatePropertiesOperator> updatePropertiesOperator;
    std::shared_ptr<RegisterPropertiesOperator> registerPropertiesOperator;

  public:
    Options() = default;
    class Builder {
      private:
        std::unique_ptr<Options> options;

      public:
        Builder() : options(std::make_unique<Options>()) {}

        Builder& withPath(const std::string& path) {
            options->path = path;
            return *this;
        }

        Builder& addLevelType(CompressType compressType, LocalOperatorType operatorType) {
            options->level_type.emplace_back(compressType, operatorType);
            return *this;
        }

        Builder& addIsRemovableOperator(std::shared_ptr<IsRemovableOperator> op) {
            options->isRemovableOperators.push_back(op);
            return *this;
        }

        Builder& withCalculateErrorOperator(std::shared_ptr<CalculateErrorOperator> op) {
            options->calculateErrorOperator = op;
            return *this;
        }

        Builder& withUpdatePropertiesOperator(std::shared_ptr<UpdatePropertiesOperator> op) {
            options->updatePropertiesOperator = op;
            return *this;
        }

        Builder& withRegisterPropertiesOperator(std::shared_ptr<RegisterPropertiesOperator> op) {
            options->registerPropertiesOperator = op;
            return *this;
        }

        std::unique_ptr<Options> build() {
            return std::move(options);
        }
    };

    // Accessors for the private members
    const std::string& getPath() const {
        return path;
    }
    const std::vector<std::pair<CompressType, LocalOperatorType>>& getLevelTypes() const {
        return level_type;
    }
    const std::vector<std::shared_ptr<IsRemovableOperator>>& getIsRemovableOperators() const {
        return isRemovableOperators;
    }
    std::shared_ptr<CalculateErrorOperator> getCalculateErrorOperator() const {
        return calculateErrorOperator;
    }
    std::shared_ptr<UpdatePropertiesOperator> getUpdatePropertiesOperator() const {
        return updatePropertiesOperator;
    }
    std::shared_ptr<RegisterPropertiesOperator> getRegisterPropertiesOperator() const {
        return registerPropertiesOperator;
    }

    // Static method to create a builder
    static Builder createBuilder() {
        return Builder();
    }
};

#endif