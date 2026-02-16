#pragma once
#include <utils.h>
#include <algorithm>

class Particle
{
public:
    vec2 position;
    vec2 velocity;
    vec4 color;
    float lifespan;

    bool active = true;      // Toggle active
    bool useGravity = false; // Toggle gravity per particle

    void Update(float deltaTime)
    {
        if (!active)
            return;

        if (useGravity)
        {
            float gravity = 800.0f; // You can tweak this
            velocity.y += gravity * deltaTime;
        }

        position += velocity * deltaTime;
        lifespan -= deltaTime;

        if (lifespan <= 0.0f)
            active = false;
    }
};

class ParticleSystem
{
public:
    Array<Particle> particles;
    bool active = true; // Entire system toggle

    void emit(const Particle &particle)
    {
        if (!active)
            return;
        particles.push_back(particle);
    }

    void Update(float deltaTime)
    {
        if (!active)
            return;

        for (auto &particle : particles)
            particle.Update(deltaTime);

        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                           [](const Particle &p)
                           {
                               return !p.active;
                           }),
            particles.end());
    }
};
