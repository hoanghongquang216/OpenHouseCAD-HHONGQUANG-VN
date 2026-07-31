#pragma once

#include <utility>

namespace openhouse::foundation {

template<class F>
class ScopeExit {
public:
    explicit ScopeExit(F&& f) : func_(std::forward<F>(f)) {}
    ~ScopeExit() { func_(); }
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
private:
    F func_;
};

}
