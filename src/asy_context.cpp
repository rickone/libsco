#include "asy_context.h"
#include "pthread.h"

using namespace asy;

static std::once_flag s_init_context_flag;
static pthread_key_t s_context_key;

static void delete_context(void* ctx) {
    delete (context*)ctx;
}

static void init_context_key() {
    pthread_key_create(&s_context_key, delete_context);
}

context* asy::get_context() {
    auto ctx = (context*)pthread_getspecific(s_context_key);
    if (ctx == nullptr) {
        std::call_once(s_init_context_flag, init_context_key);

        ctx = new context();
        pthread_setspecific(s_context_key, ctx);
    }

    return ctx;
}
