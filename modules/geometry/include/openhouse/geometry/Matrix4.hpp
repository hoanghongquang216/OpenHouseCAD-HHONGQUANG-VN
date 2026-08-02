#pragma once

namespace openhouse::geometry
{

class Matrix4
{
public:
    constexpr Matrix4()
        : data_{1.0,0.0,0.0,0.0,
                0.0,1.0,0.0,0.0,
                0.0,0.0,1.0,0.0,
                0.0,0.0,0.0,1.0}
    {
    }

    constexpr double At(int row, int column) const
    {
        return data_[row * 4 + column];
    }

private:
    double data_[16];
};

}
