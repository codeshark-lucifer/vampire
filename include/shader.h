#pragma once
#include <glad/glad.h>
#include <utils.h>

class Shader
{
public:
    Shader(const str &vert, const str &frag);
    ~Shader();

    void Use();

    template <typename T>
    void SetUniform(const char *name, T value);

private:
    u32 id = 0;
    u32 Compile(GLenum shaderType, const char *source);
    u32 GetUniform(const char *name);

    bool CheckShader(u32 shader, str name);
    bool CheckProgram();
};

// Explicit template specializations declarations (at global namespace scope)
template <> void Shader::SetUniform<float>(const char *name, float value);
template <> void Shader::SetUniform<int>(const char *name, int value);
template <> void Shader::SetUniform<bool>(const char *name, bool value);
template <> void Shader::SetUniform<vec2>(const char *name, vec2 value);
template <> void Shader::SetUniform<vec3>(const char *name, vec3 value);
template <> void Shader::SetUniform<vec4>(const char *name, vec4 value);
template <> void Shader::SetUniform<mat3>(const char *name, mat3 value);
template <> void Shader::SetUniform<mat4>(const char *name, mat4 value);
