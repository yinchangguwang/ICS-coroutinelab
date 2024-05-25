#pragma once

#include <coroutine>
#include <iterator>
#include <utility>

namespace coro {

template <typename Ref, typename Value = std::remove_cvref_t<Ref>>
class generator {
public:
    // TODO: implement promise_type
    class promise_type {
    public:
        promise_type() : root_(this) {}
        //返回与此promise关联的generator对象
        generator get_return_object() noexcept {
            return generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        //处理未处理的异常，存储异常指针
        void unhandled_exception() {
            if (exception_ == nullptr)
                throw;
            *exception_ = std::current_exception();
        }
        //处理协程返回void的情况
        void return_void() noexcept {}

        //协程初始暂停点
        std::suspend_always initial_suspend() noexcept { return {}; }

        // Transfers control back to the parent of a nested coroutine
        //用于将控制权传递回嵌套协程的父协程的最终暂停点
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

        std::suspend_always yield_value(Ref&& x) noexcept {
            root_->value_ = std::addressof(x);
            return {};
        }
        std::suspend_always yield_value(Ref& x) noexcept {
            root_->value_ = std::addressof(x);
            return {};
        }

        //恢复与此promise关联的协程
        void resume() {
            std::coroutine_handle<promise_type>::from_promise(*leaf_).resume();
        }

        // Disable use of co_await within this coroutine.
        //禁止在此协程内使用co_await
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
    //默认构造函数
    generator() noexcept = default;
    //协程超出范围时销毁协程
    ~generator() noexcept {
        /* TODO */
        if (coro_) {
            coro_.destroy();
        }
    }
    //用于生成器末尾的哨兵值
    struct sentinel {};
    //生成器的迭代器
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
        //迭代器的的移动构造函数
        iterator(iterator&& o) {
            std::swap(coro_, o.coro_);
        }
        //迭代器的移动赋值运算符
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
        //迭代器的协程句柄
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
    std::coroutine_handle<promise_type> coro_;
};

}  // namespace coro
