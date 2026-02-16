#pragma once
#include "vector.h" // Added to define Vec3

struct Color {
    constexpr Color() : r(0), g(0), b(0), a(0) {}
    constexpr Color(float v) : r(v), g(v), b(v), a(v) {}
    constexpr Color(float r, float g, float b, float a)
        : r(r), g(g), b(b), a(a) {}

    constexpr Color(const Vec3 &v, float a)
        : r(v.x), g(v.y), b(v.z), a(a) {}

    Color operator+(const Color &v) const { return {r + v.r, g + v.g, b + v.b, a + v.a}; }
    Color operator-(const Color &v) const { return {r - v.r, g - v.g, b - v.b, a - v.a}; }
    Color operator*(float s) const { return {r * s, g * s, b * s, a * s}; }
    Color operator/(float s) const { return {r / s, g / s, b / s, a / s}; }

    bool operator==(const Color &other) const { return r == other.r && g == other.g && b == other.b && a == other.a; }
    bool operator!=(const Color &other) const { return !(*this == other); }

    Vec3 rgb() const { return Vec3(r, g, b); }
    float r, g, b, a; // Moved members to end for aggregate initialization with C++20
};

// Common color constants
inline constexpr Color COLOR_WHITE {1.0f, 1.0f, 1.0f, 1.0f};
inline constexpr Color COLOR_BLACK {0.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color COLOR_RED   {1.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color COLOR_GREEN {0.0f, 1.0f, 0.0f, 1.0f};
inline constexpr Color COLOR_BLUE  {0.0f, 0.0f, 1.0f, 1.0f};
