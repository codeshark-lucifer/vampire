#pragma once
#include <cmath>
#include "vector.h"

#define M_PI 3.14159265f

struct Quat
{
    float w, x, y, z;

    Quat() : w(1), x(0), y(0), z(0) {}
    Quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    static Quat Identity()
    {
        return Quat();
    }

    static Quat FromAxisAngle(const Vec3 &axis, float radians)
    {
        float half = radians * 0.5f;
        float s = std::sin(half);
        return {
            std::cos(half),
            axis.x * s,
            axis.y * s,
            axis.z * s};
    }

    Quat Normalized() const
    {
        float len = std::sqrt(w * w + x * x + y * y + z * z);
        return {w / len, x / len, y / len, z / len};
    }

    Quat operator*(const Quat &q) const
    {
        return {
            w * q.w - x * q.x - y * q.y - z * q.z,
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w};
    }
    static Quat FromEuler(const Vec3 &eulerDegrees)
    {
        // Convert degrees → radians
        Vec3 r = {
            eulerDegrees.x * (float)M_PI / 180.0f,
            eulerDegrees.y * (float)M_PI / 180.0f,
            eulerDegrees.z * (float)M_PI / 180.0f};

        float cx = std::cos(r.x * 0.5f);
        float sx = std::sin(r.x * 0.5f);
        float cy = std::cos(r.y * 0.5f);
        float sy = std::sin(r.y * 0.5f);
        float cz = std::cos(r.z * 0.5f);
        float sz = std::sin(r.z * 0.5f);

        // Pitch (X), Yaw (Y), Roll (Z)
        Quat q;
        q.w = cx * cy * cz + sx * sy * sz;
        q.x = sx * cy * cz - cx * sy * sz;
        q.y = cx * sy * cz + sx * cy * sz;
        q.z = cx * cy * sz - sx * sy * cz;

        return q.Normalized();
    }
};

inline Vec3 operator*(const Quat &q, const Vec3 &v)
{
    Vec3 u{q.x, q.y, q.z};
    float s = q.w;

    return u * (2.0f * Vec3::Dot(u, v)) +
           v * (s * s - Vec3::Dot(u, u)) +
           Vec3::Cross(u, v) * (2.0f * s);
}
