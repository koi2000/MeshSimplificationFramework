#include "Mesh.h"
#include "Facet.h"
#include "Global.h"
#include "Halfedge.h"
#include "Vertex.h"
#include "VertexSplitNode.h"
#include "biops.h"
#include "string.h"
#include <array>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <sstream>
namespace MCGAL {

#define COLOR

constexpr std::array<std::array<int, 3>, 68> colors = {{
    // {0, 0, 0},        // Black
    {255, 0, 0},      // Red
    {0, 255, 0},      // Green
    {0, 0, 255},      // Blue
    {255, 255, 0},    // Yellow
    {255, 0, 255},    // Magenta
    {0, 255, 255},    // Cyan
    {128, 128, 128},  // Gray
    {255, 165, 0},    // Orange
    {128, 0, 128},    // Purple
    {64, 224, 208},   // Turquoise
    {255, 250, 205},  // Lemon Yellow
    {128, 0, 0},      // Maroon
    {127, 255, 212},  // Aquamarine
    {75, 0, 130},     // Indigo
    {255, 160, 122},  // Light Salmon
    {32, 178, 170},   // Light Sea Green
    {135, 206, 250},  // Light Sky Blue
    {119, 136, 153},  // Light Slate Gray
    {176, 196, 222},  // Light Steel Blue
    {255, 255, 224},  // Light Yellow
    {250, 128, 114},  // Salmon
    {106, 90, 205},   // Slate Blue
    {112, 128, 144},  // Slate Gray
    {0, 0, 128},      // Navy
    {189, 183, 107},  // Dark Khaki
    {153, 50, 204},   // Dark Orchid
    {205, 133, 63},   // Peru
    {128, 128, 0},    // Olive
    {160, 82, 45},    // Sienna
    {102, 205, 170},  // Medium Aquamarine
    {50, 205, 50},    // Lime Green
    {70, 130, 180},   // Steel Blue
    {210, 105, 30},   // Chocolate
    {154, 205, 50},   // Yellow Green
    {219, 112, 147},  // Pale Violet Red
    {173, 255, 47},   // Green Yellow
    {240, 255, 255},  // Azure
    {255, 127, 80},   // Coral
    {238, 130, 238},  // Violet
    {75, 0, 130},     // Indigo
    {255, 105, 180},  // Hot Pink
    {0, 250, 154},    // Medium Spring Green
    {0, 191, 255},    // Deep Sky Blue
    {255, 99, 71},    // Tomato
    {245, 222, 179},  // Wheat
    {255, 228, 196},  // Bisque
    {255, 215, 0},    // Gold
    {139, 69, 19},    // Saddle Brown
    {255, 228, 181},  // Moccasin
    {152, 251, 152},  // Pale Green
    {238, 232, 170},  // Pale Goldenrod
    {255, 140, 0},    // Dark Orange
    {255, 140, 105},  // Light Salmon
    {0, 128, 128},    // Teal
    {255, 228, 225},  // Blanched Almond
    {192, 192, 192},  // Silver
    {0, 0, 139},      // Dark Blue
    {173, 216, 230},  // Light Blue
    {0, 255, 255},    // Aqua
    {139, 0, 139},    // Dark Magenta
    {255, 250, 250},  // Snow
    {245, 245, 220},  // Beige
    {0, 255, 127},    // Spring Green
    {255, 105, 180},  // Hot Pink
    {255, 255, 255}   // White
}};

Mesh::~Mesh() {}

Facet* Mesh::add_face(std::vector<Vertex*>& vs) {
    Facet* f = nullptr;
    f = new Facet(vs);

    f = MCGAL::contextPool.allocateFaceFromPool(DEFAULT_MESH_ID, vs);
    faces_.push_back(f);
    return f;
}

Facet* Mesh::add_face(Facet* face) {
    faces_.push_back(face);
    return face;
}

Vertex* Mesh::add_vertex(Vertex* vertex) {
    vertices_.push_back(vertex);
    return vertex;
}

void Mesh::eraseFacetByPointer(Facet* facet) {
    for (auto it = faces_.begin(); it != faces_.end();) {
        if ((*it) == facet) {
            faces_.erase(it);
            break;
        } else {
            it++;
        }
    }
}

void Mesh::eraseVertexByPointer(Vertex* vertex) {
    for (auto it = vertices_.begin(); it != vertices_.end();) {
        if ((*it) == vertex) {
            vertices_.erase(it);
            break;
        } else {
            it++;
        }
    }
}

void Mesh::resetState() {
    for (int i = 0; i < vertices_.size(); i++) {
        vertices_[i]->resetState();
    }
}

// 计算边坍缩代价（可以使用各种度量标准）
float calculateEdgeCollapseCost(Halfedge* edge) {
    // 这里使用简单的边长作为代价
    return edge->length();
}

// 需要确定方向
MCGAL::Vertex* Mesh::halfedge_collapse(MCGAL::Halfedge* h, MCGAL::Point newp) {
    static int order = 0;
    MCGAL::Vertex* v1 = h->vertex();
    MCGAL::Vertex* v2 = h->end_vertex();
    MCGAL::Point c = v1->point();
    MCGAL::Point d = v2->point();
    std::set<MCGAL::Vertex*> belongv2;
    for (MCGAL::Halfedge* hit : v2->halfedges()) {
        belongv2.insert(hit->end_vertex());
    }

#ifdef DEBUG
    if (v1->isRemoved() || v2->isRemoved()) {
        std::cout << "error" << std::endl;
    }
#endif
    v1->setPoint(newp);

    MCGAL::Vertex* banned1 = nullptr;
    MCGAL::Vertex* banned2 = nullptr;

    v2->setRemoved();

    if (h->face()->facet_degree() == 3) {
        Halfedge* h1 = h->next();
        banned1 = h1->end_vertex();
        Halfedge* h2 = find_prev(h);
        MCGAL::Halfedge* oh1 = h1->opposite();
        MCGAL::Halfedge* oh2 = h2->opposite();
        oh1->setOpposite(oh2);
        oh2->setOpposite(oh1);

        // banned1 = h->next()->end_vertex();

        h->face()->setRemoved();
        MCGAL::Halfedge* st = h1->face()->proxyHalfedge();
        MCGAL::Halfedge* ed = h1->face()->proxyHalfedge();
        do {
            st->vertex()->eraseHalfedgeByPointer(st);
            if (st->vertex() == v2) {
                st->opposite()->setEndVertex(v1);
            }
            st->setRemoved();
            st = st->next();
        } while (st != ed);

    } else if (h->face()->facet_degree() > 3) {
        Halfedge* h1 = h->next();
        Halfedge* h2 = find_prev(h);
        h2->setNext(h1);
        h2->face()->reset(h2);
    }
    // 获得one ring
    MCGAL::Halfedge* around = h->next()->opposite()->next()->next();

    h = h->opposite();

    if (h->face()->facet_degree() == 3) {
        Halfedge* h1 = h->next();
        banned2 = h1->end_vertex();
        Halfedge* h2 = find_prev(h);
        MCGAL::Halfedge* oh1 = h1->opposite();
        MCGAL::Halfedge* oh2 = h2->opposite();
        oh1->setOpposite(oh2);
        oh2->setOpposite(oh1);

        // banned2 = h->next()->end_vertex();

        h->face()->setRemoved();
        MCGAL::Halfedge* st = h1->face()->proxyHalfedge();
        MCGAL::Halfedge* ed = h1->face()->proxyHalfedge();
        do {
            st->vertex()->eraseHalfedgeByPointer(st);
            if (st->vertex() == v2) {
                st->opposite()->setEndVertex(v1);
            }
            st->setRemoved();
            st = st->next();
        } while (st != ed);

    } else if (h->face()->facet_degree() > 3) {
        Halfedge* h1 = h->next();
        Halfedge* h2 = find_prev(h);
        h2->setNext(h1);
        h2->face()->reset(h2);
    }

    for (MCGAL::Halfedge* hit : v2->halfedges()) {
        // hit->end_vertex()->eraseHalfedgeByPointer(hit->opposite());
        hit->setVertex(v1);
        hit->opposite()->setEndVertex(v1);
        // if (hit != h->opposite() && hit != h->next()) {
        v1->halfedges().push_back(hit);
        // }
    }
    MCGAL::Halfedge* st = around;
    MCGAL::Halfedge* ed = around;
    MCGAL::Halfedge* minEdge = st;
    do {
        if (st->vertex()->point() < minEdge->vertex()->point()) {
            minEdge = st;
        }
        st = st->next()->opposite()->next();
    } while (st != ed);
    uint16_t bitmap = 0;
    // memset(bitmap, 0, 2);
    st = minEdge;
    ed = st;
    int idx = 0;
    int bannedIdx1 = -1;
    int bannedIdx2 = -1;
    int flag = 0;
    // 需要一个人bit来存储先banned1还是banned2
    // 记录一下是从1->2还是从2->1
    do {
        if (st->vertex() == banned1) {
            bannedIdx1 = idx;
            setBit(reinterpret_cast<char*>(&bitmap), idx);
        } else if (st->vertex() == banned2) {
            bannedIdx2 = idx;
            setBit(reinterpret_cast<char*>(&bitmap), idx);
        }
        if (bannedIdx1 != -1 && bannedIdx2 != -1) {
            break;
        }
        // 说明是从1->2
        if (flag == 0 && bannedIdx1 != -1) {
            if (belongv2.count(st->end_vertex())) {
                flag = 1;
            } else {
                flag = 3;
            }
        }
        // 说明是从2->1
        if (flag == 0 && bannedIdx2 != -1) {
            if (belongv2.count(st->end_vertex())) {
                flag = 2;
            } else {
                flag = 3;
            }
        }
        idx++;
        st = st->next()->opposite()->next();
    } while (st != ed);
    // 说明是从小到大
    if (flag == 1 && bannedIdx1 < bannedIdx2) {
        // if(banned1.next()->vertex() == banned2)
        setBit(reinterpret_cast<char*>(&bitmap), 15);
    }
    if (flag == 2 && bannedIdx2 < bannedIdx1) {
        setBit(reinterpret_cast<char*>(&bitmap), 15);
    }

    // 需要把自己的孩子也都记下来
    VertexSplitNode* vsn = new VertexSplitNode();
    vsn->order = order++;
    // vsn->c = c;
    // vsn->d = d;
    vsn->c = c;
    vsn->d = d;
    if (v1->isCollapsed()) {
        // setBit(bitmap, 13);
        vsn->left = v1->vsplitNode();
    }
    if (v2->isCollapsed()) {
        // setBit(bitmap, 14);
        vsn->right = v2->vsplitNode();
    }
    v1->setCollapsed();
    vsn->bitmap = bitmap;
    v1->setVertexSplitNode(vsn);
    if(v2->isBoundary()) {
        v1->setBoundary();
    }
    return v1;
}

MCGAL::Halfedge* Mesh::vertex_split(MCGAL::Vertex* v, MCGAL::VertexSplitNode* node) {
    // 获取到one ring的halfedge
    MCGAL::Halfedge* st = v->halfedges()[0]->next();
    MCGAL::Halfedge* ed = st;
    MCGAL::Halfedge* minEdge = st;
    do {
        if (st->vertex()->point() < minEdge->vertex()->point()) {
            minEdge = st;
        }
        st = st->next()->opposite()->next();
    } while (st != ed);
    char* bitmap = reinterpret_cast<char*>(&node->bitmap);
    bool ordered = getBit(bitmap, 15);
    bool isFirst = true;
    st = minEdge;
    ed = st;
    int idx = 0;
    MCGAL::Halfedge* banned1 = nullptr;
    MCGAL::Halfedge* banned2 = nullptr;
    // 需要一个bit来存储先banned1还是banned2
    do {
        if (getBit(bitmap, idx)) {
            if (isFirst && ordered) {
                banned1 = st;
                isFirst = false;
            } else if (!isFirst && ordered) {
                banned2 = st;
            }
            if (isFirst && !ordered) {
                banned2 = st;
                isFirst = false;
            } else if (!isFirst && !ordered) {
                banned1 = st;
            }
            // if (isBanned1) {
            //     banned1 = st;
            //     isBanned1 = false;
            // } else {
            //     banned2 = st;
            //     isBanned1 = true;
            // }
        }
        idx++;
        st = st->next()->opposite()->next();
    } while (st != ed);
    if (v->poolId() == 8869 || banned1->vertex()->poolId() == 8869 || banned2->vertex()->poolId() == 8869) {
        int i = 0;
    }
    // MCGAL::Halfedge* op = banned1->next()->next()->opposite();
    // op->setOpposite(nullptr);
    // banned1->next()->next()->setOpposite(nullptr);
    // op = banned2->next()->next()->opposite();
    // op->setOpposite(nullptr);
    // banned2->next()->next()->setOpposite(nullptr);

    MCGAL::Point c = node->c;
    MCGAL::Point d = node->d;
    // MCGAL::Point v1p = c - d / 2;
    // MCGAL::Point v2p = c + d / 2;
    MCGAL::Point v1p = c;
    MCGAL::Point v2p = d;

    v->setPoint(v1p);
    MCGAL::Vertex* v2 = MCGAL::contextPool.allocateVertexFromPool(meshId_, v2p);
    // if (node->order == 4333) {
    //     std::swap(banned1, banned2);
    // }

    st = banned1;
    ed = banned2;

    do {
        MCGAL::Halfedge* hit1 = st->next()->next();
        MCGAL::Halfedge* hit2 = st->next()->next()->opposite();
        MCGAL::Halfedge* next = st->next()->opposite()->next();
        if (st->vertex() == banned1->vertex() || st->vertex() == banned2->vertex()) {
            MCGAL::Halfedge* op = hit1->opposite();
            op->setOpposite(nullptr);
            hit1->setOpposite(nullptr);
        }
        // if (next == ed) {
        //     st->next()->setEndVertex(v2);
        //     st = next;
        //     break;
        // }
        hit1->vertex()->eraseHalfedgeByPointer(hit1);
        hit1->setVertex(v2);
        v2->halfedges().push_back(hit1);
        if (st->vertex() != banned1->vertex()) {
            hit2->setEndVertex(v2);
        }
        st = next;
    } while (st != ed);
    st->next()->next()->opposite()->setEndVertex(v2);
    if (st->vertex() == banned2->vertex()) {
        MCGAL::Halfedge* op = st->next()->next()->opposite();
        op->setOpposite(nullptr);
        st->next()->next()->setOpposite(nullptr);
    }
    // for (MCGAL::Halfedge* hit : v->halfedges()) {
    //     if (hit->end_vertex() == banned1->vertex() || hit->end_vertex() == banned2->vertex()) {
    //         MCGAL::Halfedge* op = hit->opposite();
    //         op->setOpposite(nullptr);
    //         hit->setOpposite(nullptr);
    //     }
    // }
    MCGAL::Facet* fit1 = MCGAL::contextPool.allocateFaceFromPool(meshId_, {banned1->vertex(), v2, v});
    MCGAL::Facet* fit2 = MCGAL::contextPool.allocateFaceFromPool(meshId_, {banned2->vertex(), v, v2});

    if (node->left) {
        v->setVertexSplitNode(node->left);
    } else {
        v->setVertexSplitNode(nullptr);
    }
    if (node->right) {
        v2->setVertexSplitNode(node->right);
    }
    vertices_.push_back(v2);
    faces_.push_back(fit1);
    faces_.push_back(fit2);
    MCGAL::Halfedge* res = nullptr;
    for (MCGAL::Halfedge* hit : v->halfedges()) {
        if (hit->end_vertex() == v2) {
            return hit;
        }
    }

    return nullptr;
}

bool Mesh::IsFlipped(MCGAL::Halfedge* edge, const MCGAL::Point& ptTarget) {
    MCGAL::Vertex* v0 = edge->vertex();
    MCGAL::Vertex* v1 = edge->end_vertex();
    // // 检查坍缩后是否会导致重叠面片或非流形网格
    // // 坍缩后确保其他点的邻居数不能少于三个

    for (MCGAL::Halfedge* h : v0->halfedges()) {
        if (h->face()->isRemoved()) {
            continue;
        }
        std::vector<MCGAL::Vertex*> vs;
        MCGAL::Halfedge* st = h->face()->proxyHalfedge();
        MCGAL::Halfedge* ed = st;
        do {
            vs.push_back(st->vertex());
            st = st->next();
        } while (st != ed);
        if (vs[0] == v1 || vs[1] == v1 || vs[2] == v1) {
            continue;
        }
        int idxV0 = 0;
        for (int i = 0; i < vs.size(); i++) {
            MCGAL::Vertex* v = vs[i];
            if (vs[i] == v0) {
                idxV0 = i;
            }
        }
        MCGAL::Point pt0 = v0->point();
        MCGAL::Point pt1 = vs[(idxV0 + 1) % 3]->point();
        MCGAL::Point pt2 = vs[(idxV0 + 2) % 3]->point();

        MCGAL::Point dir1 = pt1 - ptTarget;
        dir1.normalize();
        MCGAL::Point dir2 = pt2 - ptTarget;
        dir2.normalize();

        // The angle below can be adjusted, but now is enough
        // if the angle between dir1 and dir2 small than 2.6 angle, return true
        if (fabs(dir1.dot(dir2)) > 0.999)
            return true;
        MCGAL::Point normold;
        try {
            normold = h->face()->computeNormal();
        } catch (std::string exp) { return true; }
        MCGAL::Point norm = dir1.cross(dir2);
        norm.normalize();

        // if the angle between normold and norm large than 78.5 angle, return true
        if ((normold.dot(norm)) < 0.2)
            return true;
    }
    return false;
}

bool Mesh::is_collapse_ok(MCGAL::Halfedge* v0v1) {
    // 检查边是否已删除
    if (v0v1->isRemoved()) {
        return false;
    }

    MCGAL::Halfedge* v1v0 = v0v1->opposite();
    MCGAL::Vertex* v0 = v1v0->vertex();
    MCGAL::Vertex* v1 = v0v1->vertex();
    if (v1->halfedges().size() + v0->halfedges().size() >= 16) {
        return false;
    }

    bool v0v1_triangle = false;
    bool v1v0_triangle = false;

    // 检查是否为三角形面
    if (!v0v1->isBoundary()) {
        v0v1_triangle = v0v1->face()->facet_degree() == 3;
    }
    if (!v1v0->isBoundary()) {
        v1v0_triangle = v1v0->face()->facet_degree() == 3;
    }

    // 获取下一个顶点
    MCGAL::Vertex* v_01_n = v0v1->next()->end_vertex();
    MCGAL::Vertex* v_10_n = v1v0->next()->end_vertex();
    if (v_01_n->halfedges().size() <= 3 || v_10_n->halfedges().size() <= 3) {
        return false;
    }

    // 检查顶点是否已删除
    if (v0->isRemoved() || v1->isRemoved()) {
        return false;
    }

    // 检查三角形面的边界情况
    MCGAL::Vertex* vl = nullptr;
    if (!v0v1->isBoundary()) {
        if (v0v1_triangle) {
            MCGAL::Halfedge* h1 = v0v1->next();
            MCGAL::Halfedge* h2 = h1->next();

            vl = h1->vertex();
            if (h1->opposite()->isBoundary() && h2->opposite()->isBoundary()) {
                return false;
            }
        }
    }

    MCGAL::Vertex* vr = nullptr;
    if (!v1v0->isBoundary()) {
        if (v1v0_triangle) {
            MCGAL::Halfedge* h1 = v1v0->next();
            MCGAL::Halfedge* h2 = h1->next();

            vr = h1->vertex();
            if (h1->opposite()->isBoundary() && h2->opposite()->isBoundary()) {
                return false;
            }
        }
    }

    // 检查 vl 和 vr 是否相同
    if (vl && vr && vl == vr) {
        return false;
    }

    // 检查边界顶点
    // if (v0->isBoundary() && v1->isBoundary() && !v0v1->isBoundary() && !v1v0->isBoundary()) {
    //     return false;
    // }

    // 检查一环邻域交集
    std::set<MCGAL::Vertex*> v0_neighbors;
    std::set<MCGAL::Vertex*> v1_neighbors;

    // 收集 v0 的邻居
    for (MCGAL::Halfedge* hit : v0->halfedges()) {
        if (!hit->end_vertex()->isRemoved() && hit->face() != v0v1->face() && hit->face() != v1v0->face()) {
            v0_neighbors.insert(hit->end_vertex());
        }
    }

    // 收集 v1 的邻居
    for (MCGAL::Halfedge* hit : v1->halfedges()) {
        if (!hit->end_vertex()->isRemoved() && hit->face() != v0v1->face() && hit->face() != v1v0->face()) {
            v1_neighbors.insert(hit->end_vertex());
        }
    }

    // 检查交集
    std::set<MCGAL::Vertex*> cneighbor;
    for (MCGAL::Vertex* neighbor : v0_neighbors) {
        if (v1_neighbors.count(neighbor) > 0) {
            // 排除特殊三角形情况
            if (!(neighbor == v_01_n && v0v1_triangle) && !(neighbor == v_10_n && v1v0_triangle)) {
                return false;
            }
            cneighbor.insert(neighbor);
        }
    }
    MCGAL::Vertex* cv1 = v0v1->next()->end_vertex();
    MCGAL::Vertex* cv2 = v0v1->opposite()->next()->end_vertex();
    for (MCGAL::Halfedge* hit : cv1->halfedges()) {
        if (hit->end_vertex() == cv2) {
            return false;
        }
    }

    // for (MCGAL::Vertex* neighbor : cneighbor) {
    //     for (MCGAL::Halfedge* hit : neighbor->halfedges()) {
    //         if (cneighbor.count(hit->end_vertex())) {
    //             return false;
    //         }
    //     }
    // }

    // 检查三角形面的退化情况
    if (v0v1_triangle) {
        MCGAL::Halfedge* one = v0v1->next();
        MCGAL::Halfedge* two = one->next();

        one = one->opposite();
        two = two->opposite();

        if (one->face() == two->face() && one->face()->facet_degree() != 3) {
            return false;
        }
    }

    if (v1v0_triangle) {
        MCGAL::Halfedge* one = v1v0->next();
        MCGAL::Halfedge* two = one->next();

        one = one->opposite();
        two = two->opposite();

        if (one->face() == two->face() && one->face()->facet_degree() != 3) {
            return false;
        }
    }

    // 额外的特殊情况检查
    if (v_01_n == v_10_n && v0v1_triangle && v1v0_triangle) {
        return false;
    }

    // 通过所有检查
    return true;
}

MCGAL::Vertex* Mesh::halfedge_collapse(MCGAL::Halfedge* h) {
    MCGAL::Vertex* v1 = h->vertex();
    MCGAL::Vertex* v2 = h->end_vertex();
    v1->setPoint({(v1->point().x() + v2->point().x()) / 2, (v1->point().y() + v2->point().y()) / 2, (v1->point().z() + v2->point().z()) / 2});
    v1->setProcessed();
    std::set<Vertex*> vset;

    // for (MCGAL::Halfedge* hit : v1->halfedges()) {
    //     hit->face()->reset_without_init(hit);
    // }

    v2->setRemoved();
    // v2->halfedges().clear();

    if (h->face()->facet_degree() == 3) {
        Halfedge* h1 = h->next();
        Halfedge* h2 = find_prev(h);
        MCGAL::Halfedge* oh1 = h1->opposite();
        MCGAL::Halfedge* oh2 = h2->opposite();
        oh1->setOpposite(oh2);
        oh2->setOpposite(oh1);

        h->face()->setRemoved();
        MCGAL::Halfedge* st = h1->face()->proxyHalfedge();
        MCGAL::Halfedge* ed = h1->face()->proxyHalfedge();
        do {
            st->vertex()->eraseHalfedgeByPointer(st);
            st->setRemoved();
            st = st->next();
        } while (st != ed);

    } else if (h->face()->facet_degree() > 3) {
        Halfedge* h1 = h->next();
        Halfedge* h2 = find_prev(h);
        h2->setNext(h1);
        h2->face()->reset(h2);
    }
    h = h->opposite();

    if (h->face()->facet_degree() == 3) {
        Halfedge* h1 = h->next();
        Halfedge* h2 = find_prev(h);
        MCGAL::Halfedge* oh1 = h1->opposite();
        MCGAL::Halfedge* oh2 = h2->opposite();
        oh1->setOpposite(oh2);
        oh2->setOpposite(oh1);

        h->face()->setRemoved();
        MCGAL::Halfedge* st = h1->face()->proxyHalfedge();
        MCGAL::Halfedge* ed = h1->face()->proxyHalfedge();
        do {
            st->vertex()->eraseHalfedgeByPointer(st);
            st->setRemoved();
            st = st->next();
        } while (st != ed);

    } else if (h->face()->facet_degree() > 3) {
        Halfedge* h1 = h->next();
        Halfedge* h2 = find_prev(h);
        h2->setNext(h1);
        h2->face()->reset(h2);
    }

    for (MCGAL::Halfedge* hit : v2->halfedges()) {
        // hit->end_vertex()->eraseHalfedgeByPointer(hit->opposite());
        hit->setVertex(v1);
        hit->opposite()->setEndVertex(v1);
        if (hit != h->opposite() && hit != h->next()) {
            v1->halfedges().push_back(hit);
        }
    }
    return v1;
}

/**
 * 半边折叠的逆操作
 * @param v 需要被split的点
 * @param p 需要被新插入的边
 * @param conn 需要被连接到新点的边
 * @param vid1 vid2
 */
// MCGAL::Halfedge* Mesh::vertex_split(MCGAL::Vertex* v, MCGAL::Point p, std::vector<Halfedge*> conn, int vid1, int vid2) {
//     // TODO: 完善该方法
//     MCGAL::Vertex* vnew = MCGAL::contextPool.tryAllocVertexFromPool(p);
//     // MCGAL::Halfedge* newh = MCGAL::contextPool.allocateHalfedgeFromPool(v, vnew);
//     MCGAL::Halfedge* split1 = nullptr;
//     MCGAL::Halfedge* split2 = nullptr;
//     std::set<int> poolIds;
//     // 某个点，谁两边的边 groupId不一样谁就被分割
//     // conn里只处理groupId属于该半边的
//     /**
//      * 四种case，每种都要单独处理
//      * 最好是分的一人一边，然后直接创建新的face
//      */
//     std::vector<MCGAL::Vertex*> vts;
//     for (MCGAL::Halfedge* hit : conn) {
//         // 分割边被分割后仍属于同一个group
//         // 需要维护opposite
//         if (hit->end_vertex->id == vid1 && hit->opposite != nullptr) {
//             hit->vertex->eraseHalfedgeByPointer(hit);
//             hit->vertex = vnew;
//             vnew->halfedges.push_back(hit);
//             poolIds.insert(hit->end_vertex->poolId);
//             vts.push_back(hit->end_vertex);
//             vts.push_back(vnew);
//             vts.push_back(v);
//         }
//         // 边界情况
//         if (hit->end_vertex->id == vid1 && hit->opposite == nullptr) {
//             hit->vertex = vnew;
//             poolIds.insert(hit->end_vertex->poolId);
//         }
//         // 该group只有end_vertex指向自己的边
//         if (hit->vertex->id == vid1) {
//             vnew->halfedges.push_back(hit);
//             vts.push_back(vnew);
//             vts.push_back(hit->vertex);
//             vts.push_back(v);
//         }
//         if (hit->vertex == v) {
//             hit->vertex = vnew;
//         }
//         if (hit->opposite != nullptr && hit->opposite->face->groupId == hit->face->groupId) {
//             hit->opposite->end_vertex = vnew;
//         }
//     }
//     // 删除v中不属于他的边
//     for (auto it = v->halfedges.begin(); it != v->halfedges.end();) {
//         if (poolIds.count((*it)->poolId)) {
//             vnew->halfedges.push_back(*it);
//             it = v->halfedges.erase(it);
//         } else {
//             it++;
//         }
//     }
//     return nullptr;
// }

Halfedge* Mesh::split_facet_non_meshId(Halfedge* h, Halfedge* g) {
    Facet* origin = h->face();
    // early expose
    Facet* fnew = MCGAL::contextPool.allocateFaceFromPool(meshId_);
    // create new halfedge
    Halfedge* hnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, h->end_vertex(), g->end_vertex());
    Halfedge* oppo_hnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, g->end_vertex(), h->end_vertex());
    // set the opposite
    // set the connect information

    hnew->setNext(g->next());
    oppo_hnew->setNext(h->next());

    h->setNext(hnew);
    g->setNext(oppo_hnew);
    // create new face depend on vertexs
    origin->reset(hnew);
    fnew->reset(oppo_hnew);
    fnew->setProcessedFlag(origin->processedFlag());
    fnew->setGroupId(origin->groupId());
    // fnew->removedFlag = origin->removedFlag;
    // add halfedge and face to mesh
    this->faces_.push_back(fnew);
    return hnew;
}

Halfedge* Mesh::split_facet(Halfedge* h, Halfedge* g) {
    Facet* origin = h->face();
    // early expose
    Facet* fnew = MCGAL::contextPool.allocateFaceFromPool(meshId_);
    // create new halfedge
    Halfedge* hnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, h->end_vertex(), g->end_vertex());
    Halfedge* oppo_hnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, g->end_vertex(), h->end_vertex());
    // set the opposite
    // set the connect information

    hnew->setNext(g->next());
    oppo_hnew->setNext(h->next());

    h->setNext(hnew);
    g->setNext(oppo_hnew);
    // create new face depend on vertexs
    origin->reset(hnew);
    fnew->reset(oppo_hnew);
    fnew->setProcessedFlag(origin->processedFlag());
    fnew->setGroupId(origin->groupId());
    // fnew->removedFlag = origin->removedFlag;
    // add halfedge and face to mesh
    this->faces_.push_back(fnew);
    return hnew;
}

Halfedge* Mesh::erase_center_vertex(Halfedge* h) {
    Halfedge* g = h->next()->opposite();
    Halfedge* hret = find_prev(h);
    while (g != h) {
        Halfedge* gprev = find_prev(g);
        remove_tip(gprev);
        if (g->face() != h->face()) {
            // eraseFacetByPointer(g->facet());
            g->face()->setRemoved();
        }
        Halfedge* gnext = g->next()->opposite();
        g->vertex()->eraseHalfedgeByPointer(g);
        // g->end_vertex->eraseHalfedgeByPointer(g->opposite);
        g->setRemoved();
        g->opposite()->setRemoved();
        g = gnext;
    }
    h->setRemoved();
    h->opposite()->setRemoved();
    remove_tip(hret);
    h->vertex()->eraseHalfedgeByPointer(h);
    h->end_vertex()->halfedges().clear();
    h->end_vertex()->setRemoved();
    // eraseVertexByPointer(h->end_vertex);
    h->face()->reset(hret);
    return hret;
}

Halfedge* Mesh::create_center_vertex(Halfedge* h) {
    // Vertex* vnew = new Vertex();
    Vertex* vnew = MCGAL::contextPool.allocateVertexFromPool(meshId_);
    this->vertices_.push_back(vnew);
    Halfedge* hnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, h->end_vertex(), vnew);
    Halfedge* oppo_new = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, vnew, h->end_vertex());
    // add new halfedge to current mesh and set opposite
    // set the next element
    // now the next of hnew and prev of oppo_new is unknowen
    insert_tip(hnew->opposite(), h);
    Halfedge* g = hnew->opposite()->next();
    std::vector<Halfedge*> origin_around_halfedge;

    Halfedge* hed = hnew;
    while (g->next() != hed) {
        Halfedge* gnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, g->end_vertex(), vnew);
        Halfedge* oppo_gnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, vnew, g->end_vertex());
        origin_around_halfedge.push_back(g);
        gnew->setNext(hnew->opposite());
        insert_tip(gnew->opposite(), g);

        g = gnew->opposite()->next();
        hnew = gnew;
    }
    hed->setNext(hnew->opposite());
    h->face()->reset(h);
    // collect all the halfedge
    for (Halfedge* hit : origin_around_halfedge) {
        Facet* face = MCGAL::contextPool.allocateFaceFromPool(meshId_, hit);
        this->faces_.push_back(face);
    }
    return oppo_new;
}

Halfedge* Mesh::create_center_vertex(Halfedge* h, Point point) {
    // Vertex* vnew = new Vertex();
    Vertex* vnew = MCGAL::contextPool.allocateVertexFromPool(meshId_, point);
    this->vertices_.push_back(vnew);
    Halfedge* hnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, h->end_vertex(), vnew);
    Halfedge* oppo_new = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, vnew, h->end_vertex());
    // add new halfedge to current mesh and set opposite
    // set the next element
    // now the next of hnew and prev of oppo_new is unknowen
    insert_tip(hnew->opposite(), h);
    Halfedge* g = hnew->opposite()->next();
    std::vector<Halfedge*> origin_around_halfedge;
    origin_around_halfedge.reserve(5);

    Halfedge* hed = hnew;
    while (g->next() != hed) {
        Halfedge* gnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, g->end_vertex(), vnew);
        Halfedge* oppo_gnew = MCGAL::contextPool.allocateHalfedgeFromPool(meshId_, vnew, g->end_vertex());
        origin_around_halfedge.push_back(g);
        gnew->setNext(hnew->opposite());
        insert_tip(gnew->opposite(), g);

        g = gnew->opposite()->next();
        hnew = gnew;
    }
    hed->setNext(hnew->opposite());
    h->face()->reset_without_init(h);
    // collect all the halfedge
    for (Halfedge* hit : origin_around_halfedge) {
        Facet* face = MCGAL::contextPool.allocateFaceFromPool(meshId_, hit);
        this->faces_.push_back(face);
    }
    return oppo_new;
}

Halfedge* Mesh::create_center_vertex_without_init(Halfedge* h, Point point) {
    // Vertex* vnew = new Vertex();
    Vertex* vnew = MCGAL::contextPool.allocateVertexFromPool(meshId_, point);
    // this->vertices_.push_back(vnew);
    Halfedge* hnew = MCGAL::contextPool.allocateHalfedgeFromPoolWithOutInit(meshId_, h->end_vertex(), vnew);
    Halfedge* oppo_new = MCGAL::contextPool.allocateHalfedgeFromPoolWithOutInit(meshId_, vnew, h->end_vertex());
    hnew->setOpposite(oppo_new);
    oppo_new->setOpposite(hnew);
    // add new halfedge to current mesh and set opposite
    // set the next element
    // now the next of hnew and prev of oppo_new is unknowen
    insert_tip(hnew->opposite(), h);
    Halfedge* g = hnew->opposite()->next();
    // std::vector<Halfedge*> origin_around_halfedge;
    // origin_around_halfedge.reserve(5);

    Halfedge* hed = hnew;
    while (g->next() != hed) {
        Halfedge* gnew = MCGAL::contextPool.allocateHalfedgeFromPoolWithOutInit(meshId_, g->end_vertex(), vnew);
        Halfedge* oppo_gnew = MCGAL::contextPool.allocateHalfedgeFromPoolWithOutInit(meshId_, vnew, g->end_vertex());

        gnew->setOpposite(oppo_gnew);
        oppo_gnew->setOpposite(gnew);

        gnew->setNext(hnew->opposite());
        insert_tip(gnew->opposite(), g);
        Facet* face = MCGAL::contextPool.allocateFaceFromPool(meshId_, g);
        // origin_around_halfedge.push_back(g);
        // this->faces_.push_back(face);
        g = gnew->opposite()->next();
        hnew = gnew;
    }
    hed->setNext(hnew->opposite());
    h->face()->reset_without_init(h);
    // collect all the halfedge
    // for (Halfedge* hit : origin_around_halfedge) {
    //     Facet* face = MCGAL::contextPool.allocateFaceFromPool(hit, meshId);
    //     this->faces_.push_back(face);
    // }
    return oppo_new;
}
// #endif

inline void Mesh::close_tip(Halfedge* h, Vertex* v) const {
    h->setNext(h->opposite());
    h->setVertex(v);
}

inline void Mesh::insert_tip(Halfedge* h, Halfedge* v) const {
    h->setNext(v->next());
    v->setNext(h->opposite());
}

Halfedge* Mesh::find_prev(Halfedge* h) const {
    Halfedge* g = h;
    while (g->next() != h) {
        g = g->next();
    }
    return g;
}

inline void Mesh::remove_tip(Halfedge* h) const {
    h->setNext(h->next()->opposite()->next());
}

Halfedge* Mesh::join_face(Halfedge* h) {
    Halfedge* hprev = find_prev(h);
    Halfedge* gprev = find_prev(h->opposite());
    remove_tip(hprev);
    remove_tip(gprev);
    h->opposite()->setRemoved();
    h->setRemoved();

    gprev->face()->setRemoved();
    hprev->face()->reset_without_init(hprev);
    return hprev;
}

bool Mesh::loadOFF(std::string path) {
    std::ifstream fp(path);
    if (!fp.is_open()) {
        std::cerr << "Error: Unable to open file " << path << std::endl;
        return false;
    }

    std::stringstream file;
    file << fp.rdbuf();  // Read the entire file content into a stringstream

    std::string format;

    file >> format >> nb_vertices >> nb_faces >> nb_edges;

    meshId_ = MCGAL::contextPool.registerPool(nb_vertices, nb_faces * 3, nb_faces);

    if (format != "OFF") {
        std::cerr << "Error: Invalid OFF file format" << std::endl;
        return false;
    }

    // std::vector<Vertex*> vertices_;
    for (int i = 0; i < nb_vertices; ++i) {
        float x, y, z;
        file >> x >> y >> z;
        Vertex* vt = MCGAL::contextPool.allocateVertexFromPool(DEFAULT_MESH_ID, x, y, z);
        // Vertex* vt = new Vertex(x, y, z);
        this->vertices_.push_back(vt);
    }

    for (int i = 0; i < nb_faces; ++i) {
        int num_face_vertices;
        file >> num_face_vertices;
        std::vector<Vertex*> vts;
        for (int j = 0; j < num_face_vertices; ++j) {
            int vertex_index;
            file >> vertex_index;
            vts.push_back(vertices_[vertex_index]);
        }
#ifdef LOAD_COLOR
        int r, g, b;
        file >> r >> g >> b;
#endif
        MCGAL::Facet* fit = MCGAL::contextPool.allocateFaceFromPool(DEFAULT_MESH_ID, vts);
        this->faces_.push_back(fit);
    }
    // vertices_.clear();
    fp.close();
    return true;
}

inline bool isFacetRemovable(MCGAL::Facet* fit) {
    return fit->isRemoved();
}

inline bool isVertexRemovable(MCGAL::Vertex* vit) {
    return vit->isRemoved();
}

MCGAL::Vector3 Mesh::computeNormal() {
    MCGAL::Vector3 normal(0, 0, 0);
    int len = faces_.size();
    for (int i = 0; i < len; i++) {
        normal = normal + faces_[i]->computeNormal();
    }
    return MCGAL::Vector3(normal.x() / len, normal.y() / len, normal.z() / len);
}

void Mesh::driftAlongNormal(int step) {
    MCGAL::Vector3 normal = computeNormal();
    for (int i = 0; i < vertices_.size(); i++) {
        vertices_[i]->setPoint({vertices_[i]->x() + step * normal.x(), vertices_[i]->y() + step * normal.y(), vertices_[i]->z() + step * normal.z()});
    }
}

#ifdef DECODE
void Mesh::dumpto(std::string path) {
    auto newEnd = std::remove_if(faces_.begin(), faces_.end(), isFacetRemovable);
    faces_.resize(std::distance(faces_.begin(), newEnd));

    auto newVEnd = std::remove_if(vertices_.begin(), vertices_.end(), isVertexRemovable);
    vertices_.resize(std::distance(vertices_.begin(), newVEnd));

    std::ofstream offFile(path);
    if (!offFile.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return;
    }
    // write header
    offFile << "OFF\n";
    offFile << this->vertices_.size() << " " << this->faces_.size() << " 0\n";
    offFile << "\n";
    // write vertex
    int id = 0;
    for (Vertex* vertex : this->vertices_) {
        if (vertex->isRemoved())
            continue;
        offFile << vertex->x() << " " << vertex->y() << " " << vertex->z() << "\n";
        vertex->setVid(id++);
    }

    for (Facet* face : this->faces_) {
        if (face->isRemoved())
            continue;

        offFile << face->vertices_.size() << " ";
        Halfedge* hst = face->halfedges[0];
        Halfedge* hed = hst;
        do {
            offFile << hst->vertex->getVid() << " ";
            hst = hst->next;
        } while (hst != hed);

#    ifdef COLOR
        if (face->groupId >= 0) {
            offFile << colors[face->groupId][0] << " " << colors[face->groupId][1] << " " << colors[face->groupId][2] << " ";
        } else {
            offFile << 0 << " " << 0 << " " << 0 << " ";
        }
#    endif
        offFile << "\n";
    }
    offFile.close();
}
#else

void Mesh::garbage_collection() {
    auto newEnd = std::remove_if(faces_.begin(), faces_.end(), isFacetRemovable);
    faces_.resize(std::distance(faces_.begin(), newEnd));

    auto newVEnd = std::remove_if(vertices_.begin(), vertices_.end(), isVertexRemovable);
    vertices_.resize(std::distance(vertices_.begin(), newVEnd));
}

void Mesh::submesh_dumpto_oldtype(std::string path, int groupId) {
    auto newEnd = std::remove_if(faces_.begin(), faces_.end(), isFacetRemovable);
    faces_.resize(std::distance(faces_.begin(), newEnd));

    auto newVEnd = std::remove_if(vertices_.begin(), vertices_.end(), isVertexRemovable);
    vertices_.resize(std::distance(vertices_.begin(), newVEnd));

    std::set<Vertex*> vertices;
    std::vector<Facet*> faces;
    // std::copy_if(std::begin(vertices_), std::end(vertices_), std::back_inserter(vertices),
    //              [&](Vertex* v) { return v->groupId() == groupId; });
    std::copy_if(std::begin(faces_), std::end(faces_), std::back_inserter(faces), [&](Facet* f) { return f->groupId() == groupId; });
    for (Facet* f : faces) {
        for (Facet::halfedge_iterator it = f->halfedges_begin(); it != f->halfedges_end(); ++it) {
            vertices.insert((*it)->vertex());
        }
    }

    std::ofstream offFile(path);
    if (!offFile.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return;
    }
    // write header
    offFile << "OFF\n";
    offFile << vertices.size() << " " << faces.size() << " 0\n";
    offFile << "\n";
    // write vertex
    int id = 0;
    for (Vertex* vertex : vertices) {
        if (vertex->isRemoved())
            continue;
        offFile << vertex->x() << " " << vertex->y() << " " << vertex->z() << "\n";
        vertex->setVid(id++);
    }

    for (Facet* face : faces) {
        if (face->isRemoved())
            continue;

        int num = 0;
        // offFile << face->vertices_.size() << " ";
        Halfedge* hst = face->proxyHalfedge();
        Halfedge* hed = face->proxyHalfedge();
        bool fg = false;
        do {
            // offFile << hst->vertex->getVid() << " ";
            num++;
            if (hst->vertex()->isRemoved()) {
                fg = true;
            }
            hst = hst->next();
        } while (hst != hed);
        if (fg) {
            offFile << 3 << " ";
            offFile << 0 << " " << 0 << " " << 0 << " ";
            offFile << "\n";
            continue;
        }
        offFile << num << " ";
        do {
            offFile << hst->vertex()->vid() << " ";
            hst = hst->next();
        } while (hst != hed);

#    ifdef COLOR
        if (face->groupId() >= 0) {
            offFile << colors[face->groupId()][0] << " " << colors[face->groupId()][1] << " " << colors[face->groupId()][2] << " ";
        } else {
            offFile << 0 << " " << 0 << " " << 0 << " ";
        }
#    endif
        offFile << "\n";
    }
    offFile.close();
}

void Mesh::dumpto_oldtype(std::string path) {
    auto newEnd = std::remove_if(faces_.begin(), faces_.end(), isFacetRemovable);
    faces_.resize(std::distance(faces_.begin(), newEnd));

    auto newVEnd = std::remove_if(vertices_.begin(), vertices_.end(), isVertexRemovable);
    vertices_.resize(std::distance(vertices_.begin(), newVEnd));

    std::ofstream offFile(path);
    if (!offFile.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return;
    }
    // write header
    offFile << "OFF\n";
    offFile << this->vertices_.size() << " " << this->faces_.size() << " 0\n";
    offFile << "\n";
    // write vertex
    int id = 0;
    for (Vertex* vertex : this->vertices_) {
        if (vertex->isRemoved())
            continue;
        offFile << vertex->x() << " " << vertex->y() << " " << vertex->z() << "\n";
        vertex->setVid(id++);
    }

    for (Facet* face : this->faces_) {
        if (face->isRemoved())
            continue;

        int num = 0;
        // offFile << face->vertices_.size() << " ";
        Halfedge* hst = face->proxyHalfedge();
        Halfedge* hed = face->proxyHalfedge();
        bool fg = false;
        do {
            // offFile << hst->vertex->getVid() << " ";
            num++;
            if (hst->vertex()->isRemoved()) {
                fg = true;
            }
            hst = hst->next();
        } while (hst != hed);
        if (fg) {
            offFile << 3 << " ";
            offFile << 0 << " " << 0 << " " << 0 << " ";
            offFile << "\n";
            continue;
        }
        offFile << num << " ";
        do {
            offFile << hst->vertex()->vid() << " ";
            hst = hst->next();
        } while (hst != hed);

#    ifdef COLOR
        if (face->groupId() >= 0) {
            offFile << colors[face->groupId()][0] << " " << colors[face->groupId()][1] << " " << colors[face->groupId()][2] << " ";
        } else {
            offFile << 0 << " " << 0 << " " << 0 << " ";
        }
#    endif
        offFile << "\n";
    }
    offFile.close();
}

// void Mesh::dumpto(std::string path) {
//     int subVPoolSize = MCGAL::contextPool.getSubVPoolSize(meshId);
//     int subFPoolSize = MCGAL::contextPool.getSubFPoolSize(meshId);
//     // 重置未移除的面和其半边的状态
//     // auto newEnd = std::remove_if(faces_.begin(), faces_.end(), isFacetRemovable);
//     // faces_.resize(std::distance(faces_.begin(), newEnd));
//     // auto newVEnd = std::remove_if(vertices_.begin(), vertices_.end(), isVertexRemovable);
//     // vertices_.resize(std::distance(vertices_.begin(), newVEnd));
//     std::ofstream offFile(path);
//     if (!offFile.is_open()) {
//         std::cerr << "Error opening file: " << path << std::endl;
//         return;
//     }
//     // 计算实际的有效顶点数量
//     int validVCount = 0;
//     for (int i = 0; i < subVPoolSize; i++) {
//         MCGAL::Vertex* vertex = MCGAL::contextPool.getVertexByIndexInSubPool(meshId, i);
//         if (!vertex->isRemoved()) {
//             validVCount++;
//         }
//     }
//     // 计算实际的有效面数量
//     int validFCount = 0;
//     for (int i = 0; i < subFPoolSize; i++) {
//         MCGAL::Facet* face = MCGAL::contextPool.getFacetByIndexInSubPool(meshId, i);
//         if (!face->isRemoved()) {
//             validFCount++;
//         }
//     }
//     // write header
//     offFile << "OFF\n";
//     offFile << validVCount << " " << validFCount << " 0\n";
//     offFile << "\n";
//     // write vertex
//     int id = 0;
//     for (int i = 0; i < subVPoolSize; i++) {
//         MCGAL::Vertex* vertex = MCGAL::contextPool.getVertexByIndexInSubPool(meshId, i);
//         if (vertex->isRemoved()) {
//             continue;
//         }
//         offFile << vertex->x() << " " << vertex->y() << " " << vertex->z() << "\n";
//         vertex->setVid(id++);
//     }
//     for (int i = 0; i < subFPoolSize; i++) {
//         MCGAL::Facet* face = MCGAL::contextPool.getFacetByIndexInSubPool(meshId, i);
//         if (face->isRemoved())
//             continue;
//         int num = 0;
//         // offFile << face->vertices_.size() << " ";
//         Halfedge* hst = face->proxyHalfedge;
//         Halfedge* hed = face->proxyHalfedge;
//         do {
//             // offFile << hst->vertex->getVid() << " ";
//             num++;
//             hst = hst->next;
//         } while (hst != hed);
//         offFile << num << " ";
//         do {
//             offFile << hst->vertex->getVid() << " ";
//             hst = hst->next;
//         } while (hst != hed);
// #    ifdef COLOR
//         if (face->groupId >= 0) {
//             offFile << colors[face->groupId][0] << " " << colors[face->groupId][1] << " " << colors[face->groupId][2] << " ";
//         } else {
//             offFile << 0 << " " << 0 << " " << 0 << " ";
//         }
// #    endif
//         // else if(face->groupId == -1) {
//         //     std::cout << 1 << std::endl;
//         //     offFile << 255 << " " << 255 << " " << 255 << " ";
//         // }
//         // if (face->groupId >= 0 && face->groupId < 10) {
//         //     offFile << colors[face->level][0] << " " << colors[face->level][1] << " " << colors[face->level][2] << " ";
//         // } else {
//         //     offFile << 255 << " " << 255 << " " << 255 << " ";
//         // }
//         // if (face->level <10) {
//         //     offFile << colors[face->level][0] << " " << colors[face->level][1] << " " << colors[face->level][2] << " ";
//         // } else {
//         //     offFile << 255 << " " << 255 << " " << 255 << " ";
//         // }
//         offFile << "\n";
//     }
//     offFile.close();
// }

#endif

}  // namespace MCGAL