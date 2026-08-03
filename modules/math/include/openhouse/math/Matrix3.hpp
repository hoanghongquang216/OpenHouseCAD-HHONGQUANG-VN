#pragma once

#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/geometry/Point2.hpp>
#include <openhouse/geometry/Vector2.hpp>
#include <openhouse/math/Angle.hpp>

#include <array>

namespace openhouse::math {

// A 3x3 row-major matrix for 2D affine transforms (translation, rotation,
// scale), the 2D analog of Matrix4. Unlike Matrix4 (which was built
// ahead of any real consumer -- see conversation history's audit
// finding), Matrix3 exists because it is immediately needed: all of
// this project's geometry/render work so far is 2D (Point2, Line2,
// Circle2, Arc2, SvgDocument), and Spiral 3 (Selection/Move) will need
// to move 2D points around. See Matrix4.hpp for the row-major storage /
// column-vector transform convention this mirrors.
template<foundation::FloatingPoint T>
class Matrix3 {
public:
    constexpr Matrix3() noexcept : rows_{} {
        for (int i = 0; i < 3; ++i) {
            rows_[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)] = T{1};
        }
    }

    [[nodiscard]] static constexpr Matrix3 Identity() noexcept { return Matrix3(); }

    [[nodiscard]] static constexpr Matrix3 Translation(const geometry::Vector2<T>& t) noexcept {
        Matrix3 m = Identity();
        m.At(0, 2) = t.x;
        m.At(1, 2) = t.y;
        return m;
    }

    [[nodiscard]] static constexpr Matrix3 Scale(const geometry::Vector2<T>& s) noexcept {
        Matrix3 m = Zero();
        m.At(0, 0) = s.x;
        m.At(1, 1) = s.y;
        m.At(2, 2) = T{1};
        return m;
    }

    [[nodiscard]] static constexpr Matrix3 UniformScale(T s) noexcept {
        return Scale(geometry::Vector2<T>{s, s});
    }

    [[nodiscard]] static Matrix3 Rotation(Angle<T> angle) noexcept {
        const T c = Cos(angle);
        const T s = Sin(angle);
        Matrix3 m = Identity();
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

    friend constexpr bool operator==(const Matrix3&, const Matrix3&) = default;

    [[nodiscard]] friend constexpr Matrix3 operator*(const Matrix3& a, const Matrix3& b) noexcept {
        Matrix3 result = Zero();
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                T sum{};
                for (int k = 0; k < 3; ++k) {
                    sum += a.At(r, k) * b.At(k, c);
                }
                result.At(r, c) = sum;
            }
        }
        return result;
    }

    // Transforms a direction: ignores translation (w = 0).
    [[nodiscard]] friend constexpr geometry::Vector2<T> operator*(
        const Matrix3& m, const geometry::Vector2<T>& v) noexcept {
        return {
            m.At(0, 0) * v.x + m.At(0, 1) * v.y,
            m.At(1, 0) * v.x + m.At(1, 1) * v.y,
        };
    }

    // Transforms a position: includes translation (w = 1).
    [[nodiscard]] friend constexpr geometry::Point2<T> operator*(
        const Matrix3& m, const geometry::Point2<T>& p) noexcept {
        const T x = m.At(0, 0) * p.x + m.At(0, 1) * p.y + m.At(0, 2);
        const T y = m.At(1, 0) * p.x + m.At(1, 1) * p.y + m.At(1, 2);
        const T w = m.At(2, 0) * p.x + m.At(2, 1) * p.y + m.At(2, 2);
        return {x / w, y / w};
    }

private:
    [[nodiscard]] static constexpr Matrix3 Zero() noexcept {
        Matrix3 m;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                m.At(r, c) = T{0};
            }
        }
        return m;
    }

    std::array<std::array<T, 3>, 3> rows_;
};

using Matrix3f = Matrix3<float>;
using Matrix3d = Matrix3<double>;

}
