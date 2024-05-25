#pragma once

#include <coroutine>
#include <iterator>
#include <utility>

namespace coro {

template <typename Ref, typename Value = std::remove_cvref_t<Ref>>
class generator {
public:
    // TODO: implement promise_type
    // 协程的 promise_type 类，负责管理协程状态和生成值
    class promise_type {
    public:
        // 构造函数将协程的根初始化为自身
        promise_type() : root_(this) {}
        // 返回一个生成器对象，与当前 promise 关联
        generator get_return_object() noexcept {
            return generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // 处理未处理的异常，存储异常指针
        void unhandled_exception() {
            if (exception_ == nullptr)
                throw;
            *exception_ = std::current_exception();
        }
        // 处理协程返回 void 的情况
        void return_void() noexcept {}
        // 初始暂停点，协程首次启动时使用
        std::suspend_always initial_suspend() noexcept { return {}; }

        // Transfers control back to the parent of a nested coroutine
        struct final_awaiter {
            bool await_ready() noexcept {
                return false;
            }
            std::coroutine_handle<> await_suspend(
                std::coroutine_handle<promise_type> h) noexcept {
                auto& promise  = h.promise();
                auto  parent   = h.promise().parent_;
                if (parent) {
                    promise.root_->leaf_ = parent;
                    return std::coroutine_handle<promise_type>::from_promise(*parent);
                }
                return std::noop_coroutine();
            }
            void await_resume() noexcept {}
        };

        final_awaiter final_suspend() noexcept { return {}; }
        // 暂停点，用于从协程中产生值
        std::suspend_always yield_value(Ref&& x) noexcept {
            root_->value_ = std::addressof(x);
            return {};
        }
        std::suspend_always yield_value(Ref& x) noexcept {
            root_->value_ = std::addressof(x);
            return {};
        }

        // 用于支持生成器链式调用，传递嵌套生成器
        struct yield_sequence_awaiter {
            generator gen_;
            std::exception_ptr exception_;

            explicit yield_sequence_awaiter(generator&& g) noexcept
            // Taking ownership of the generator ensures frame are destroyed 
            // in the reverse order of their creation
            : gen_(std::move(g))
            {}

            bool await_ready() noexcept {
                return !gen_.coro_;
            }

            // set the parent, root and exceptions pointer and
            // resume the nested coroutine
            std::coroutine_handle<> await_suspend(
                std::coroutine_handle<promise_type> h) noexcept {
                auto& current = h.promise();
                auto& nested  = gen_.coro_.promise();
                auto& root    = current.root_;
                
                nested.root_   = root;
                root->leaf_    = &nested;
                nested.parent_ = &current;

                nested.exception_  = &exception_;
                
                // Immediately resume the nested coroutine (nested generator)
                return gen_.coro_;
            }

            void await_resume() {
                if (exception_) {
                    std::rethrow_exception(std::move(exception_));
                }
            }
        };
        // 生成器链式调用，产生嵌套生成器
        yield_sequence_awaiter yield_value(generator&& g) noexcept {
            return yield_sequence_awaiter{std::move(g)};
        }
        // 恢复与此 promise 关联的协程
        void resume() {
            std::coroutine_handle<promise_type>::from_promise(*leaf_).resume();
        }

        // Disable use of co_await within this coroutine.
        void await_transform() = delete;
    private:
        friend generator;
        
        // Technically UB, for demonstration purpose
        union {
            promise_type* root_;
            promise_type* leaf_;
        };
        promise_type* parent_ = nullptr;
        std::exception_ptr* exception_ = nullptr;
        std::add_pointer_t<Ref> value_;
    };

    generator() noexcept = default;
    generator(generator &&other) noexcept
        : coro_(std::exchange(other.coro_, {}))
        {}

    ~generator() noexcept {
        /* TODO */
        if (coro_) {
            coro_.destroy();
        }
    }
    // 生成器的结束标志
    struct sentinel {};
    // 生成器的迭代器类
    class iterator {
        using coroutine_handle = std::coroutine_handle<promise_type>;
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Value;
        using reference = Ref;
        using pointer = std::add_pointer_t<Ref>;

        iterator() noexcept = default;
        iterator(const iterator&) = delete;
        iterator(iterator&& o) {
            std::swap(coro_, o.coro_);
        }

        iterator& operator=(iterator&& o) {
            std::swap(coro_, o.coro_);
            return *this;
        }

        ~iterator() {}

        // TODO: implement operator== and operator!=
        friend bool operator==(const iterator &it, sentinel) noexcept {
            return !it.coro_ || it.coro_.done();
        }
        friend bool operator!=(const iterator &it, sentinel) noexcept {
            return it.coro_ && !it.coro_.done();
        }
        // TODO: implement operator++ and operator++(int)
        iterator &operator++() {
            coro_.promise().resume();
            return *this;
        }
        void operator++(int) {
            (void)operator++();
        }
        // TODO: implement operator* and operator->
        reference operator*() const noexcept {
            return static_cast<reference>(*coro_.promise().value_);
        }

        pointer operator->() const noexcept 
        requires std::is_reference_v<reference> {
            return std::addressof(operator*());
        }

    private:
        friend generator;

        // TODO: implement iterator constructor
        // hint: maybe you need to a promise handle
        explicit iterator(coroutine_handle coro) noexcept
            : coro_(coro) {}

        // TODO: add member variables you need
        coroutine_handle coro_;
    };

    // TODO: implement begin() and end() member functions
    iterator begin() {
        if (coro_) {
            coro_.resume();
        }
        return iterator{coro_};
    }

    sentinel end() noexcept {
        return {};
    }

private:
    // TODO: implement generator constructor
    explicit generator(std::coroutine_handle<promise_type> coro) noexcept
    : coro_(coro)
    {}

    // TODO: add member variables you need
    // 迭代器的协程句柄
    std::coroutine_handle<promise_type> coro_;
};

}  // namespace coro
