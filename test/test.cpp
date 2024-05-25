#include <ucontext.h>
#include <stdio.h>
#include <string.h>

int main() {
    ucontext_t ctx;
    int m = 0;
    printf("%d", m);
    getcontext(&ctx);
    m++;
    printf("%d", m);
    std::cout << "Hello, ICS 2023!" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    setcontext(&ctx);

}