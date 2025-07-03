#include "include/SimpleDecoder.h"
#include "BufferUtils.h"
#include "VertexSplitNode.h"
#include "biops.h"
#include <fstream>
#include <queue>
#include <unordered_set>

SimpleDecoder::SimpleDecoder(std::string path) {
    loadBuffer(path);
    readBaseMeshs();
    readSeed();
}

SimpleDecoder::~SimpleDecoder() {}

void SimpleDecoder::readSeed() {
    int st = readInt(buffer, dataOffset);
    int ed = readInt(buffer, dataOffset);
    MCGAL::Vertex* v1 = mesh.vertices()[st];
    MCGAL::Vertex* v2 = mesh.vertices()[ed];
    for (MCGAL::Halfedge* hit : v1->halfedges()) {
        if (hit->end_vertex() == v2) {
            seed = hit;
            break;
        }
    }
}

MCGAL::VertexSplitNode* readVertexSplitNode(char* buffer, int& dataOffset) {
    MCGAL::VertexSplitNode* node = new MCGAL::VertexSplitNode();
    node->c = readPoint(buffer, dataOffset);
    node->d = readPoint(buffer, dataOffset);
    node->bitmap = readInt16(buffer, dataOffset);
    node->order = readInt(buffer, dataOffset);
    return node;
}

MCGAL::VertexSplitNode* deserialize(char* buffer, int& dataOffset) {
    uint16_t total_bits = readuInt16(buffer, dataOffset);
    short bitmap_bytes = (total_bits + 7) / 8;
    if (bitmap_bytes == 0)
        return nullptr;
    char* bitmap = readCharPointer(buffer, dataOffset, bitmap_bytes);

    MCGAL::VertexSplitNode* root = readVertexSplitNode(buffer, dataOffset);
    std::queue<std::pair<MCGAL::VertexSplitNode*, size_t>> q;
    q.push({root, 0});
    size_t data_idx = 1;

    while (!q.empty()) {
        auto [node, index] = q.front();
        q.pop();

        size_t left_idx = 2 * index + 1;
        if (left_idx < total_bits && getBit(bitmap, left_idx)) {
            node->left = readVertexSplitNode(buffer, dataOffset);
            q.push({node->left, left_idx});
        }

        size_t right_idx = 2 * index + 2;
        if (right_idx < total_bits && getBit(bitmap, right_idx)) {
            node->right = readVertexSplitNode(buffer, dataOffset);
            q.push({node->right, right_idx});
        }
    }
    return root;
}

void SimpleDecoder::decode() {
    std::vector<MCGAL::Vertex*> collapsed_vertexs = decodeVertexSymbol();
    // std::deque<MCGAL::Vertex*> q;
    auto cmp = [](MCGAL::Vertex* a, MCGAL::Vertex* b) { return a->vsplitNode()->order < b->vsplitNode()->order; };
    std::priority_queue<MCGAL::Vertex*, std::vector<MCGAL::Vertex*>, decltype(cmp)> q(cmp);
    for (MCGAL::Vertex* v : collapsed_vertexs) {
        // std::cout << dataOffset << " ";
        v->setVertexSplitNode(deserialize(buffer, dataOffset));
        q.push(v);
    }
    std::string path = "./tp/res_origin.off";
    mesh.dumpto_oldtype(path);
    int idx = 5833;
    while (!q.empty()) {
        MCGAL::Vertex* v = q.top();
        q.pop();
        MCGAL::VertexSplitNode* node = v->vsplitNode();
        // if (node->order != idx--) {
        //     std::cout << "error" << std::endl;
        // }
        std::string path = "./tp2/res" + std::to_string(node->order) + ".off";
        // if (node->order > 10000) {
        // mesh.dumpto_oldtype(path);
        // }
        MCGAL::Halfedge* hit = mesh.vertex_split(v, node);

        // }
        if (hit->vertex()->vsplitNode()) {
            // v->setVertexSplitNode(v->vsplitNode()->left);
            q.push(hit->vertex());
        }
        if (hit->end_vertex()->vsplitNode()) {
            q.push(hit->end_vertex());
        }
    }
}

std::vector<MCGAL::Vertex*> SimpleDecoder::decodeVertexSymbol() {
    // mesh.garbage_collection();
    std::vector<MCGAL::Vertex*> collapsed_vertexs;
    std::deque<MCGAL::VertexSplitNode*> nodes;
    std::queue<int> gateQueue;
    gateQueue.push(seed->face()->poolId());

    int bitmapsize = readInt(buffer, dataOffset);
    char* bitmap = readCharPointer(buffer, dataOffset, bitmapsize / 8 + 1);
    int idx = 0;
    while (!gateQueue.empty()) {
        int fid = gateQueue.front();
        MCGAL::Facet* f = MCGAL::contextPool.getFacetByIndex(mesh.meshId(), fid);
        gateQueue.pop();
        if (f->isProcessed()) {
            continue;
        }
        int vpoolId = -1;
        f->setProcessedFlag();

        MCGAL::Halfedge* st = f->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        MCGAL::Vertex* minVid = st->vertex();
        MCGAL::Halfedge* hit = st;
        do {
            if (st->vertex()->point() > minVid->point()) {
                hit = st;
                minVid = st->vertex();
            }
            st = st->next();
        } while (st != ed);

        st = hit;
        ed = st;
        do {
            if (!st->vertex()->isProcessed()) {
                st->vertex()->setCollapsed();
                st->vertex()->setProcessed();
                if (getBit(bitmap, idx++)) {
                    collapsed_vertexs.push_back(st->vertex());
                }
            }

            if (!st->opposite()->face()->isProcessed()) {
                gateQueue.push(st->opposite()->face()->poolId());
            }
            st = st->next();
        } while (st != ed);
    }
    return collapsed_vertexs;
}

void SimpleDecoder::loadBuffer(std::string path) {
    std::ifstream fin(path, std::ios::binary);
    int len2;
    fin.read((char*)&len2, sizeof(int));
    dataOffset = 0;
    buffer = new char[len2];
    memset(buffer, 0, len2);
    fin.read(buffer, len2);
}

void SimpleDecoder::readBaseMeshs() {
    mesh.setMeshId(0);
    unsigned i_nbVerticesBaseMesh = readInt(buffer, dataOffset);
    unsigned i_nbFacesBaseMesh = readInt(buffer, dataOffset);
    MCGAL::contextPool.initPoolSize(1);
    MCGAL::contextPool.registerPool(i_nbVerticesBaseMesh, i_nbFacesBaseMesh * 3, i_nbFacesBaseMesh);

    std::deque<MCGAL::Point>* p_pointDeque = new std::deque<MCGAL::Point>();
    std::deque<uint32_t*>* p_faceDeque = new std::deque<uint32_t*>();
    for (unsigned i = 0; i < i_nbVerticesBaseMesh; ++i) {
        MCGAL::Point pos = readPoint(buffer, dataOffset);
        p_pointDeque->push_back(pos);
    }
    for (unsigned i = 0; i < i_nbFacesBaseMesh; ++i) {
        int nv = readInt(buffer, dataOffset);
        uint32_t* f = new uint32_t[nv + 1];
        f[0] = nv;
        for (unsigned j = 1; j < nv + 1; ++j) {
            f[j] = readInt(buffer, dataOffset);
        }
        p_faceDeque->push_back(f);
    }
    buildFromBuffer(mesh, p_pointDeque, p_faceDeque);

    for (unsigned i = 0; i < p_faceDeque->size(); ++i) {
        delete[] p_faceDeque->at(i);
    }
    delete p_faceDeque;
    delete p_pointDeque;
}

void SimpleDecoder::buildFromBuffer(MCGAL::Mesh& mesh, std::deque<MCGAL::Point>* p_pointDeque, std::deque<uint32_t*>* p_faceDeque) {
    mesh.vertices().clear();
    mesh.faces().clear();
    std::vector<MCGAL::Vertex*> vertices;
    for (std::size_t i = 0; i < p_pointDeque->size(); ++i) {
        // float x, y, z;
        MCGAL::Point p = p_pointDeque->at(i);
        MCGAL::Vertex* vt = MCGAL::contextPool.allocateVertexFromPool(mesh.meshId(), p);
        mesh.vertices().push_back(vt);
        vertices.push_back(vt);
    }
    for (int t = 0; t < p_faceDeque->size(); ++t) {
        uint32_t* ptr = p_faceDeque->at(t);
        int num_face_vertices = ptr[0];
        std::vector<MCGAL::Vertex*> vts;
        for (int j = 0; j < num_face_vertices; ++j) {
            int vertex_index = ptr[j + 1];
            vts.push_back(vertices[vertex_index]);
        }
        MCGAL::Facet* face = MCGAL::contextPool.allocateFaceFromPool(mesh.meshId(), vts);
        mesh.add_face(face);
    }
    vertices.clear();
}

void SimpleDecoder::dumpTo(std::string path) {
    mesh.dumpto_oldtype(path);
}