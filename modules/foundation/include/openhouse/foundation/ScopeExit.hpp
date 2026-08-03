#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace openhouse::foundation {

template<typename F>
class ScopeExit {
public:
    static_assert(std::is_nothrow_invocable_v<F&>,
                  "ScopeExit requires a noexcept callable");

    explicit ScopeExit(F func) noexcept(std::is_nothrow_move_constructible_v<F>)
        : func_(std::move(func)) {}

    ~ScopeExit() noexcept {
        if (active_) {
            std::invoke(func_);
        }
    }

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;

    ScopeExit(ScopeExit&& other) noexcept(std::is_nothrow_move_constructible_v<F>)
        : func_(std::move(other.func_)), active_(std::exchange(other.active_, false)) {}

    ScopeExit& operator=(ScopeExit&&) = delete;

    void Release() noexcept {
        active_ = false;
    }

private:
    F func_;
    bool active_{true};
};

}
