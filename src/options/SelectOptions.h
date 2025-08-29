/*
 * @Author: koi
 * @Description: SelectOperator的配置项，包括使用那个propertity作为error项，以及如何阻止一个半边被选择
 */
#ifndef SELECT_OPTIONS_H
#define SELECT_OPTIONS_H

#include "../operator/IsRemovableOperator.h"
#include "selection/ErrorSource.h"
#include <memory>
#include <vector>
#include <functional>


/**
 * 需要传入error accessor，通过传入func的形式来做，而不是指定属性了
 */

class SelectOptions {
public:
    SelectOptions() = default;
    virtual ~SelectOptions() = default;

    // 获取半边误差访问器
    const std::function<float(MCGAL::Halfedge*)>& getHalfedgeErrorAccessor() const {
        return halfedgeErrorAccessor_;
    }

    // 获取顶点误差访问器
    const std::function<float(MCGAL::Vertex*)>& getVertexErrorAccessor() const {
        return vertexErrorAccessor_;
    }

    // 获取ErrorSource
    ErrorSource getErrorSource() const {
        return errorSource_;
    }

    // 获取所有可移除算子
    const std::vector<std::shared_ptr<IsRemovableOperator>>& getIsRemovableOperators() const {
        return isRemovableOperators_;
    }

    // 设置半边误差访问器
    void setHalfedgeErrorAccessor(std::function<float(MCGAL::Halfedge*)> accessor) {
        halfedgeErrorAccessor_ = std::move(accessor);
    }

    // 设置顶点误差访问器
    void setVertexErrorAccessor(std::function<float(MCGAL::Vertex*)> accessor) {
        vertexErrorAccessor_ = std::move(accessor);
    }

    // 设置ErrorSource
    void setErrorSource(ErrorSource source) {
        errorSource_ = source;
    }

    // 添加可移除算子
    void addIsRemovableOperator(std::shared_ptr<IsRemovableOperator> op) {
        isRemovableOperators_.push_back(std::move(op));
    }

    class Builder {
    public:
        Builder() = default;

        Builder& withHalfedgeErrorAccessor(std::function<float(MCGAL::Halfedge*)> accessor) {
            halfedgeErrorAccessor_ = std::move(accessor);
            return *this;
        }

        Builder& withVertexErrorAccessor(std::function<float(MCGAL::Vertex*)> accessor) {
            vertexErrorAccessor_ = std::move(accessor);
            return *this;
        }

        Builder& withErrorSource(ErrorSource source) {
            errorSource_ = source;
            return *this;
        }

        Builder& withIsRemovableOperator(std::shared_ptr<IsRemovableOperator> op) {
            isRemovableOperators_.push_back(std::move(op));
            return *this;
        }

        Builder& withIsRemovableOperators(const std::vector<std::shared_ptr<IsRemovableOperator>>& ops) {
            isRemovableOperators_.insert(isRemovableOperators_.end(), ops.begin(), ops.end());
            return *this;
        }

        std::shared_ptr<SelectOptions> build() {
            auto options = std::make_shared<SelectOptions>();
            options->setHalfedgeErrorAccessor(std::move(halfedgeErrorAccessor_));
            options->setVertexErrorAccessor(std::move(vertexErrorAccessor_));
            options->setErrorSource(errorSource_);
            for (auto& op : isRemovableOperators_) {
                options->addIsRemovableOperator(std::move(op));
            }
            return options;
        }

        Builder& reset() {
            halfedgeErrorAccessor_ = nullptr;
            vertexErrorAccessor_ = nullptr;
            isRemovableOperators_.clear();
            return *this;
        }

    private:
        ErrorSource errorSource_{};
        std::function<float(MCGAL::Halfedge*)> halfedgeErrorAccessor_;
        std::function<float(MCGAL::Vertex*)> vertexErrorAccessor_;
        std::vector<std::shared_ptr<IsRemovableOperator>> isRemovableOperators_;
    };

    static Builder builder() {
        return Builder{};
    }

private:
    ErrorSource errorSource_{};
    std::function<float(MCGAL::Halfedge*)> halfedgeErrorAccessor_;
    std::function<float(MCGAL::Vertex*)> vertexErrorAccessor_;
    std::vector<std::shared_ptr<IsRemovableOperator>> isRemovableOperators_;
};

#endif