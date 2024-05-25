#include <coroutine>
#include <functional>
#include <queue>
#include <thread>

namespace coro {

// 静态任务队列，用于存储协程任务
static std::queue<std::function<bool()>> task_queue;
// sleep 结构体，用于协程中的休眠操作
struct sleep {
    sleep(int n_ms) : delay{n_ms} {}

    std::chrono::milliseconds delay;

    // TODO: add functions to make sleep be an awaitable object
    bool await_ready() const noexcept{
        return false;
    }
    void await_suspend(std::coroutine_handle<> h) noexcept{
        auto start = std::chrono::steady_clock::now();
        // 将 lambda 函数推入任务队列，用于延时唤醒协程
        task_queue.push([start, h, d = delay]{
            if(decltype(start)::clock::now() - start > d){
                h.resume(); return true;
            }else{ return false; }
        });
    }
    void await_resume() const noexcept{}
};
// Task 结构体，用于协程中的任务处理
struct Task {
    // TODO: add functions to make Task be an coroutine handle
    // promise_type 结构体，用于 Task 的协程处理
    struct promise_type{
        std::string value;

        std::suspend_never initial_suspend() noexcept { return{}; }

        std::suspend_never final_suspend() noexcept { return{}; }

        Task get_return_object() noexcept { return{}; }

        void unhandled_exception() {}

        void return_void() {}
    };
};
// 等待任务队列为空的函数
void wait_task_queue_empty() {
    // TODO: block current thread until task queue is empty
    while (!task_queue.empty()) {
        auto task = task_queue.front();
        if(!task()) task_queue.push(task);
        task_queue.pop();
        // 短暂延时，避免空闲等待导致的高 CPU 占用
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

}  // namespace coro
