#include <algorithm>
#include <cassert>
#include <iostream>
#include <queue>
#include <random>
#include <vector>

#include "coro.h"

// =========== global variables ===========
std::random_device rd;
std::mt19937 gen(rd());
// =========== global variables ===========

// =========== test-1 ===========
std::queue<int> queue;

static void foo_1([[maybe_unused]] void*) {
    for (int i = 0; i < 10; i++) {
        assert(queue.empty() == true);
        queue.push(i);
        coro::yield();
    }
}

static void bar_1([[maybe_unused]] void*) {
    for (int i = 0; i < 10; i++) {
        assert(queue.empty() == false);
        assert(queue.front() == i);
        queue.pop();
        coro::yield();
    }
}

static void test_1() {
    coro::coroutine* co1 = coro::create(foo_1, nullptr);
    coro::coroutine* co2 = coro::create(bar_1, nullptr);

    while (!co1->end || !co2->end) {
        if (!co1->end) {
            coro::resume(co1);
        }
        if (!co2->end) {
            coro::resume(co2);
        }
    }

    coro::release(co1);
    coro::release(co2);

    std::cout << "test-1 passed" << std::endl;
}
// =========== test-1 ===========

// =========== test-2 ===========
std::vector<int> vec;

struct args_2 {
    int x;
};

void foo_2(void* args) {
    int x = ((args_2*)args)->x;
    for (int i = 0; i < x; i++)
        coro::yield();
    vec.push_back(x);
}

static void test_2() {
    std::uniform_int_distribution<> dis_1_10(1, 10);
    std::uniform_int_distribution<> dis_1_100(1, 100);
    int num = dis_1_10(gen);

    std::vector<coro::coroutine*> coros;
    std::vector<args_2*> args;
    std::vector<int> ans;

    for (int i = 0; i < num; i++) {
        args_2* arg = new args_2{dis_1_100(gen)};
        coros.push_back(coro::create(foo_2, arg));
        args.push_back(arg);
        ans.push_back(arg->x);
    }

    while (coros.size() > 0) {
        for (size_t i = 0; i < coros.size(); i++) {
            if (coros[i]->end) {
                coro::release(coros[i]);
                delete args[i];
                coros.erase(coros.begin() + i);
                args.erase(args.begin() + i);
                i--;
            } else {
                coro::resume(coros[i]);
            }
        }
    }

    std::sort(ans.begin(), ans.end());

    for (size_t i = 0; i < ans.size(); i++) {
        assert(ans[i] == vec[i]);
    }

    std::cout << "test-2 passed" << std::endl;
}

// =========== test-2 ===========

// =========== test-3 ===========
void foo_3([[maybe_unused]] void*) {
    static const char str[] = "test-3 passed\n";
    for (const auto& c : str) {
        assert(coro::yield(c) == 233);
    }
}

void test_3() {
    auto gen = coro::create(foo_3, nullptr);
    while (1) {
        const char c = coro::resume(gen, 233);
        if (c == -1) break;
        std::cout << c;
    }
    coro::release(gen);
}
// =========== test-3 ===========

// =========== cat ===========
const char str[128] = {"Congratulations! You have passed all the tests of libco-v1!\n"};

struct arg_n {
    int start;
    int step;
};

static void foo_n(void* args) {
    arg_n* arg = (arg_n*)args;
    for (int i = arg->start; str[i] != '\0'; i += arg->step) {
        std::cout << str[i];
        coro::yield();
    }
}

static void cat() {
    std::uniform_int_distribution<> dis(1, 10);
    int n = dis(gen);

    std::vector<coro::coroutine*> coros;
    std::vector<arg_n*> args;

    for (int i = 0; i < n; i++) {
        arg_n* a = new arg_n{i, n};
        args.push_back(a);
        coros.push_back(coro::create(foo_n, a));
    }

    while (coros.size() > 0) {
        for (size_t i = 0; i < coros.size(); i++) {
            if (coros[i]->end) {
                coro::release(coros[i]);
                delete args[i];
                coros.erase(coros.begin() + i);
                args.erase(args.begin() + i);
                i--;
            } else {
                coro::resume(coros[i]);
            }
        }
    }
    assert(coros.empty());
    assert(args.empty());
}
// =========== cat ===========

int main() {
    test_1();
    test_2();
    test_3();
    cat();
}
