#pragma once

#include <cmath>
#include <cstdint>

namespace cyrex::math
{
struct Vector3
{
    float x;
    float y;
    float z;

    constexpr Vector3() : x(0), y(0), z(0)
    {
    }
    constexpr Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_)
    {
    }

    static constexpr Vector3 zero()
    {
        return {0.f, 0.f, 0.f};
    }
    static constexpr Vector3 one()
    {
        return {1.f, 1.f, 1.f};
    }
    static constexpr Vector3 up()
    {
        return {0.f, 1.f, 0.f};
    }
    static constexpr Vector3 down()
    {
        return {0.f, -1.f, 0.f};
    }

    float length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    float lengthSquared() const
    {
        return x * x + y * y + z * z;
    }

    Vector3 normalized() const
    {
        float len = length();
        if (len == 0.f)
            return Vector3::zero();
        return {x / len, y / len, z / len};
    }

    float dot(const Vector3& o) const
    {
        return x * o.x + y * o.y + z * o.z;
    }

    Vector3 cross(const Vector3& o) const
    {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }

    float distance(const Vector3& o) const
    {
        return (*this - o).length();
    }

    float distanceSquared(const Vector3& o) const
    {
        return (*this - o).lengthSquared();
    }

    Vector3 floor() const
    {
        return {std::floor(x), std::floor(y), std::floor(z)};
    }

    Vector3 ceil() const
    {
        return {std::ceil(x), std::ceil(y), std::ceil(z)};
    }

    Vector3 abs() const
    {
        return {std::fabs(x), std::fabs(y), std::fabs(z)};
    }

    Vector3 lerp(const Vector3& to, float t) const
    {
        return {x + (to.x - x) * t, y + (to.y - y) * t, z + (to.z - z) * t};
    }

    Vector3 operator+(const Vector3& o) const
    {
        return {x + o.x, y + o.y, z + o.z};
    }

    Vector3 operator-(const Vector3& o) const
    {
        return {x - o.x, y - o.y, z - o.z};
    }

    Vector3 operator*(float s) const
    {
        return {x * s, y * s, z * s};
    }

    Vector3 operator/(float s) const
    {
        return {x / s, y / s, z / s};
    }

    Vector3& operator+=(const Vector3& o)
    {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& o)
    {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        return *this;
    }

    Vector3& operator*=(float s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    Vector3& operator/=(float s)
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    bool operator==(const Vector3& o) const
    {
        return x == o.x && y == o.y && z == o.z;
    }

    bool operator!=(const Vector3& o) const
    {
        return !(*this == o);
    }
};
} // namespace cyrex::math
