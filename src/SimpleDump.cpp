#include "BufferUtils.h"
#include "VertexSplitNode.h"
#include "biops.h"
#include "include/SimpleEncoder.h"
#include <fstream>
#include <unordered_set>

void set_bit(std::vector<char>& bitmap, size_t bitIndex, bool value) {
    // size_t byte_idx = index / 8;
    // size_t bit_offset = index % 8;
    // if (byte_idx >= bitmap.size()) {
    //     bitmap.resize(byte_idx + 1, 0);
    //     // bitmap.insert(bitmap.begin(), 0);
    // }
    // if (value) {
    //     bitmap[byte_idx] |= (1 << (7 - bit_offset));  // 高位在前
    // } else {
    //     bitmap[byte_idx] &= ~(1 << (7 - bit_offset));
    // }

    int byteIndex = bitIndex / 8;
    unsigned char bitMask = 1 << (bitIndex % 8);
    if (byteIndex >= bitmap.size()) {
        bitmap.resize(bitIndex + 1, 0);
        // bitmap.insert(bitmap.begin(), 0);
    }
    bitmap[byteIndex] |= bitMask;
}

// 序列化树到文件
void serialize(MCGAL::VertexSplitNode* root, char* buffer, int& dataOffset) {
    if (!root) {
        return;
    }
    std::queue<std::pair<MCGAL::VertexSplitNode*, size_t>> q;
    std::vector<char> bitmap;
    std::vector<MCGAL::VertexSplitNode*> data;
    size_t max_index = 0;

    q.push({root, 0});
    set_bit(bitmap, 0, true);
    data.push_back(root);

    while (!q.empty()) {
        auto [node, index] = q.front();
        q.pop();

        max_index = std::max(max_index, index);

        size_t left_idx = 2 * index + 1;
        if (node->left) {
            set_bit(bitmap, left_idx, true);
            data.push_back(node->left);
            q.push({node->left, left_idx});
        } else {
            // set_bit(bitmap, left_idx, false);
        }

        size_t right_idx = 2 * index + 2;
        if (node->right) {
            set_bit(bitmap, right_idx, true);
            data.push_back(node->right);
            q.push({node->right, right_idx});
        } else {
            // set_bit(bitmap, right_idx, false);
        }
    }
    size_t total_bits = max_index + 1;
    short bitmap_bytes = (total_bits + 7) / 8;
    // bitmap.resize(bitmap_bytes, 0);
    // 存储bitmap的大小
    writeuInt16(buffer, dataOffset, total_bits);
    // 存储bitmap
    writeCharPointer(buffer, dataOffset, bitmap.data(), bitmap_bytes);
    // 紧凑布局，存储所有的node
    for (int i = 0; i < data.size(); i++) {
        writePoint(buffer, dataOffset, data[i]->c);
        writePoint(buffer, dataOffset, data[i]->d);
        // writeCharPointer(buffer, dataOffset, data[i]->bitmap, 2);
        writeInt16(buffer, dataOffset, data[i]->bitmap);
        writeInt(buffer, dataOffset, data[i]->order);
    }
}

void SimpleEncoder::dumpToBuffer() {
    writeBaseMesh();
    writeInt(buffer, dataOffset, seed->vertex()->vid());
    writeInt(buffer, dataOffset, seed->end_vertex()->vid());

    for (int i = vertexSymbols.size() - 1; i >= 0; i--) {
        std::deque<char>& symbols = vertexSymbols[i];
        std::deque<MCGAL::VertexSplitNode*>& nodes = vertexSplitNodes[i];

        int bitmapsize = symbols.size() / 8 + 1;
        char* bitmap = new char[bitmapsize];
        memset(bitmap, 0, bitmapsize);
        for (size_t j = 0; j < symbols.size(); j++) {
            if (symbols[j] == 1) {
                setBit(bitmap, j);
            }
        }
        writeInt(buffer, dataOffset, symbols.size());
        writeCharPointer(buffer, dataOffset, bitmap, bitmapsize);
        for (size_t j = 0; j < nodes.size(); j++) {
            // std::cout << dataOffset << " ";
            MCGAL::VertexSplitNode* node = nodes[j];
            serialize(node, buffer, dataOffset);
        }
    }

    dumpToFile("./bunny_try.loc");
}

void SimpleEncoder::dumpToFile(std::string path) {
    std::ofstream fout(path, std::ios::binary);
    int len = dataOffset;
    fout.write((char*)&len, sizeof(int));
    fout.write(buffer, len);
    fout.close();
}

void SimpleEncoder::writeBaseMesh() {
    MCGAL::Mesh& subMesh = mesh;
    unsigned i_nbVerticesBaseMesh = subMesh.size_of_vertices();
    unsigned i_nbFacesBaseMesh = subMesh.size_of_facets();

    writeInt(buffer, dataOffset, i_nbVerticesBaseMesh);
    writeInt(buffer, dataOffset, i_nbFacesBaseMesh);
    int id = 0;
    for (MCGAL::Vertex* vit : subMesh.vertices()) {
        if (vit->isRemoved()) {
            continue;
        }
        MCGAL::Point point = vit->point();
        writePoint(buffer, dataOffset, point);
        vit->setVid(id++);
    }
    for (MCGAL::Facet* fit : subMesh.faces()) {
        if (fit->isRemoved()) {
            continue;
        }
        unsigned i_faceDegree = fit->facet_degree();
        writeInt(buffer, dataOffset, i_faceDegree);
        MCGAL::Halfedge* st = fit->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        std::vector<int> ids;
        do {
            writeInt(buffer, dataOffset, st->vertex()->vid());
            ids.push_back(st->vertex()->vid());
            st = st->next();
        } while (st != ed);
    }
}

void SimpleEncoder::dumpTo(std::string path) {
    mesh.dumpto_oldtype(path);
}
