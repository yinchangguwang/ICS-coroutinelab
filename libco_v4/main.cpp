#include <iostream>
#include <ranges>

#include "generator.h"

int main() {
    auto coro = []() -> coro::generator<const char> {
        static const char str[] = "Hello, ICS 2023!\n";
        for (const auto& c : str) {
            co_yield c;
        }
    }();

    for (const auto& c : coro) {
        std::cout << c;
    }

    auto gen = []() -> coro::generator<int> {
        int a = 0, b = 1;
        while (true) {
            co_yield a;
            std::tie(a, b) = std::make_pair(b, a + b);
        }
    }();

    for (const auto& n : gen | std::views::drop(3) | std::views::take(10)) {
        std::cout << n << ' ';
    }
    std::cout << '\n';
}
