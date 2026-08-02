#pragma once

#include <openhouse/geometry/Matrix4.hpp>
#include <openhouse/geometry/Point3D.hpp>

namespace openhouse::geometry
{

class Transform
{
public:
    Transform()
        : position_(0.0, 0.0, 0.0), matrix_(Matrix4::Identity())
    {
    }

    explicit Transform(const Point3D& position)
        : position_(position), matrix_(Matrix4::Translation(position.X(), position.Y(), position.Z()))
    {
    }

    const Point3D& Position() const { return position_; }

    void SetPosition(const Point3D& position)
    {
        position_ = position;
        RebuildMatrix();
    }

    void SetRotation(const Matrix4& rotation)
    {
        rotation_ = rotation;
        RebuildMatrix();
    }

    void SetScale(const Matrix4& scale)
    {
        scale_ = scale;
        RebuildMatrix();
    }

    Point3D Apply(const Point3D& point) const
    {
        return matrix_.Transform(point);
    }

    const Matrix4& Matrix() const { return matrix_; }

private:
    void RebuildMatrix()
    {
        matrix_ = Matrix4::Translation(position_.X(), position_.Y(), position_.Z())
            .Multiply(rotation_)
            .Multiply(scale_);
    }

private:
    Point3D position_;
    Matrix4 rotation_ = Matrix4::Identity();
    Matrix4 scale_ = Matrix4::Identity();
    Matrix4 matrix_;
};

}
