#include "coro.h"

namespace coro {

static coroutine_env g_coro_env;

coroutine* create(func_t func, void* args) {
    // TODO: implement your code here
    coroutine* co = new coroutine(func, args);
    return co;
}

void release(coroutine* co) {
    // TODO: implement your code here
    if(g_coro_env.get_coro(-1) == co){
        g_coro_env.pop();
    }else{
        coroutine* curr = g_coro_env.get_coro(-1);
        g_coro_env.pop();
        if(g_coro_env.get_coro(-1) == co){
            g_coro_env.pop();
        }
        g_coro_env.push(curr);
    }
    delete co;
}

static void func_wrap(coroutine* co) {
    if (co->coro_func) {
        co->coro_func(co->args);
    }
    co->end = true;
    yield(-1);
}

int resume(coroutine* co, int param) {
    // TODO: implement your code here
    coroutine* curr = g_coro_env.get_coro(-1);
    if(!co->started){
        getcontext(&co->ctx);
        co->ctx.uc_stack.ss_sp = co->stack;
        co->ctx.uc_stack.ss_size = coroutine::stack_size;
        co->ctx.uc_link = &curr->ctx;
        makecontext(&co->ctx, (void(*)(void))func_wrap, 1, co);

        co->started = true;
    }
    g_coro_env.push(co);
    co->data = param;
    swapcontext(&curr->ctx, &co->ctx);
    return curr->data;
}

int yield(int ret) {
    // TODO: implement your code here
    coroutine* curr = g_coro_env.get_coro(-1);
    g_coro_env.pop();
    coroutine* next = g_coro_env.get_coro(-1);
    next->data = ret;
    swapcontext(&curr->ctx, &next->ctx);
    return curr->data;
}

}  // namespace coro