#pragma once

#include <openhouse/geometry/Point3D.hpp>

namespace openhouse::geometry
{

class Matrix4
{
public:
    Matrix4()
    {
        SetIdentity();
    }

    static Matrix4 Identity()
    {
        return Matrix4();
    }

    static Matrix4 Translation(double x, double y, double z)
    {
        Matrix4 matrix;
        matrix.data_[3] = x;
        matrix.data_[7] = y;
        matrix.data_[11] = z;
        return matrix;
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
    void SetIdentity()
    {
        for (int i = 0; i < 16; ++i)
        {
            data_[i] = 0.0;
        }

        data_[0] = 1.0;
        data_[5] = 1.0;
        data_[10] = 1.0;
        data_[15] = 1.0;
    }

private:
    double data_[16];
};

}
