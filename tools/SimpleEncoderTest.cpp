#include "SimpleEncoder.h"

int main() {
    SimpleEncoder simple("../static/untitled1.off");
    // simple.SimplifyVertexTo(29761);
    simple.SimplifyVertexTo(500);
    std::string path = "./res_final.off";
    simple.dumpTo(path);
    return 0;
}