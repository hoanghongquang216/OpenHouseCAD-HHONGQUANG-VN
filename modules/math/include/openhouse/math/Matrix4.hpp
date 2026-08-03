#pragma once

#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/geometry/Point3.hpp>
#include <openhouse/geometry/Vector3.hpp>
#include <openhouse/math/Angle.hpp>

#include <array>

namespace openhouse::math {

// A 4x4 row-major matrix, primarily intended for affine transforms
// (translation, rotation, scale) composed via multiplication. Row-major
// storage: m[row][col]. Points/vectors are treated as column vectors,
// transformed as M * v.
template<foundation::FloatingPoint T>
class Matrix4 {
public:
    constexpr Matrix4() noexcept : rows_{} {
        for (int i = 0; i < 4; ++i) {
            rows_[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)] = T{1};
        }
    }

    [[nodiscard]] static constexpr Matrix4 Identity() noexcept { return Matrix4(); }

    [[nodiscard]] static constexpr Matrix4 Translation(const geometry::Vector3<T>& t) noexcept {
        Matrix4 m = Identity();
        m.At(0, 3) = t.x;
        m.At(1, 3) = t.y;
        m.At(2, 3) = t.z;
        return m;
    }

    [[nodiscard]] static constexpr Matrix4 Scale(const geometry::Vector3<T>& s) noexcept {
        Matrix4 m = Zero();
        m.At(0, 0) = s.x;
        m.At(1, 1) = s.y;
        m.At(2, 2) = s.z;
        m.At(3, 3) = T{1};
        return m;
    }

    [[nodiscard]] static constexpr Matrix4 UniformScale(T s) noexcept {
        return Scale(geometry::Vector3<T>{s, s, s});
    }

    [[nodiscard]] static Matrix4 RotationX(Angle<T> angle) noexcept {
        const T c = Cos(angle);
        const T s = Sin(angle);
        Matrix4 m = Identity();
        m.At(1, 1) = c;
        m.At(1, 2) = -s;
        m.At(2, 1) = s;
        m.At(2, 2) = c;
        return m;
    }

    [[nodiscard]] static Matrix4 RotationY(Angle<T> angle) noexcept {
        const T c = Cos(angle);
        const T s = Sin(angle);
        Matrix4 m = Identity();
        m.At(0, 0) = c;
        m.At(0, 2) = s;
        m.At(2, 0) = -s;
        m.At(2, 2) = c;
        return m;
    }

    [[nodiscard]] static Matrix4 RotationZ(Angle<T> angle) noexcept {
        const T c = Cos(angle);
        const T s = Sin(angle);
        Matrix4 m = Identity();
        m.At(0, 0) = c;
        m.At(0, 1) = -s;
        m.At(1, 0) = s;
        m.At(1, 1) = c;
        return m;
    }

    [[nodiscard]] constexpr T At(int row, int col) const noexcept {
        return rows_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
    }

    [[nodiscard]] constexpr T& At(int row, int col) noexcept {
        return rows_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
    }

    [[nodiscard]] constexpr Matrix4 Transposed() const noexcept {
        Matrix4 result = Zero();
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                result.At(c, r) = At(r, c);
            }
        }
        return result;
    }

    friend constexpr bool operator==(const Matrix4&, const Matrix4&) = default;

    [[nodiscard]] friend constexpr Matrix4 operator*(const Matrix4& a, const Matrix4& b) noexcept {
        Matrix4 result = Zero();
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                T sum{};
                for (int k = 0; k < 4; ++k) {
                    sum += a.At(r, k) * b.At(k, c);
                }
                result.At(r, c) = sum;
            }
        }
        return result;
    }

    // Transforms a direction: ignores translation (w = 0).
    [[nodiscard]] friend constexpr geometry::Vector3<T> operator*(
        const Matrix4& m, const geometry::Vector3<T>& v) noexcept {
        return {
            m.At(0, 0) * v.x + m.At(0, 1) * v.y + m.At(0, 2) * v.z,
            m.At(1, 0) * v.x + m.At(1, 1) * v.y + m.At(1, 2) * v.z,
            m.At(2, 0) * v.x + m.At(2, 1) * v.y + m.At(2, 2) * v.z,
        };
    }

    // Transforms a position: includes translation (w = 1), then
    // dehomogenizes (divides by w). For a pure affine matrix (the only
    // kind this class constructs), w is always 1 after the transform, so
    // the divide is effectively a no-op safety net rather than a
    // meaningful perspective divide.
    [[nodiscard]] friend constexpr geometry::Point3<T> operator*(
        const Matrix4& m, const geometry::Point3<T>& p) noexcept {
        const T x = m.At(0, 0) * p.x + m.At(0, 1) * p.y + m.At(0, 2) * p.z + m.At(0, 3);
        const T y = m.At(1, 0) * p.x + m.At(1, 1) * p.y + m.At(1, 2) * p.z + m.At(1, 3);
        const T z = m.At(2, 0) * p.x + m.At(2, 1) * p.y + m.At(2, 2) * p.z + m.At(2, 3);
        const T w = m.At(3, 0) * p.x + m.At(3, 1) * p.y + m.At(3, 2) * p.z + m.At(3, 3);
        return {x / w, y / w, z / w};
    }

private:
    [[nodiscard]] static constexpr Matrix4 Zero() noexcept {
        Matrix4 m;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                m.At(r, c) = T{0};
            }
        }
        return m;
    }

    std::array<std::array<T, 4>, 4> rows_;
};

using Matrix4f = Matrix4<float>;
using Matrix4d = Matrix4<double>;

}
