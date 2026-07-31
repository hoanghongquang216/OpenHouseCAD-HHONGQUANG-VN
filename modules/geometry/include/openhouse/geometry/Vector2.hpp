#pragma once

namespace openhouse::geometry {

template<typename T>
struct Vector2 {
    T x{};
    T y{};
};

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2i = Vector2<int>;

}
