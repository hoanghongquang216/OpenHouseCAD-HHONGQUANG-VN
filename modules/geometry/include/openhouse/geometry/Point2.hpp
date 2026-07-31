#pragma once

namespace openhouse::geometry {

template<typename T>
struct Point2 {
    T x{};
    T y{};
};

using Point2f = Point2<float>;
using Point2d = Point2<double>;
using Point2i = Point2<int>;

}
