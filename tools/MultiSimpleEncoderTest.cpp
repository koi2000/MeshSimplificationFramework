#include "MultiSimpleEncoder.h"

int main() {
    MultiSimpleEncoder simple("../static/untitled1.off");
    // simple.SimplifyVertexTo(29761);
    simple.encode_group(0, 500);
    std::string path = "./res_final.off";
    simple.dumpGroupTo(path, 0);
    return 0;
}