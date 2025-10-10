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
 #include "decompress/SegmentationDeserializeOperator.h"
 #include "decompress/SegmentationSymbolReadOperator.h"
 #include "decompress/DeserializeOperator.h"
 #include "reconstruct/BasicReconstructOpertator.h"
 #include <memory>
 
 int main() {
     PMSF pmsf;
     std::shared_ptr<SymbolReadOperator> symbolReadOperator = std::make_shared<SegmentationSymbolReadOperator>();
     std::shared_ptr<ReconstructOperator> reconstructOperator = std::make_shared<BasicReconstructOpertator>();
     std::shared_ptr<DeserializeOperator> deserializeOperator = std::make_shared<SegmentationDeserializeOperator>(symbolReadOperator, reconstructOperator);
     DecompressOptions options = DecompressOptions::Builder()
                                     .setDeserializeOperator(deserializeOperator)
                                     .setPath("./compress_1.bin")
                                     .build();
     pmsf.segmentDecompress(options);
     return 0;
 }