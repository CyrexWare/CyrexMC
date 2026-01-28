#pragma once

#include <cmath>
#include <cstdint>

namespace cyrex::math
{
struct Vector2
{
    float x;
    float y;

    constexpr Vector2() : x(0), y(0)
    {
    }
    constexpr Vector2(float x_, float y_) : x(x_), y(y_)
    {
    }

    static constexpr Vector2 zero()
    {
        return {0.f, 0.f};
    }
    static constexpr Vector2 one()
    {
        return {1.f, 1.f};
    }

    float length() const
    {
        return std::sqrt(x * x + y * y);
    }

    float lengthSquared() const
    {
        return x * x + y * y;
    }

    Vector2 normalized() const
    {
        float len = length();
        if (len == 0.f)
            return Vector2::zero();
        return {x / len, y / len};
    }

    float dot(const Vector2& o) const
    {
        return x * o.x + y * o.y;
    }

    float distance(const Vector2& o) const
    {
        return (*this - o).length();
    }

    float distanceSquared(const Vector2& o) const
    {
        return (*this - o).lengthSquared();
    }

    Vector2 floor() const
    {
        return {std::floor(x), std::floor(y)};
    }

    Vector2 ceil() const
    {
        return {std::ceil(x), std::ceil(y)};
    }

    Vector2 abs() const
    {
        return {std::fabs(x), std::fabs(y)};
    }

    Vector2 operator+(const Vector2& o) const
    {
        return {x + o.x, y + o.y};
    }

    Vector2 operator-(const Vector2& o) const
    {
        return {x - o.x, y - o.y};
    }

    Vector2 operator*(float s) const
    {
        return {x * s, y * s};
    }

    Vector2 operator/(float s) const
    {
        return {x / s, y / s};
    }

    Vector2& operator+=(const Vector2& o)
    {
        x += o.x;
        y += o.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& o)
    {
        x -= o.x;
        y -= o.y;
        return *this;
    }

    Vector2& operator*=(float s)
    {
        x *= s;
        y *= s;
        return *this;
    }

    Vector2& operator/=(float s)
    {
        x /= s;
        y /= s;
        return *this;
    }

    bool operator==(const Vector2& o) const
    {
        return x == o.x && y == o.y;
    }

    bool operator!=(const Vector2& o) const
    {
        return !(*this == o);
    }
};
} // namespace cyrex::math
