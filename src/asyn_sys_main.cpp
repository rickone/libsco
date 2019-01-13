#include "asyn_override.h"
#include <cstdint>
#include <cstdio>

struct macho_header;
extern "C" uintptr_t start(const struct macho_header* appsMachHeader, int argc, const char* argv[], 
                intptr_t slide, const struct macho_header* dyldsMachHeader,
                uintptr_t* startGlue);

ASYN_OVERRIDE(start)
extern "C" uintptr_t start(const struct macho_header* appsMachHeader, int argc, const char* argv[], 
                intptr_t slide, const struct macho_header* dyldsMachHeader,
                uintptr_t* startGlue) {
    puts("start hooked");
    return ASYN_ORIGIN(start)(appsMachHeader, argc, argv, slide, dyldsMachHeader, startGlue);
}

int main();
ASYN_OVERRIDE(main)
int main() {
    puts("main hooked");
    return ASYN_ORIGIN(main)();
}
