#include "../src/ProgressiveSimplificationFramework.h"
#include "../src/common/Options.h"
#include "../src/EdgeCollapseFunc.h"

int main() {
    ProgressiveSimplificationFramework frameWork;
    frameWork.loadMesh("/home/koi/mastercode/MeshSimplificationFramework/static/untitled1.off");
    // std::unique_ptr<Options> options = Options::createBuilder()
    //                                     .addLevelType(CompressType::GREEDY, LocalOperatorType::VERTEX_REMOVAL)
    //                                     .addLevelType(CompressType::GREEDY, LocalOperatorType::VERTEX_REMOVAL)
    //                                     .addLevelType(CompressType::GREEDY, LocalOperatorType::VERTEX_REMOVAL)
    //                                     .addLevelType(CompressType::GREEDY, LocalOperatorType::VERTEX_REMOVAL)
    //                                     .build();
    
    RegisterPropertiesOperator registerOperator(register_func,init_func);
    std::unique_ptr<Options> options = Options::createBuilder()
                                           // .addLevelType(CompressType::GREEDY, LocalOperatorType::EDGE_COLLAPSE)
                                           .addLevelType(CompressType::FAST_QUADRIC, LocalOperatorType::EDGE_COLLAPSE)
                                           // .addLevelType(CompressType::GREEDY, LocalOperatorType::EDGE_COLLAPSE)
                                           // .addLevelType(CompressType::GREEDY, LocalOperatorType::EDGE_COLLAPSE)
                                           // .addLevelType(CompressType::GREEDY, LocalOperatorType::EDGE_COLLAPSE)
                                           .build();
    frameWork.encode(*options);
    return 0;
}