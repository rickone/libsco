#pragma once

#include <dlfcn.h>

#define ASYN_OVERRIDE(func) decltype(func)* org_##func = nullptr;
#define ASYN_ORIGIN(func) (org_##func ? org_##func : (/*puts("dlsym -> "#func), */org_##func = (decltype(func)*)dlsym(RTLD_NEXT, #func)))
#define ASYN_ORIGIN_DEF(func) extern decltype(func)* org_##func
