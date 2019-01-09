#pragma once

#include <dlfcn.h>

#define ASY_OVERRIDE(func) decltype(func)* org_##func = nullptr;
#define ASY_ORIGIN(func) (org_##func ? org_##func : (/*puts("dlsym -> "#func), */org_##func = (decltype(func)*)dlsym(RTLD_NEXT, #func)))
#define ASY_ORIGIN_DEF(func) extern decltype(func)* org_##func
