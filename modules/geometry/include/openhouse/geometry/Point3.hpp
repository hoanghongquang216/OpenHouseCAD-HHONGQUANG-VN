#pragma once

namespace openhouse::geometry {

template<typename T>
struct Point3 {
    T x{};
    T y{};
    T z{};
};

using Point3f = Point3<float>;
using Point3d = Point3<double>;
using Point3i = Point3<int>;

}
