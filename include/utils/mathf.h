#pragma once
#include "color.h"
#include "matrix.h"
#include "Quat.h"
#include "vector.h"
#include "phys2d.h"
#include "logger.h"
#include "constants.h"

namespace mathf
{
    inline float radians(const float &degrees)
    {
        return degrees * (PI / 180.0f);
    }

    inline float lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    inline Vec2 linearInterpolation(Vec2 start, Vec2 end, float t) {
        return start + (end - start) * t;
    }

    inline Vec2 BezierInterpolation(Vec2 p0, Vec2 p1, Vec2 p2, float t) {
        Vec2 intermediateA = linearInterpolation(p0, p1, t);
        Vec2 intermediateB = linearInterpolation(p1, p2, t);
        return linearInterpolation(intermediateA, intermediateB, t);
    }

    inline Vec2 lerp(Vec2 a, Vec2 b, float t)
    {
        return {
            mathf::lerp(a.x, b.x, t),
            mathf::lerp(a.y, b.y, t)};
    }

    inline IVec2 lerp(IVec2 a, IVec2 b, float t)
    {
        return {
            (int)std::floor(lerp((float)a.x, (float)b.x, t)),
            (int)std::floor(lerp((float)a.y, (float)b.y, t))};
    }

    inline int sign(int x)
    {
        return (x >= 0) ? 1 : -1;
    }

    inline float sign(float x)
    {
        return (x >= 0) ? 1 : -1;
    }

    inline long long max(long long a, long long b)
    {
        return a > b ? a : b;
    }

    inline float max(float a, float b)
    {
        return a > b ? a : b;
    }

    inline int max(int a, int b)
    {
        return a > b ? a : b;
    }

    inline long long min(long long a, long long b)
    {
        return a < b ? a : b;
    }

    inline float min(float a, float b)
    {
        return a < b ? a : b;
    }

    inline int min(int a, int b)
    {
        return a < b ? a : b;
    }

    inline float approach(float current, float target, float increase)
    {
        if (current < target)
            return mathf::min(current + increase, target);
        return mathf::max(current - increase, target);
    }

    template <typename T>
    inline T clamp(T value, T min_value, T max_value)
    {
        return mathf::max(min_value, mathf::min(value, max_value));
    }
}


