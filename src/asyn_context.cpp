#include "asyn_context.h"
#include <cstdlib> // malloc, free
#include <pthread.h>

using namespace asyn;

static pthread_key_t s_context_key;
static pthread_once_t s_context_once;

static void delete_context(void* ctx) {
    free(ctx);
}

static void make_context_key() {
    pthread_key_create(&s_context_key, delete_context);
}

context* asyn::init_context() {
    pthread_once(&s_context_once, make_context_key);

    auto ctx = malloc(sizeof(context));
    pthread_setspecific(s_context_key, ctx);
    return (context*)ctx;
}

context* asyn::get_context() {
    return (context*)pthread_getspecific(s_context_key);
}
