#pragma once

#include "NonCopyable.hpp"

namespace openhouse::foundation {

template<typename T>
class Singleton : private NonCopyable {
protected:
    Singleton() = default;
public:
    static T& Instance() {
        static T instance;
        return instance;
    }
};

}
