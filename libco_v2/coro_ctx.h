#pragma once

#include <stdlib.h>

namespace coro {

using func_t = void (*)(void *);

struct context {
    // TODO: add member variables you need
    void *regs[ 14 ];
    size_t ss_size;
	char *ss_sp;
};

void ctx_make(context *ctx, func_t coro_func, const void *arg);

}  // namespace coro
