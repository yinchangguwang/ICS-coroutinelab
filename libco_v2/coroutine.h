#pragma once

#include <cassert>
#include <cstdlib>

#include "coro_ctx.h"

namespace coro {

struct coroutine;
struct coroutine_attr;

coroutine *create(func_t coro_func, void *arg, const coroutine_attr *attr = nullptr);
void release(coroutine *co);

int resume(coroutine *co, int param = 0);
int yield(int ret = 0);

struct stack_mem {
    int stack_size = 0;  // 栈的大小
    // TODO: add member variables you need
    coroutine* occupy_co = nullptr;
    char* stack_bp; //stack_buffer + stack_size
    char* stack_buffer; // = stack start

    stack_mem(size_t size) : stack_size(size) {
        // TODO: implement your code here
        stack_buffer = new char[size];
        stack_bp = stack_buffer + size;
    }

    ~stack_mem() {
        // TODO: implement your code here
        delete []stack_buffer;
    }
};

struct share_stack {
    // TODO: add member variables you need
    unsigned int alloc_idx = 0;
    int count = 0;
    int stack_size = 0;
    stack_mem **stack_array = nullptr;

    share_stack(int count, size_t stack_size)
        : count(count), stack_size(stack_size) {
        // TODO: implement your code here
        stack_array = new stack_mem*[count];
        for(int i = 0; i < count; i++){
            stack_array[i] = new stack_mem(stack_size);
        }
    }

    ~share_stack() {
        // TODO: implement your code here
        for(int i = 0; i < count; i++){
            delete stack_array[i];
        }
        delete []stack_array;
    }

    stack_mem *get_stackmem() {
        // TODO: implement your code here
        if(!stack_array){
            return nullptr;
        }
        int idx = alloc_idx % count;
        alloc_idx++;
        return stack_array[idx];
        // return nullptr;
    }
};

struct coroutine {
    bool started = false;
    bool end = false;

    func_t coro_func = nullptr;
    void *arg = nullptr;

    // TODO: add member variables you need
    bool isShareStack;

    stack_mem* stackmem;

    char* stack_sp;
    char* save_buffer;
    int data;
    unsigned int stack_size = 8192;

    context ctx;
    coroutine(func_t func, void* args): coro_func(func), arg(args) {
        save_buffer = new char[stack_size];
    }
    ~coroutine() {
        // TODO: implement your code here
        // if(!isShareStack){
        //     free(stackmem->stack_buffer);
        //     free(stackmem);
        // }else{
        //     if(save_buffer){
        //         free(save_buffer);
        //     }
        //     if(stackmem->occupy_co == this){
        //         stackmem->occupy_co = nullptr;
        //     }
        // }
        free(save_buffer);
    }
};

struct coroutine_attr {
    int stack_size = 128 * 1024;
    share_stack *sstack = nullptr;
};

class coroutine_env {
private:
    // TODO: add member variables you need
    coroutine* co_stack[128];
    int size = 0;
public:
    // TODO: add member variables you need
    //for copy stack log lastco and nextco
    coroutine* pending_co;
    coroutine* occupy_co;
public:
    coroutine_env() {
        // TODO: implement your code here
        coroutine* main_coro = new coroutine(nullptr, nullptr);
        push(main_coro);
    }

    coroutine *get_coro(int idx) {
        // TODO: implement your code here
        return co_stack[size + idx];
    }

    void pop() {
        // TODO: implement your code here
        size--;
    }

    void push(coroutine *co) {
        // TODO: implement your code here
        co_stack[size++] = co;
    }
};

}  // namespace coro
