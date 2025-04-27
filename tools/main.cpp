#include <cmath>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

// 基本数据结构定义
struct Vertex;
struct HalfEdge;
struct Face;

// 顶点结构
struct Vertex {
    int id;
    float x, y, z;                 // 坐标
    HalfEdge* outgoing = nullptr;  // 从该顶点出发的任意一条半边

    // 用于构建顶点优先级队列的构造函数
    Vertex(int _id, float _x, float _y, float _z) : id(_id), x(_x), y(_y), z(_z) {}

    // 计算与另一个顶点的距离
    float distance(const Vertex& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

// 半边结构
struct HalfEdge {
    int id;
    Vertex* origin = nullptr;  // 起点
    HalfEdge* twin = nullptr;  // 对偶半边
    HalfEdge* next = nullptr;  // 面内的下一条半边
    Face* face = nullptr;      // 所属面片

    // 获取半边的终点
    Vertex* destination() const {
        return twin ? twin->origin : nullptr;
    }

    // 获取半边的长度
    float length() const {
        if (!origin || !destination())
            return INFINITY;
        return origin->distance(*destination());
    }
};

// 面片结构（支持多边形）
struct Face {
    int id;
    HalfEdge* halfedge = nullptr;  // 面片的任意一条半边

    // 获取面片的顶点数量
    int vertexCount() const {
        if (!halfedge)
            return 0;

        int count = 0;
        HalfEdge* start = halfedge;
        HalfEdge* current = start;

        do {
            count++;
            current = current->next;
        } while (current != start);

        return count;
    }

    // 获取面片的所有顶点
    std::vector<Vertex*> getVertices() const {
        std::vector<Vertex*> vertices;
        if (!halfedge)
            return vertices;

        HalfEdge* start = halfedge;
        HalfEdge* current = start;

        do {
            vertices.push_back(current->origin);
            current = current->next;
        } while (current != start);

        return vertices;
    }
};

// 用于边坍缩优先队列的边评估结构
struct EdgeCollapseCost {
    HalfEdge* edge;
    float cost;

    // 构造函数
    EdgeCollapseCost(HalfEdge* e, float c) : edge(e), cost(c) {}

    // 用于优先队列的比较操作符
    bool operator>(const EdgeCollapseCost& other) const {
        return cost > other.cost;
    }
};

// 网格类
class HalfEdgeMesh {
  private:
    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<HalfEdge>> halfedges;
    std::vector<std::unique_ptr<Face>> faces;

    int nextVertexId = 0;
    int nextHalfEdgeId = 0;
    int nextFaceId = 0;

  public:
    // 添加顶点
    Vertex* addVertex(float x, float y, float z) {
        auto vertex = std::make_unique<Vertex>(nextVertexId++, x, y, z);
        Vertex* ptr = vertex.get();
        vertices.push_back(std::move(vertex));
        return ptr;
    }

    // 添加半边
    HalfEdge* addHalfEdge(Vertex* origin) {
        auto halfedge = std::make_unique<HalfEdge>();
        halfedge->id = nextHalfEdgeId++;
        halfedge->origin = origin;
        HalfEdge* ptr = halfedge.get();
        halfedges.push_back(std::move(halfedge));
        return ptr;
    }

    // 添加面片
    Face* addFace(const std::vector<Vertex*>& faceVertices) {
        if (faceVertices.size() < 3) {
            std::cerr << "Error: Face must have at least 3 vertices" << std::endl;
            return nullptr;
        }

        auto face = std::make_unique<Face>();
        face->id = nextFaceId++;
        Face* facePtr = face.get();

        // 为面片创建半边
        std::vector<HalfEdge*> faceHalfEdges;
        for (size_t i = 0; i < faceVertices.size(); ++i) {
            HalfEdge* he = addHalfEdge(faceVertices[i]);
            he->face = facePtr;
            faceHalfEdges.push_back(he);
        }

        // 连接面片内的半边
        for (size_t i = 0; i < faceHalfEdges.size(); ++i) {
            faceHalfEdges[i]->next = faceHalfEdges[(i + 1) % faceHalfEdges.size()];
        }

        // 设置面片的引用半边
        facePtr->halfedge = faceHalfEdges[0];

        // 更新顶点的出边
        for (size_t i = 0; i < faceVertices.size(); ++i) {
            faceVertices[i]->outgoing = faceHalfEdges[i];
        }

        faces.push_back(std::move(face));
        return facePtr;
    }

    // 连接对偶半边
    void connectTwins(HalfEdge* he1, HalfEdge* he2) {
        he1->twin = he2;
        he2->twin = he1;
    }

    // 查找连接两个顶点的半边
    HalfEdge* findHalfEdge(Vertex* v1, Vertex* v2) {
        // 遍历从v1出发的所有半边
        HalfEdge* start = v1->outgoing;
        if (!start)
            return nullptr;

        HalfEdge* current = start;
        do {
            if (current->destination() == v2) {
                return current;
            }

            // 切换到下一条从v1出发的半边
            if (current->twin) {
                current = current->twin->next;
            } else {
                // 如果没有对偶边，我们就无法继续遍历了
                break;
            }
        } while (current != start && current);

        return nullptr;
    }

    // 获取顶点的所有邻接顶点
    std::vector<Vertex*> getAdjacentVertices(Vertex* vertex) {
        std::vector<Vertex*> adjacentVertices;
        std::set<int> addedIds;  // 用于防止重复添加

        if (!vertex->outgoing)
            return adjacentVertices;

        HalfEdge* start = vertex->outgoing;
        HalfEdge* current = start;

        do {
            Vertex* adjacent = current->destination();
            if (adjacent && addedIds.find(adjacent->id) == addedIds.end()) {
                adjacentVertices.push_back(adjacent);
                addedIds.insert(adjacent->id);
            }

            // 切换到下一条从此顶点出发的半边
            if (current->twin) {
                current = current->twin->next;
            } else {
                // 如果没有对偶边，我们就无法继续遍历了
                break;
            }
        } while (current != start && current);

        return adjacentVertices;
    }

    // 获取边的相邻面片
    std::vector<Face*> getAdjacentFaces(HalfEdge* edge) {
        std::vector<Face*> adjacentFaces;

        if (edge->face) {
            adjacentFaces.push_back(edge->face);
        }

        if (edge->twin && edge->twin->face) {
            adjacentFaces.push_back(edge->twin->face);
        }

        return adjacentFaces;
    }

    // 检查边坍缩是否合法
    bool isEdgeCollapseValid(HalfEdge* edge) {
        Vertex* v1 = edge->origin;
        Vertex* v2 = edge->destination();

        if (!v1 || !v2)
            return false;

        // 获取边的相邻面片
        std::vector<Face*> edgeFaces = getAdjacentFaces(edge);

        // 获取两个顶点的所有邻接顶点
        std::vector<Vertex*> v1Neighbors = getAdjacentVertices(v1);
        std::vector<Vertex*> v2Neighbors = getAdjacentVertices(v2);

        // 检查坍缩后是否会导致重叠面片或非流形网格
        std::set<Vertex*> commonNeighbors;
        for (Vertex* n1 : v1Neighbors) {
            if (n1 != v2) {  // 排除v2本身
                for (Vertex* n2 : v2Neighbors) {
                    if (n2 != v1 && n1 == n2) {  // 找到共同邻居(排除v1本身)
                        commonNeighbors.insert(n1);
                    }
                }
            }
        }

        // 对于每个共同邻居，检查与v1和v2组成的面片
        for (Vertex* neighbor : commonNeighbors) {
            // 检查是否存在包含v1、v2和neighbor的面片
            bool foundFace = false;
            for (Face* face : edgeFaces) {
                std::vector<Vertex*> faceVertices = face->getVertices();
                bool hasV1 = false, hasV2 = false, hasNeighbor = false;

                for (Vertex* v : faceVertices) {
                    if (v == v1)
                        hasV1 = true;
                    if (v == v2)
                        hasV2 = true;
                    if (v == neighbor)
                        hasNeighbor = true;
                }

                if (hasV1 && hasV2 && hasNeighbor) {
                    foundFace = true;
                    break;
                }
            }

            // 如果没有找到包含三个顶点的面片，可能会导致拓扑问题
            if (!foundFace) {
                // 检查是否存在其他包含这个共同邻居的面片，这可能会导致面片重叠
                Face* faceWithV1 = nullptr;
                Face* faceWithV2 = nullptr;

                for (auto& f : faces) {
                    std::vector<Vertex*> faceVertices = f->getVertices();

                    bool hasV1 = false, hasV2 = false, hasNeighbor = false;
                    for (Vertex* v : faceVertices) {
                        if (v == v1)
                            hasV1 = true;
                        if (v == v2)
                            hasV2 = true;
                        if (v == neighbor)
                            hasNeighbor = true;
                    }

                    if (hasV1 && hasNeighbor && !hasV2)
                        faceWithV1 = f.get();
                    if (hasV2 && hasNeighbor && !hasV1)
                        faceWithV2 = f.get();
                }

                if (faceWithV1 && faceWithV2) {
                    // 坍缩后这两个面片会重叠
                    return false;
                }
            }
        }

        return true;
    }

    // 计算边坍缩代价（可以使用各种度量标准）
    float calculateEdgeCollapseCost(HalfEdge* edge) {
        // 这里使用简单的边长作为代价
        return edge->length();
    }

    // 执行边坍缩操作
    Vertex* collapseEdge(HalfEdge* edge) {
        if (!edge)
            return nullptr;

        Vertex* v1 = edge->origin;
        Vertex* v2 = edge->destination();

        if (!v1 || !v2 || !isEdgeCollapseValid(edge)) {
            return nullptr;
        }

        // 将要被移除的面片
        std::vector<Face*> facesToRemove = getAdjacentFaces(edge);

        // 计算新顶点位置（可以使用各种策略）
        float newX = (v1->x + v2->x) / 2.0f;
        float newY = (v1->y + v2->y) / 2.0f;
        float newZ = (v1->z + v2->z) / 2.0f;

        // 创建新顶点
        Vertex* newVertex = addVertex(newX, newY, newZ);

        // 获取两个端点的一环邻域顶点，用于更新拓扑
        std::vector<Vertex*> v1Neighbors = getAdjacentVertices(v1);
        std::vector<Vertex*> v2Neighbors = getAdjacentVertices(v2);

        // 合并邻居列表并移除v1和v2
        std::set<Vertex*> allNeighbors;
        for (Vertex* v : v1Neighbors) {
            if (v != v2)
                allNeighbors.insert(v);
        }
        for (Vertex* v : v2Neighbors) {
            if (v != v1)
                allNeighbors.insert(v);
        }

        // 收集所有受影响的面片
        std::set<Face*> affectedFaces;
        for (Vertex* neighbor : allNeighbors) {
            // 查找连接v1和邻居的半边
            HalfEdge* edgeToNeighborFromV1 = findHalfEdge(v1, neighbor);
            if (edgeToNeighborFromV1 && edgeToNeighborFromV1->face) {
                affectedFaces.insert(edgeToNeighborFromV1->face);
            }

            // 查找连接邻居和v1的半边
            HalfEdge* edgeFromNeighborToV1 = findHalfEdge(neighbor, v1);
            if (edgeFromNeighborToV1 && edgeFromNeighborToV1->face) {
                affectedFaces.insert(edgeFromNeighborToV1->face);
            }

            // 查找连接v2和邻居的半边
            HalfEdge* edgeToNeighborFromV2 = findHalfEdge(v2, neighbor);
            if (edgeToNeighborFromV2 && edgeToNeighborFromV2->face) {
                affectedFaces.insert(edgeToNeighborFromV2->face);
            }

            // 查找连接邻居和v2的半边
            HalfEdge* edgeFromNeighborToV2 = findHalfEdge(neighbor, v2);
            if (edgeFromNeighborToV2 && edgeFromNeighborToV2->face) {
                affectedFaces.insert(edgeFromNeighborToV2->face);
            }
        }

        // 移除包含要坍缩边的面片
        for (Face* face : facesToRemove) {
            affectedFaces.erase(face);
        }

        // 为剩余的受影响面片创建新的半边结构
        std::vector<std::vector<Vertex*>> newFaceVertices;
        for (Face* face : affectedFaces) {
            std::vector<Vertex*> faceVerts = face->getVertices();
            std::vector<Vertex*> newFaceVerts;

            for (Vertex* v : faceVerts) {
                if (v == v1 || v == v2) {
                    // 替换为新顶点
                    bool flag = true;
                    for (int i = 0; i < newFaceVerts.size(); i++) {
                        if (newFaceVerts[i] == v) {
                            flag = false;
                            break;
                        }
                    }
                    if (flag) {
                        newFaceVerts.push_back(newVertex);
                    }

                } else {
                    newFaceVerts.push_back(v);
                }
            }

            // 如果新面片至少有3个顶点，则保存
            if (newFaceVerts.size() >= 3) {
                newFaceVertices.push_back(newFaceVerts);
            }
        }

        // 清理受影响的半边和面片（在实际实现中应该更小心地管理内存）
        // 此处简化处理：直接移除所有受影响的面片，然后添加新的面片

        // 创建新的面片
        for (const auto& faceVerts : newFaceVertices) {
            addFace(faceVerts);
        }

        return newVertex;
    }

    // 网格简化方法
    void simplify(int targetFaceCount) {
        // 构建边坍缩优先队列
        std::priority_queue<EdgeCollapseCost, std::vector<EdgeCollapseCost>, std::greater<EdgeCollapseCost>> edgeQueue;

        // 收集所有边并计算坍缩代价
        std::set<std::pair<int, int>> processedEdges;  // 用于防止重复添加

        for (const auto& he : halfedges) {
            Vertex* v1 = he->origin;
            Vertex* v2 = he->destination();

            if (!v1 || !v2)
                continue;

            // 确保每条边只处理一次
            int minId = std::min(v1->id, v2->id);
            int maxId = std::max(v1->id, v2->id);
            std::pair<int, int> edgePair(minId, maxId);

            if (processedEdges.find(edgePair) == processedEdges.end()) {
                processedEdges.insert(edgePair);

                if (isEdgeCollapseValid(he.get())) {
                    float cost = calculateEdgeCollapseCost(he.get());
                    edgeQueue.push(EdgeCollapseCost(he.get(), cost));
                }
            }
        }

        // 执行边坍缩直到达到目标面片数量
        while (faces.size() > targetFaceCount && !edgeQueue.empty()) {
            EdgeCollapseCost current = edgeQueue.top();
            edgeQueue.pop();

            // 执行边坍缩
            Vertex* newVertex = collapseEdge(current.edge);

            if (newVertex) {
                // 更新优先队列中的边
                // 实际实现中，我们应该更新与新顶点相关的所有边的代价
                // 此处简化处理：仅重新计算所有边的代价

                // 清空队列
                while (!edgeQueue.empty()) {
                    edgeQueue.pop();
                }

                // 重新填充队列
                processedEdges.clear();
                for (const auto& he : halfedges) {
                    Vertex* v1 = he->origin;
                    Vertex* v2 = he->destination();

                    if (!v1 || !v2)
                        continue;

                    int minId = std::min(v1->id, v2->id);
                    int maxId = std::max(v1->id, v2->id);
                    std::pair<int, int> edgePair(minId, maxId);

                    if (processedEdges.find(edgePair) == processedEdges.end()) {
                        processedEdges.insert(edgePair);

                        if (isEdgeCollapseValid(he.get())) {
                            float cost = calculateEdgeCollapseCost(he.get());
                            edgeQueue.push(EdgeCollapseCost(he.get(), cost));
                        }
                    }
                }
            }
        }
    }

    // 获取网格信息
    void printMeshInfo() const {
        std::cout << "Mesh Information:" << std::endl;
        std::cout << "Vertices: " << vertices.size() << std::endl;
        std::cout << "Half Edges: " << halfedges.size() << std::endl;
        std::cout << "Faces: " << faces.size() << std::endl;
    }
};

// 示例用法
int main() {
    HalfEdgeMesh mesh;

    // 创建一个简单的网格（例如一个立方体）
    // 顶点
    Vertex* v0 = mesh.addVertex(0, 0, 0);
    Vertex* v1 = mesh.addVertex(1, 0, 0);
    Vertex* v2 = mesh.addVertex(1, 1, 0);
    Vertex* v3 = mesh.addVertex(0, 1, 0);
    Vertex* v4 = mesh.addVertex(0, 0, 1);
    Vertex* v5 = mesh.addVertex(1, 0, 1);
    Vertex* v6 = mesh.addVertex(1, 1, 1);
    Vertex* v7 = mesh.addVertex(0, 1, 1);

    // 创建面片（注意：实际应用中需要更仔细地处理边的连接）
    // 底面（四边形）
    mesh.addFace({v0, v1, v2, v3});
    // 顶面（四边形）
    mesh.addFace({v4, v7, v6, v5});
    // 侧面（四个四边形）
    mesh.addFace({v0, v4, v5, v1});
    mesh.addFace({v1, v5, v6, v2});
    mesh.addFace({v2, v6, v7, v3});
    mesh.addFace({v3, v7, v4, v0});

    std::cout << "Original mesh:" << std::endl;
    mesh.printMeshInfo();

    // 简化网格
    mesh.simplify(4);  // 目标保留4个面片

    std::cout << "Simplified mesh:" << std::endl;
    mesh.printMeshInfo();

    return 0;
}