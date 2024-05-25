#pragma once

#include <ucontext.h>

#include <cassert>

namespace coro {

class coroutine;
using func_t = void (*)(void*);

coroutine* create(func_t func, void* args);
void release(coroutine* co);
int resume(coroutine* co, int param = 0);
int yield(int ret = 0);

struct coroutine {
    bool started = false;
    bool end = false;

    func_t coro_func = nullptr;
    void* args = nullptr;

    // TODO: add member variables you need
    int data;
    ucontext_t ctx = {0};
    char* stack;
    static const int stack_size = 8192;

    coroutine(func_t func, void* args) : coro_func(func), args(args) {
        /* TODO */
        stack = new char[stack_size];
    }

    ~coroutine() {
        /* TODO */
        delete []stack;
    }
};

class coroutine_env {
private:
    // TODO: add member variables you need
    coroutine* co_stack[3];
    int size = 0;

public:
    coroutine_env() {
        // TODO: implement your code here
        coroutine* main_coro = create(nullptr, nullptr);
        push(main_coro);
    }

    coroutine* get_coro(int idx) {
        // TODO: implement your code here
        return co_stack[size + idx];
    }

    void push(coroutine* co) {
        // TODO: implement your code here
        co_stack[size++] = co;
    }

    void pop() {
        // TODO: implement your code here
        size--;
        // 写这个也行 co_stack[size--] = nullptr;
    }
};

}  // namespace coro