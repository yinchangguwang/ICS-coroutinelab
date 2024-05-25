#include <cstring>

#include "coroutine.h"

namespace coro {

static coroutine_env g_coro_env;

extern "C" {
extern void coro_ctx_swap(context*, context*) asm("coro_ctx_swap");
};

coroutine* create(func_t coro_func, void* arg, const coroutine_attr* attr) {
    coroutine_attr at;
    if (attr != nullptr) {
        at = *attr;
    }

    // TODO: implement your code here
    // coroutine* co = new coroutine(coro_func, arg);
    // return co;

    //控制栈的大小
    if(at.stack_size <= 0) { at.stack_size = 8 * 1024; }
    else if(at.stack_size > 1024 * 128){
        at.stack_size = 1024 * 128;
    }
    //保证是4k的倍数
    if(at.stack_size & 0xFFF){
		at.stack_size &= ~0xFFF;
		at.stack_size += 0x1000;
	}
    coroutine *lp = new coroutine(coro_func, arg);
    // memset(lp, 0, (long)(sizeof(coroutine)));

    stack_mem* stackmem = nullptr;
    if(at.sstack){
        stackmem = at.sstack->get_stackmem();
        at.stack_size = at.sstack->stack_size;
    }else{
        stackmem = new stack_mem(at.stack_size);
    }
    lp->stackmem = stackmem;
    lp->ctx.ss_sp = stackmem->stack_buffer;
	lp->ctx.ss_size = at.stack_size;

    lp->isShareStack = (at.sstack != nullptr);
    lp->save_buffer = nullptr;
    lp->stack_size = 0;

    return lp;
}

void release(coroutine* co) {
    // TODO: implement your code here
    if(!co->isShareStack){
        free(co->stackmem->stack_buffer);
        free(co->stackmem);
    }else{
        if(co->save_buffer){
            free(co->save_buffer);
        }
        if(co->stackmem->occupy_co == co){
            co->stackmem->occupy_co = nullptr;
        }
    }
    free(co);
    // delete co;
}

void save_stack(coroutine* co) {
    // TODO: implement your code here
    // memcpy(co->save_buffer, co->ctx.ss_sp, co->stack_size);
    ///copy out
	stack_mem* stackmem = co->stackmem;
	int len = stackmem->stack_bp - co->stack_sp;

	if (co->save_buffer){
		free(co->save_buffer), co->save_buffer = nullptr;
	}

	co->save_buffer = new char[len]; //malloc buf;
	co->stack_size = len;

	memcpy(co->save_buffer, co->stack_sp, len);
}

void swap(coroutine* curr, coroutine* pending) {
    // TODO: implement your code here
    //get curr stack sp
    char c;
    curr->stack_sp = &c;
    if(!pending->isShareStack){
        g_coro_env.pending_co = nullptr;
        g_coro_env.occupy_co = nullptr;
    }else{
        g_coro_env.pending_co = pending;
        //get last occupy co on the same stack mem
		coroutine* occupy_co = pending->stackmem->occupy_co;
		//set pending co to occupy thest stack mem;
		pending->stackmem->occupy_co = pending;

		g_coro_env.occupy_co = occupy_co;
		if (occupy_co && (occupy_co != pending)){
			save_stack(occupy_co);
		}
    }
    //swap context
    coro_ctx_swap(&curr->ctx, &pending->ctx);
    if (g_coro_env.occupy_co && g_coro_env.pending_co && g_coro_env.occupy_co != g_coro_env.pending_co){
		//resume stack buffer
		if (g_coro_env.pending_co->save_buffer && g_coro_env.pending_co->stack_size > 0){
			memcpy(g_coro_env.pending_co->stack_sp, g_coro_env.pending_co->save_buffer, g_coro_env.pending_co->stack_size);
		}
	}
}

static void func_wrap(coroutine* co) {
    if (co->coro_func) {
        co->coro_func(co->arg);
    }
    co->end = true;
    yield(-1);
}

int resume(coroutine* co, int param) {
    // TODO: implement your code here
    coroutine* curr = g_coro_env.get_coro(-1);
    if(!co->started){
        // co->ctx.ss_sp = co->save_buffer;
        // co->ctx.ss_size = co->stack_size;
        ctx_make(&co->ctx, (void(*)(void*))func_wrap, co);

        co->started = true;
    }
    g_coro_env.push(co);
    co->data = param;
    swap(curr, co);
    return curr->data;
}

int yield(int ret) {
    // TODO: implement your code here
    coroutine* curr = g_coro_env.get_coro(-1);
    coroutine* caller = g_coro_env.get_coro(-2);
    g_coro_env.pop();
    caller->data = ret;
    swap(curr, caller);
    return curr->data;
}

}  // namespace coro
