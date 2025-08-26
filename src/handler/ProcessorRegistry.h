#ifndef PROCESSOR_REGISTRY_H
#define PROCESSOR_REGISTRY_H

#include "PropertyHandleWrapper.h"
#include <functional>
#include <unordered_map>

class ProcessorRegistry {
  public:
    using ProcessorFunc = std::function<void(void*, void*, const PropertyHandleWrapper*)>;

    template <typename HandleType> void registerProcessor(ProcessorFunc processor) {
        processors_[typeid(HandleType)] = std::move(processor);
    }

    ProcessorFunc getProcessor(const std::type_info& handleType) {
        auto it = processors_.find(handleType);
        if (it == processors_.end()) {
            return nullptr;
        }
        return it->second;
    }

    static ProcessorRegistry& instance() {
        static ProcessorRegistry registry;
        return registry;
    }

  private:
    std::unordered_map<std::type_index, ProcessorFunc> processors_;
};

#define REGISTER_PROPERTY_HANDLE(HandleType, EntityCategory, EntityType)                                                                                                                               \
    namespace {                                                                                                                                                                                        \
        struct Register##HandleType##Traits {                                                                                                                                                          \
            Register##HandleType##Traits() {                                                                                                                                                           \
                TypeTraitsRegistry::Traits traits;                                                                                                                                                     \
                if (std::string(#EntityCategory) == "Vertex") {                                                                                                                                        \
                    traits.is_vertex_handle = true;                                                                                                                                                    \
                } else if (std::string(#EntityCategory) == "Halfedge") {                                                                                                                               \
                    traits.is_halfedge_handle = true;                                                                                                                                                  \
                } else if (std::string(#EntityCategory) == "Face") {                                                                                                                                   \
                    traits.is_face_handle = true;                                                                                                                                                      \
                } else {                                                                                                                                                                               \
                    throw std::runtime_error("Invalid entity category: " #EntityCategory);                                                                                                             \
                }                                                                                                                                                                                      \
                traits.entity_type = typeid(EntityType);                                                                                                                                               \
                TypeTraitsRegistry::registerTraits<HandleType>(traits);                                                                                                                                \
            }                                                                                                                                                                                          \
        };                                                                                                                                                                                             \
        static Register##HandleType##Traits register##HandleType##Traits;                                                                                                                              \
    }

// // 用户友好的处理器注册函数
// template <typename HandleType> void registerPropertyProcessor(std::function<void(OpenMesh::TriMesh_ArrayKernelT<>&, MCGAL::Halfedge*, const HandleType&)> processor) {
//     ProcessorRegistry::instance().registerProcessor<HandleType>([processor](void* mesh, void* halfedge, const PropertyHandleWrapper* wrapper) {
//         auto* typedWrapper = dynamic_cast<const PropertyHandleWrapperImpl<HandleType>*>(wrapper);
//         if (!typedWrapper) {
//             throw std::runtime_error("Invalid handle type for processor");
//         }
//         processor(*reinterpret_cast<OpenMesh::TriMesh_ArrayKernelT<>*>(mesh), reinterpret_cast<MCGAL::Halfedge*>(halfedge), typedWrapper->get());
//     });
// }
#endif