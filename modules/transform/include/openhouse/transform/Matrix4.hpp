#pragma once

#include <array>
#include <cstddef>

namespace openhouse::transform
{

class Matrix4
{
public:
    Matrix4()
    {
        Identity();
    }

    void Identity()
    {
        data_.fill(0.0);
        data_[0] = 1.0;
        data_[5] = 1.0;
        data_[10] = 1.0;
        data_[15] = 1.0;
    }

    double& operator()(std::size_t row, std::size_t column)
    {
        return data_[row * 4 + column];
    }

    double operator()(std::size_t row, std::size_t column) const
    {
        return data_[row * 4 + column];
    }

private:
    std::array<double, 16> data_{};
};

}
