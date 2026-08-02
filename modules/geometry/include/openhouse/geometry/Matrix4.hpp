#pragma once

#include <cmath>

#include <openhouse/geometry/Point3D.hpp>

namespace openhouse::geometry
{

class Matrix4
{
public:
    constexpr Matrix4()
        : data_{}
    {
        data_[0] = 1.0;
        data_[5] = 1.0;
        data_[10] = 1.0;
        data_[15] = 1.0;
    }

    static Matrix4 Identity() { return Matrix4(); }

    static Matrix4 Translation(double x, double y, double z)
    {
        Matrix4 matrix;
        matrix.data_[3] = x;
        matrix.data_[7] = y;
        matrix.data_[11] = z;
        return matrix;
    }

    static Matrix4 Scale(double x, double y, double z)
    {
        Matrix4 matrix;
        matrix.data_[0] = x;
        matrix.data_[5] = y;
        matrix.data_[10] = z;
        return matrix;
    }

    static Matrix4 RotationX(double angle)
    {
        Matrix4 matrix;
        const double c = std::cos(angle);
        const double s = std::sin(angle);

        matrix.data_[5] = c;
        matrix.data_[6] = -s;
        matrix.data_[9] = s;
        matrix.data_[10] = c;
        return matrix;
    }

    static Matrix4 RotationY(double angle)
    {
        Matrix4 matrix;
        const double c = std::cos(angle);
        const double s = std::sin(angle);

        matrix.data_[0] = c;
        matrix.data_[2] = s;
        matrix.data_[8] = -s;
        matrix.data_[10] = c;
        return matrix;
    }

    static Matrix4 RotationZ(double angle)
    {
        Matrix4 matrix;
        const double c = std::cos(angle);
        const double s = std::sin(angle);

        matrix.data_[0] = c;
        matrix.data_[1] = -s;
        matrix.data_[4] = s;
        matrix.data_[5] = c;
        return matrix;
    }

    Matrix4 Multiply(const Matrix4& other) const
    {
        Matrix4 result;

        for (int row = 0; row < 4; ++row)
        {
            for (int column = 0; column < 4; ++column)
            {
                result.data_[row * 4 + column] = 0.0;
                for (int k = 0; k < 4; ++k)
                {
                    result.data_[row * 4 + column] +=
                        data_[row * 4 + k] * other.data_[k * 4 + column];
                }
            }
        }

        return result;
    }

    Point3D Transform(const Point3D& point) const
    {
        return Point3D(
            data_[0] * point.X() + data_[3],
            data_[5] * point.Y() + data_[7],
            data_[10] * point.Z() + data_[11]);
    }

    constexpr double At(int row, int column) const
    {
        return data_[row * 4 + column];
    }

private:
    double data_[16];
};

}
