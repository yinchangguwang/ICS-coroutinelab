all: main

main: coro_ctx_swap.S coro_ctx.cpp coroutine.cpp main.cpp
	g++-11 -g -std=c++20 -o main coro_ctx_swap.S coro_ctx.cpp coroutine.cpp main.cpp

clean:
	rm main

.PHONY: all clean
