#include "asyn.h"
#include "hiredis.h"
#include <cstdio>

int main() {
    const char* host = "127.0.0.1";
    int port = 6379;
    redisContext *c = redisConnect(host, port);
    if (c == NULL || c->err) {
        if (c) {
            printf("Error: %s\n", c->errstr);
            // handle error
        } else {
            printf("Can't allocate redis context\n");
        }
    }

    printf("redis connected to %s:%d\n", host, port);

    redisReply *reply = (redisReply*)redisCommand(c, "PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    redisFree(c);
    return 0;
}
