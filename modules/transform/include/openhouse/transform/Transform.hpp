#pragma once

#include <openhouse/transform/Matrix4.hpp>

namespace openhouse::transform
{

class Transform
{
public:
    Transform() = default;

    const Matrix4& Matrix() const
    {
        return matrix_;
    }

    void SetMatrix(const Matrix4& matrix)
    {
        matrix_ = matrix;
    }

private:
    Matrix4 matrix_;
};

}
