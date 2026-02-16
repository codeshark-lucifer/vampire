#pragma once
#include <cmath>

struct Vec2; // Forward declaration

struct IVec2
{
    int x, y;

    constexpr IVec2() : x(0), y(0) {}
    constexpr IVec2(int v) : x(v), y(v) {}
    constexpr IVec2(int x, int y) : x(x), y(y) {}

    // Constructor from Vec2, defined out-of-line later
    explicit constexpr IVec2(const Vec2 &v);

    constexpr IVec2 operator+(const IVec2 &v) const { return {x + v.x, y + v.y}; }
    constexpr IVec2 operator-(const IVec2 &v) const { return {x - v.x, y - v.y}; }
    constexpr IVec2 operator*(int s) const { return {x * s, y * s}; }
    constexpr IVec2 operator/(int s) const { return {x / s, y / s}; }

    IVec2 &operator+=(const IVec2 &v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    IVec2 &operator-=(const IVec2 &v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    static constexpr int Dot(const IVec2 &a, const IVec2 &b)
    {
        return a.x * b.x + a.y * b.y;
    }
};

struct Vec2
{
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float v) : x(v), y(v) {}
    Vec2(float x, float y) : x(x), y(y) {}
    explicit Vec2(const IVec2 &v) : x(static_cast<float>(v.x)), y(static_cast<float>(v.y)) {}

    Vec2 operator+(const Vec2 &v) const { return {x + v.x, y + v.y}; }
    Vec2 operator-(const Vec2 &v) const { return {x - v.x, y - v.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }

    Vec2 &operator+=(const Vec2 &v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    float Length() const
    {
        return std::sqrt(x * x + y * y);
    }

    Vec2 Normalized() const
    {
        float len = Length();
        if (len == 0.0f)
            return Vec2(0);
        return *this / len;
    }

    static float Dot(const Vec2 &a, const Vec2 &b)
    {
        return a.x * b.x + a.y * b.y;
    }
};

// Out-of-line definition for IVec2 constructor
constexpr IVec2::IVec2(const Vec2 &v)
    : x(static_cast<int>(v.x)),
      y(static_cast<int>(v.y)) {}

struct Vec3
{
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float v) : x(v), y(v), z(v) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3 &v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(const Vec3 &v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
    Vec3 operator-() const
    {
        return Vec3(-x, -y, -z);
    }

    Vec3 &operator+=(const Vec3 &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    float Length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 Normalized() const
    {
        float len = Length();
        if (len == 0.0f)
            return Vec3(0);
        return *this / len;
    }

    static float Dot(const Vec3 &a, const Vec3 &b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vec3 Cross(const Vec3 &a, const Vec3 &b)
    {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
    }
};

struct Vec4
{
    float x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float v) : x(v), y(v), z(v), w(v) {}
    Vec4(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w) {}

    Vec4(const Vec3 &v, float w)
        : x(v.x), y(v.y), z(v.z), w(w) {}

    Vec4 operator+(const Vec4 &v) const
    {
        return {x + v.x, y + v.y, z + v.z, w + v.w};
    }

    Vec4 operator-(const Vec4 &v) const
    {
        return {x - v.x, y - v.y, z - v.z, w - v.w};
    }

    Vec4 operator*(float s) const
    {
        return {x * s, y * s, z * s, w * s};
    }

    Vec4 operator/(float s) const
    {
        return {x / s, y / s, z / s, w / s};
    }

    Vec3 xyz() { return Vec3(x, y, z); }
    static float Dot(const Vec4 &a, const Vec4 &b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }
};

inline Vec3 operator*(float s, const Vec3 &v)
{
    return {v.x * s, v.y * s, v.z * s};
}
