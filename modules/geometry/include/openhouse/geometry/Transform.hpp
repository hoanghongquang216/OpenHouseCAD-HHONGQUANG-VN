#pragma once

#include <openhouse/geometry/Matrix4.hpp>

namespace openhouse::geometry
{

class Transform
{
public:
    constexpr Transform() = default;

    const Matrix4& Matrix() const
    {
        return matrix_;
    }

private:
    Matrix4 matrix_;
};

}
