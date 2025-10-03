/*
 * @Author: koi
 * @Date: 2025-09-23 20:27:26
 * @Description:
 */
#include "BasicBoundaryCompressOperator.h"
#include "Halfedge.h"
#include "MeshUtils.h"
#include "common/DebugTool.h"
#include <iostream>

int count = 0;

bool BasicBoundaryCompressOperator::compress_boundary() {
    std::set<int> groupIds = graph->getAllGroupId();
    count = 0;
    for (auto& groupId : groupIds) {
        compress_group(groupId);
    }
    graph->resetState();
    std::cout << "number of compress: " << count << std::endl;
    return true;
}

bool BasicBoundaryCompressOperator::compress_group(int groupId) {
    // int count = 0;
    std::unordered_map<int, std::vector<Node>>& nodes = graph->getNode(groupId);
    for (auto& [neighbourId, nodeArr] : nodes) {
        for (auto& node : nodeArr) {
            if (node.isVisiable()) {
                continue;
            }
            node.setVisiable();
            int poolId = node.st;
            MCGAL::Vertex* vt = MCGAL::contextPool.getVertexByIndex(mesh->meshId(), poolId);
            MCGAL::Halfedge* boundary = nullptr;
            if(groupId == 33 && neighbourId == 38) {
                DEBUG_DUMP_VERTEX_THREE_RING(vt, "debug-nei.off");
            }
            for (MCGAL::Halfedge* hit : vt->halfedges()) {
                if (hit->isBoundary() && hit->face()->groupId() == groupId) {
                    // if (hit->end_vertex()->poolId() == node.ed) {
                        boundary = hit;
                        break;
                    // }
                }
            }
            std::unordered_map<int, std::vector<Node>>& neighbour_nodes = graph->getNode(neighbourId);
            for (auto& [key, nds] : neighbour_nodes) {
                if (key == groupId) {
                    for (auto& nd : nds) {
                        if (nd.st == node.stop) {
                            nd.setVisiable();
                            break;
                        }
                    }
                }
            }
            int cnt = 0;
            if (groupId == 5 && neighbourId == 21) {
                int i = 0;
            }
            int stopId = node.stop;
            do {
                if (boundary->end_vertex()->poolId() == stopId) {
                    break;
                }
                MCGAL::Halfedge* newBoundary = MCGAL::next_boundary(groupId, neighbourId, boundary);
                if (!newBoundary) {
                    break;
                }
                if (isRemovable(boundary)) {
                    // newBoundary = MCGAL::next_boundary(groupId, neighbourId, newBoundary);
                    // if (!newBoundary) {
                    // break;
                    // }
                    if (groupId == 23 && neighbourId == 46) {
                        DEBUG_DUMP_VERTEX_THREE_RING(boundary->vertex(), "debug-0.off");
                    }
                    newBoundary = eliminateOperator->encode_boundary(boundary);
                    if (groupId == 23 && neighbourId == 46) {
                        DEBUG_DUMP_VERTEX_THREE_RING(newBoundary->vertex(), "debug-1.off");
                    }
                    cnt++;
                    count++;
                    
                }

                //     // MCGAL::Halfedge* next_boundary = MCGAL::next_boundary(neighbourId, boundary);
                //     // boundary->setNotBoundary();
                //     // next_boundary->setNotBoundary();
                //     // // try use split facet to triangulation and use vertex removal to compress
                //     // MCGAL::Vertex* v = boundary->end_vertex();
                //     // // 借入方存在内部
                //     // // next_boundary->next()->setBoundarySave();
                //     // // 借出方存在边界
                //     // // MCGAL::find_prev(boundary)->opposite()->setBoundarySave();
                //     // assert(v->vertex_degree() > 2);
                //     // MCGAL::Halfedge* h = boundary->opposite();
                //     // MCGAL::Halfedge* end(h);
                //     // // 遍历找到两个点之间的距离
                //     // MCGAL::Vertex* v1 = boundary->vertex();
                //     // MCGAL::Vertex* v2 = next_boundary->end_vertex();
                //     // MCGAL::Halfedge* st = boundary->opposite();
                //     // MCGAL::Halfedge* ed = st;
                //     // int opposite_number = 0;
                //     // int number = 0;
                //     // // while (st->end_vertex() != v2) {
                //     // //     opposite_number++;
                //     // //     st = MCGAL::find_prev(st->opposite());
                //     // // }
                //     // number = st->vertex()->halfedges().size() - opposite_number;
                //     // // 尝试将周围的所有大于等于四的面片提前处理
                //     // do {
                //     //     MCGAL::Facet* f = h->face();
                //     //     if (f->facet_degree() > 3) {
                //     //         MCGAL::Halfedge* hSplit(h->next());
                //     //         for (; hSplit->next()->next() != h; hSplit = hSplit->next())
                //     //             ;
                //     //         MCGAL::Halfedge* hCorner = mesh->split_facet_non_meshId(h, hSplit);
                //     //         hCorner->setAdded();
                //     //     }
                //     // } while ((h = h->opposite()->next()) != end);
                //     // MCGAL::Point vPos = boundary->end_vertex()->point();
                //     // MCGAL::Halfedge* hNewFace = mesh->erase_center_vertex(boundary);
                //     // MCGAL::Facet* added_face = hNewFace->face();
                //     // added_face->setGroupId(groupId);
                //     // // added_face->setSplittable();
                //     // // added_face->setRemovedVertexPos(vPos);
                //     // for (auto it = added_face->halfedges_begin(); it != added_face->halfedges_end(); it++) {
                //     //     if ((*it)->face()->groupId() != (*it)->opposite()->face()->groupId()) {
                //     //         (*it)->setBoundary();
                //     //     }
                //     // }
                // }
                if (boundary->end_vertex()->poolId() == stopId) {
                    break;
                }
                boundary = newBoundary;
            } while (boundary->end_vertex()->poolId() != stopId);
        }
    }
    // std::cout << "number of compress: " << count << std::endl;
    return true;
}