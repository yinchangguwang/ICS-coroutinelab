#include <cassert>
#include <iostream>

#define CO_BEGIN            switch(state) { case 0: 

#define CO_END              } return -1;

#define CO_YIELD(value)     \
    do {                    \
        state = __LINE__;   \
        return value;       \
        case __LINE__:;     \
    } while (0)

#define CO_RETURN(value)      do { state = -1; return value; } while (0)

struct coroutine_base {
    int state;
    coroutine_base():state(0){}
};

class fib : public coroutine_base {
private:
    int a, b;

public:
    // TODO: update below code when you implement
    // CO_BEGIN/CO_END/CO_YIELD/CO_RETURN
    int operator()() {
        CO_BEGIN
        a = 0, b = 1;
        while (true) {
            CO_YIELD(a);
            int tmp = a;
            a = b;
            b = tmp + b;
        }
        CO_RETURN(a);
        CO_END
    }
};

int main() {
    int ans[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    fib foo;
    for (int i = 0; i < 10; i++)
        assert(foo() == ans[i]);
    std::cout << "libco_v3 test passed!" << std::endl;
}
