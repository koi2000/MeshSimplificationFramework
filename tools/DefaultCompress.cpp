/*
 * @Author: koi
 * @Date: 2025-08-25 21:24:39
 * @Description:
 */
#include "Options.h"
#include "PMSF.h"
#include "elimination/EliminateOperator.h"
#include "elimination/VertexRemovalEliminateOperator.h"
#include "operator/IsRemovableOperator.h"
#include "operator/ManifoldIsRemovableOperator.h"
#include "operator/UnifiedManifoldIsRemovableOperator.h"
#include "options/SelectOptions.h"
#include "selection/ErrorSource.h"
#include "selection/PriorityErrorSelector.h"
#include "selection/SelectOperator.h"
#include "operator/BasicIsRemovableOperator.h"
#include <memory>

int main() {
    PMSF pmsf;
    std::shared_ptr<IsRemovableOperator> brmop = std::make_shared<BasicIsRemovableOperator>();
    std::shared_ptr<IsRemovableOperator> mrmop = std::make_shared<UnifiedManifoldIsRemovableOperator>();
    std::shared_ptr<SelectOptions> soptions =
        SelectOptions::Builder()
            .withErrorSource(ErrorSource::Halfedge)
            .withHalfedgeErrorAccessor(nullptr)
            .withVertexErrorAccessor(nullptr)
            .withIsRemovableOperator(brmop)
            .withIsRemovableOperator(mrmop)
            .build();

    std::shared_ptr<SelectOperator> selectOp = std::make_shared<PriorityErrorSelector>(*soptions);
    std::shared_ptr<EliminateOperator> eliminateOp = std::make_shared<VertexRemovalEliminateOperator>();

    CompressOptions coptions = CompressOptions::Builder()
                                   .setSelect(selectOp)
                                   .setRound(15)
                                   .setEliminate(eliminateOp)
                                   .setEnableCompress(true)
                                   .setEnablePrediction(true)
                                   .setEnableQuantization(true)
                                   .setPath("/home/koi/mastercode/MeshSimplificationFramework/static/untitled.off")
                                   .setOutputPath("./compress_1.bin")
                                   .build();
    pmsf.compress(coptions);
    return 0;
}