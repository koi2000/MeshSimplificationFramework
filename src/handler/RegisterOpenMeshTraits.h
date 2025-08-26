#ifndef REGISTER_OPEN_MESH_TRAITS_H
#define REGISTER_OPEN_MESH_TRAITS_H

#include "TypeTraitsRegistry.h"
#include "openmesh/Property.hh"

struct RegisterOpenMeshTraits {
    RegisterOpenMeshTraits() {
        {
            TypeTraitsRegistry::Traits traits;
            traits.is_vertex_handle = true;
            traits.entity_type = typeid(OpenMesh::VertexHandle);
            TypeTraitsRegistry::registerTraits<OpenMesh::VPropHandleT<void>>(traits);
        }

        {
            TypeTraitsRegistry::Traits traits;
            traits.is_halfedge_handle = true;
            traits.entity_type = typeid(OpenMesh::HalfedgeHandle);
            TypeTraitsRegistry::registerTraits<OpenMesh::HPropHandleT<void>>(traits);
        }

        {
            TypeTraitsRegistry::Traits traits;
            traits.is_face_handle = true;
            traits.entity_type = typeid(OpenMesh::FaceHandle);
            TypeTraitsRegistry::registerTraits<OpenMesh::FPropHandleT<void>>(traits);
        }
    }
};
static RegisterOpenMeshTraits registerOpenMeshTraits;

#endif