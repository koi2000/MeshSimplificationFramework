/*
 * @Author: koi
 * @Date: 2025-09-26 10:22:00
 * @Description:
 */
#include "Halfedge.h"
#include "Vertex.h"
#include <fstream>
#include <set>
#include <vector>
#define DEBUG
#ifdef DEBUG

#    include "core.h"

#    pragma ONCE

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

static void dumpTo(std::string path, std::set<MCGAL::Vertex*> vertices, std::set<MCGAL::Facet*> facets) {
    std::ofstream offFile(path);
    offFile << "OFF\n";
    offFile << vertices.size() << " " << facets.size() << " 0\n";
    offFile << "\n";
    int id = 0;
    for (MCGAL::Vertex* v : vertices) {
        offFile << v->x() << " " << v->y() << " " << v->z() << "\n";
        v->setVid(id++);
    }
    offFile << "\n";
    for (MCGAL::Facet* face : facets) {
        offFile << "3 ";
        std::map<int, int> mpcnt;
        for (auto it = face->halfedges_begin(); it != face->halfedges_end(); it++) {
            mpcnt[(*it)->vertex()->groupId()]++;
            offFile << (*it)->vertex()->vid() << " ";
        }

        // if (face->groupId() >= 0) {
        //     offFile << colors[face->groupId()][0] << " " << colors[face->groupId()][1] << " " << colors[face->groupId()][2] << " ";
        // } else {
        //     offFile << 0 << " " << 0 << " " << 0 << " ";
        // }
        if (mpcnt.size() == 1) {
            offFile << colors[mpcnt.begin()->first][0] << " " << colors[mpcnt.begin()->first][1] << " " << colors[mpcnt.begin()->first][2] << " ";
        } else {
            offFile << 0 << " " << 0 << " " << 0 << " ";
        }

        offFile << "\n";
    }
}

static void DEBUG_DUMP_HALFEDGE(MCGAL::Halfedge* h) {
    h->face()->dumpTo("./submesh1/h_" + std::to_string(h->poolId()) + ".off");
}

static void DEBUG_DUMP_FACE(MCGAL::Facet* f) {
    f->dumpTo("./submesh1/f_" + std::to_string(f->poolId()) + ".off");
}

static void DEBUG_DUMP_VERTEX_ONE_RING(MCGAL::Vertex* v, std::string path) {
    std::set<MCGAL::Vertex*> vone_ring;
    std::set<MCGAL::Facet*> fone_ring;
    vone_ring.insert(v);
    for (MCGAL::Halfedge* h : v->halfedges()) {
        vone_ring.insert(h->end_vertex());
        fone_ring.insert(h->face());
    }
    dumpTo(path, vone_ring, fone_ring);
}

static void DEBUG_DUMP_VERTEX_TWO_RING(MCGAL::Vertex* v, std::string path) {
    std::set<MCGAL::Vertex*> vone_ring;
    std::vector<MCGAL::Vertex*> one_ring;
    std::set<MCGAL::Facet*> fone_ring;
    vone_ring.insert(v);
    for (MCGAL::Halfedge* h : v->halfedges()) {
        vone_ring.insert(h->end_vertex());
        fone_ring.insert(h->face());
        one_ring.push_back(h->end_vertex());
    }
    for (MCGAL::Vertex* v : one_ring) {
        for (MCGAL::Halfedge* h : v->halfedges()) {
            vone_ring.insert(h->end_vertex());
            fone_ring.insert(h->face());
        }
    }
    dumpTo(path, vone_ring, fone_ring);
}

static void DEBUG_DUMP_VERTEX_THREE_RING(MCGAL::Vertex* v, std::string path) {
    std::set<MCGAL::Vertex*> vone_ring;
    std::vector<MCGAL::Vertex*> one_ring;
    std::vector<MCGAL::Vertex*> two_ring;
    std::set<MCGAL::Facet*> fone_ring;
    vone_ring.insert(v);
    for (MCGAL::Halfedge* h : v->halfedges()) {
        vone_ring.insert(h->end_vertex());
        fone_ring.insert(h->face());
        one_ring.push_back(h->end_vertex());
    }
    for (MCGAL::Vertex* v : one_ring) {
        for (MCGAL::Halfedge* h : v->halfedges()) {
            vone_ring.insert(h->end_vertex());
            fone_ring.insert(h->face());
            two_ring.push_back(h->end_vertex());
        }
    }
    for (MCGAL::Vertex* v : two_ring) {
        for (MCGAL::Halfedge* h : v->halfedges()) {
            vone_ring.insert(h->end_vertex());
            fone_ring.insert(h->face());
        }
    }
    dumpTo(path, vone_ring, fone_ring);
}

#elif

static void DEBUG_DUMP_HALFEDGE(MCGAL::Halfedge* h) {}

static void DEBUG_DUMP_FACE(MCGAL::Facet* f) {}

static void DEBUG_DUMP_VERTEX_ONE_RING(MCGAL::Vertex* v) {}

static void DEBUG_DUMP_VERTEX_TWO_RING(MCGAL::Vertex* v) {}

static void DEBUG_DUMP_VERTEX_THREE_RING(MCGAL::Vertex* v, std::string path) {}

#endif
