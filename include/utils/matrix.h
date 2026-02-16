#pragma once
#include <cstring>
#include <cmath>
#include "vector.h"
#include "Quat.h"

struct Mat3; // forward declaration

struct Mat4
{
    float m[16];

    Mat4()
    {
        std::memset(m, 0, sizeof(m));
    }

    explicit Mat4(float v)
    {
        for (int i = 0; i < 16; i++)
            m[i] = v;
    }

    // Construct Mat4 from Mat3 (upper-left 3x3), rest identity
    Mat4(const Mat3 &Mat3_);

    // Construct Mat4 from Mat3 + translation Vec3
    Mat4(const Mat3 &Mat3_, const Vec3 &t);

    // Copy constructor
    Mat4(const Mat4 &other)
    {
        for (int i = 0; i < 16; i++)
            m[i] = other.m[i];
    }
    static Mat4 Identity()
    {
        Mat4 r;
        r.m[0] = 1.0f;
        r.m[5] = 1.0f;
        r.m[10] = 1.0f;
        r.m[15] = 1.0f;
        return r;
    }

    static Mat4 Translate(const Vec3 &t)
    {
        Mat4 r = Identity();
        r.m[12] = t.x;
        r.m[13] = t.y;
        r.m[14] = t.z;
        return r;
    }

    static Mat4 Scale(const Vec3 &s)
    {
        Mat4 r = Identity();
        r.m[0] = s.x;
        r.m[5] = s.y;
        r.m[10] = s.z;
        return r;
    }

    static Mat4 Rotate(const Quat &q)
    {
        Quat n = q.Normalized();
        Mat4 r = Identity();

        float xx = n.x * n.x;
        float yy = n.y * n.y;
        float zz = n.z * n.z;
        float xy = n.x * n.y;
        float xz = n.x * n.z;
        float yz = n.y * n.z;
        float wx = n.w * n.x;
        float wy = n.w * n.y;
        float wz = n.w * n.z;

        r.m[0] = 1 - 2 * (yy + zz);
        r.m[1] = 2 * (xy + wz);
        r.m[2] = 2 * (xz - wy);

        r.m[4] = 2 * (xy - wz);
        r.m[5] = 1 - 2 * (xx + zz);
        r.m[6] = 2 * (yz + wx);

        r.m[8] = 2 * (xz + wy);
        r.m[9] = 2 * (yz - wx);
        r.m[10] = 1 - 2 * (xx + yy);

        return r;
    }

    Mat4 operator*(const Mat4 &o) const
    {
        Mat4 r;
        for (int c = 0; c < 4; c++)
        {
            for (int r0 = 0; r0 < 4; r0++)
            {
                r.m[c * 4 + r0] =
                    m[0 * 4 + r0] * o.m[c * 4 + 0] +
                    m[1 * 4 + r0] * o.m[c * 4 + 1] +
                    m[2 * 4 + r0] * o.m[c * 4 + 2] +
                    m[3 * 4 + r0] * o.m[c * 4 + 3];
            }
        }
        return r;
    }

    static Mat4 Perspective(float fovRadians, float aspect, float nearPlane, float farPlane)
    {
        Mat4 r;

        float tanHalfFov = std::tan(fovRadians * 0.5f);

        r.m[0] = 1.0f / (aspect * tanHalfFov);
        r.m[5] = 1.0f / tanHalfFov;
        r.m[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        r.m[11] = -1.0f;
        r.m[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);

        return r;
    }

    static Mat4 Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
    {
        Mat4 r = Mat4::Identity();

        r.m[0] = 2.0f / (right - left);
        r.m[5] = 2.0f / (top - bottom);
        r.m[10] = -2.0f / (farPlane - nearPlane);

        r.m[12] = -(right + left) / (right - left);
        r.m[13] = -(top + bottom) / (top - bottom);
        r.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);

        return r;
    }

    static Mat4 LookAt(const Vec3 &eye, const Vec3 &center, const Vec3 &up)
    {
        Vec3 f = (center - eye).Normalized();     // forward
        Vec3 s = Vec3::Cross(f, up).Normalized(); // right
        Vec3 u = Vec3::Cross(s, f);               // up

        Mat4 r = Mat4::Identity();

        r.m[0] = s.x;
        r.m[1] = u.x;
        r.m[2] = -f.x;

        r.m[4] = s.y;
        r.m[5] = u.y;
        r.m[6] = -f.y;

        r.m[8] = s.z;
        r.m[9] = u.z;
        r.m[10] = -f.z;

        r.m[12] = -Vec3::Dot(s, eye);
        r.m[13] = -Vec3::Dot(u, eye);
        r.m[14] = Vec3::Dot(f, eye);

        return r;
    }
};

struct Mat3
{
    // Column-major (OpenGL style)
    float m[9];

    Mat3()
    {
        std::memset(m, 0, sizeof(m));
    }

    static Mat3 Identity()
    {
        Mat3 r;
        r.m[0] = 1.0f;
        r.m[4] = 1.0f;
        r.m[8] = 1.0f;
        return r;
    }

    static Mat3 Translate(const Vec2 &t)
    {
        Mat3 r = Identity();
        r.m[6] = t.x;
        r.m[7] = t.y;
        return r;
    }

    static Mat3 Scale(const Vec2 &s)
    {
        Mat3 r = Identity();
        r.m[0] = s.x;
        r.m[4] = s.y;
        return r;
    }

    static Mat3 Rotate(float radians)
    {
        Mat3 r = Identity();
        float c = std::cos(radians);
        float s = std::sin(radians);

        r.m[0] = c;
        r.m[1] = s;
        r.m[3] = -s;
        r.m[4] = c;

        return r;
    }

    Vec2 MultiplyPoint(const Vec2 &v) const
    {
        return {
            m[0] * v.x + m[3] * v.y + m[6],
            m[1] * v.x + m[4] * v.y + m[7]};
    }

    Mat3 operator*(const Mat3 &o) const
    {
        Mat3 r;
        for (int c = 0; c < 3; c++)
        {
            for (int r0 = 0; r0 < 3; r0++)
            {
                r.m[c * 3 + r0] =
                    m[0 * 3 + r0] * o.m[c * 3 + 0] +
                    m[1 * 3 + r0] * o.m[c * 3 + 1] +
                    m[2 * 3 + r0] * o.m[c * 3 + 2];
            }
        }
        return r;
    }


    inline Mat3(const Mat4 &Mat4_)
    {
        m[0] = Mat4_.m[0];
        m[1] = Mat4_.m[1];
        m[2] = Mat4_.m[2];

        m[3] = Mat4_.m[4];
        m[4] = Mat4_.m[5];
        m[5] = Mat4_.m[6];

        m[6] = Mat4_.m[8];
        m[7] = Mat4_.m[9];
        m[8] = Mat4_.m[10];
    };
};

// Out-of-line definitions for Mat4 constructors
inline Mat4::Mat4(const Mat3 &Mat3_)
{
    m[0] = Mat3_.m[0];
    m[1] = Mat3_.m[1];
    m[2] = Mat3_.m[2];
    m[3] = 0.0f;
    m[4] = Mat3_.m[3];
    m[5] = Mat3_.m[4];
    m[6] = Mat3_.m[5];
    m[7] = 0.0f;
    m[8] = Mat3_.m[6];
    m[9] = Mat3_.m[7];
    m[10] = Mat3_.m[8];
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

inline Mat4::Mat4(const Mat3 &Mat3_, const Vec3 &t)
{
    m[0] = Mat3_.m[0];
    m[1] = Mat3_.m[1];
    m[2] = Mat3_.m[2];
    m[3] = 0.0f;
    m[4] = Mat3_.m[3];
    m[5] = Mat3_.m[4];
    m[6] = Mat3_.m[5];
    m[7] = 0.0f;
    m[8] = Mat3_.m[6];
    m[9] = Mat3_.m[7];
    m[10] = Mat3_.m[8];
    m[11] = 0.0f;
    m[12] = t.x;
    m[13] = t.y;
    m[14] = t.z;
    m[15] = 1.0f;
}


