#include "SimpleDecoder.h"

int main() {
    SimpleDecoder simple("./bunny_try.loc");    
    simple.decode();
    std::string path = "./res_decoder.off";
    simple.dumpTo(path);
    return 0;
}