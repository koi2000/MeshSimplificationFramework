#ifndef MESH_H
#define MESH_H
#include "Configuration.h"
#include "JemallocStl.h"
#include "VertexSplitNode.h"
#include <algorithm>
#include <stdlib.h>
#include <string>
#include <vector>
namespace MCGAL {
class Vector3;
class Vertex;
class Halfedge;
class Facet;
class Point;

#define MESH_BUCKET_SIZE 4096
class Mesh {
  public:
    Mesh() {
        faces_.reserve(MESH_BUCKET_SIZE);
        vertices_.reserve(MESH_BUCKET_SIZE);
    }
    ~Mesh();

    std::vector<Vertex*>& vertices() {
        return vertices_;
    }

    std::vector<Facet*>& faces() {
        return faces_;
    }

    // IOS
    bool loadOFF(std::string path);
    void dumpto(std::string path);
    void print();
    std::string to_string();
    Vertex* get_vertex(int vseq = 0);

    int meshId() {
        return meshId_;
    }

    void setMeshId(int meshId) {
        this->meshId_ = meshId;
    }

    static bool is_collapse_ok(MCGAL::Halfedge* v0v1);

    static bool IsFlipped(MCGAL::Halfedge* edge, const MCGAL::Point& ptTarget);

    // element operating
    Vertex* add_vertex(Vertex* vertex);
    Facet* add_face(std::vector<Vertex*>& vs);
    Facet* add_face(Facet* face);
    void eraseFacetByPointer(Facet* facet);
    void eraseVertexByPointer(Vertex* vertex);
    void dumpto_oldtype(std::string path);
    void submesh_dumpto_oldtype(std::string path, int groupId);

    Facet* remove_vertex(Vertex* v);
    Halfedge* merge_edges(Vertex* v);

    size_t size_of_vertices() {
        return vertices_.size();
    }

    size_t size_of_facets() {
        return faces_.size();
    }

    void resetState();

    void garbage_collection();

    bool isEdgeCollapseValid(MCGAL::Halfedge* edge);

    MCGAL::Vertex* halfedge_collapse(MCGAL::Halfedge* h, MCGAL::Point newp);

    MCGAL::Vertex* halfedge_collapse(MCGAL::Halfedge* h);

    MCGAL::Halfedge* vertex_split(MCGAL::Vertex* v, MCGAL::VertexSplitNode* node);

    Halfedge* split_facet(Halfedge* h, Halfedge* g);

    Halfedge* split_facet_non_meshId(Halfedge* h, Halfedge* g);

    Halfedge* create_center_vertex(Halfedge* h);

    Halfedge* create_center_vertex(Halfedge* h, Point point);

    // used for decode
    Halfedge* create_center_vertex_without_init(Halfedge* h, Point point);

    inline void close_tip(Halfedge* h, Vertex* v) const;

    inline void insert_tip(Halfedge* h, Halfedge* v) const;

    Halfedge* find_prev(Halfedge* h) const;

    Halfedge* erase_center_vertex(Halfedge* h);

    void set_face_in_face_loop(Halfedge* h, Facet* f) const;

    inline void remove_tip(Halfedge* h) const;

    Halfedge* join_face(Halfedge* h);

    MCGAL::Vector3 computeNormal();

    void driftAlongNormal(int step);

  public:
    JemallocVector<Vertex*> vertices_;
    JemallocVector<Facet*> faces_;
    int meshId_ = -1;
    int nb_vertices, nb_faces, nb_edges;
};
}  // namespace MCGAL
#endif