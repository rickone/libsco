#include "asy_override.h"
#include <cstdint>
#include <cstdio>

struct macho_header;
extern "C" uintptr_t start(const struct macho_header* appsMachHeader, int argc, const char* argv[], 
                intptr_t slide, const struct macho_header* dyldsMachHeader,
                uintptr_t* startGlue);

ASY_OVERRIDE(start)
extern "C" uintptr_t start(const struct macho_header* appsMachHeader, int argc, const char* argv[], 
                intptr_t slide, const struct macho_header* dyldsMachHeader,
                uintptr_t* startGlue) {
    puts("start hooked");
    return ASY_ORIGIN(start)(appsMachHeader, argc, argv, slide, dyldsMachHeader, startGlue);
}
