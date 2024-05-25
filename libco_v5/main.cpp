#include <cassert>
#include <iostream>
#include <ranges>

#include "generator.h"
#include "sleep.h"

coro::generator<int> range(int start, int end) {
    co_yield start;
    if (start + 1 < end) {
        co_yield range(start + 1, end);
    }
}

int main() {
    auto coro = []() -> coro::generator<const char> {
        static const char str[] = "Start libco_v5 test\n";
        for (const auto& c : str) {
            co_yield c;
        }
    }();

    for (const auto& c : coro) {
        std::cout << c;
    }

    auto gen = range(0, 100);

    int ans = 0;
    for (const auto& i : gen | std::views::filter([](int i) { return i % 2 == 0; }) | std::views::take(10)) {
        assert(i == ans);
        ans += 2;
    }
    assert(ans == 20);

    auto task = []() -> coro::Task {
        static const char str[] = "lbov akts asd\n";
        for (const auto& c : str) {
            co_await coro::sleep{300};
            std::cout << c << std::flush;
        }
    }();
    // libco_v5 task test passed!\n
    // l b o v   a k t s   a s d \n
    //  i c _ 5 t s   e t p s e !
    auto task2 = []() -> coro::Task {
        static const char str[] = "ic_5ts etpse!";
        for (const auto& c : str) {
            co_await coro::sleep{305};
            std::cout << c << std::flush;
        }
    }();

    coro::wait_task_queue_empty();
}
