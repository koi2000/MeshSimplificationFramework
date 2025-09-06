/*
 * @Author: koi
 * @Date: 2025-09-03 09:42:17
 * @Description:
 */
/*
 * @Author: koi
 * @Date: 2025-08-25 21:24:39
 * @Description:
 */
#include "Options.h"
#include "PMSF.h"
#include "decompress/BasicDeserializeOperator.h"
#include "decompress/BasicSymbolReadOperator.h"
#include "decompress/DeserializeOperator.h"
#include "reconstruct/BasicReconstructOpertator.h"
#include <memory>

int main() {
    PMSF pmsf;
    std::shared_ptr<SymbolReadOperator> symbolReadOperator = std::make_shared<BasicSymbolReadOperator>();
    std::shared_ptr<ReconstructOperator> reconstructOperator = std::make_shared<BasicReconstructOpertator>();
    std::shared_ptr<DeserializeOperator> deserializeOperator = std::make_shared<BasicDeserializeOperator>(symbolReadOperator, reconstructOperator);
    DecompressOptions options = DecompressOptions::Builder()
                                    .setDeserializeOperator(deserializeOperator)
                                    .setPath("./compress.bin")
                                    .build();
    pmsf.decompress(options);
    return 0;
}