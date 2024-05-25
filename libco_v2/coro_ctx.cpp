#include <cstdint>
#include <cstring>

#include "coro_ctx.h"

#define ESP 0
#define EIP 1
#define EAX 2
#define ECX 3
// -----------
#define RSP 0
#define RIP 1
#define RBX 2
#define RDI 3
#define RSI 4

#define RBP 5
#define R12 6
#define R13 7
#define R14 8
#define R15 9
#define RDX 10
#define RCX 11
#define R8 12
#define R9 13

enum {
    kRDI = 7,
    kRSI = 8,
    kRETAddr = 9,
    kRSP = 13,
};

namespace coro {

void ctx_make(context* ctx, func_t coro_func, const void* arg) {
    // TODO: implement your code here
    char* sp = ctx->ss_sp + ctx->ss_size - sizeof(void*);
    sp = (char*)((unsigned long)sp & -16LL);

    memset(ctx->regs, 0, sizeof(ctx->regs));
    void** ret_addr = (void**)(sp);
    *ret_addr = (void*)coro_func;

    ctx->regs[kRSP] = sp;

    ctx->regs[kRETAddr] = (char*)coro_func;

    ctx->regs[kRDI] = (char*)arg;
}

}  // namespace coro
