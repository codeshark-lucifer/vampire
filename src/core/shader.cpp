#include <shader.h>
#include <cstring>

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
Shader::Shader(const str& vertPath, const str& fragPath)
{
    id = 0;

    int fileSize = 0;

    char* vertSource = read_file(vertPath, &fileSize);
    char* fragSource = read_file(fragPath, &fileSize);

    if (!vertSource || !fragSource)
    {
        LOG_ERROR("Failed to read shader files.");
        return;
    }

    u32 vertex = Compile(GL_VERTEX_SHADER, vertSource);
    if (!CheckShader(vertex, vertPath))
    {
        glDeleteShader(vertex);
        return;
    }

    u32 fragment = Compile(GL_FRAGMENT_SHADER, fragSource);
    if (!CheckShader(fragment, fragPath))
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return;
    }

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (!CheckProgram())
    {
        glDeleteProgram(id);
        id = 0;
        return;
    }
}

// ------------------------------------------------------------
// Destructor
// ------------------------------------------------------------
Shader::~Shader()
{
    if (id)
        glDeleteProgram(id);
}

// ------------------------------------------------------------
// Use Program
// ------------------------------------------------------------
void Shader::Use()
{
    glUseProgram(id);
}

// ------------------------------------------------------------
// Shader Compilation
// ------------------------------------------------------------
u32 Shader::Compile(GLenum type, const char* source)
{
    if (!source || strlen(source) == 0)
    {
        LOG_ERROR("Shader source is empty!");
        return 0;
    }

    u32 shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    return shader;
}

// ------------------------------------------------------------
// Check Shader Compile
// ------------------------------------------------------------
bool Shader::CheckShader(u32 shader, str name)
{
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[4096];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOG_ERROR("Failed to compile shader (%s):\n%s", name, log);
        return false;
    }

    return true;
}

// ------------------------------------------------------------
// Check Program Link
// ------------------------------------------------------------
bool Shader::CheckProgram()
{
    GLint success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[4096];
        glGetProgramInfoLog(id, sizeof(log), nullptr, log);
        LOG_ERROR("Failed to link shader program:\n%s", log);
        return false;
    }

    return true;
}

// ------------------------------------------------------------
// Uniform Location
// ------------------------------------------------------------
u32 Shader::GetUniform(const char* name)
{
    return glGetUniformLocation(id, name);
}

// ------------------------------------------------------------
// Template Specializations
// ------------------------------------------------------------
template <>
void Shader::SetUniform<float>(const char* name, float value)
{
    glUniform1f(GetUniform(name), value);
}

template <>
void Shader::SetUniform<int>(const char* name, int value)
{
    glUniform1i(GetUniform(name), value);
}

template <>
void Shader::SetUniform<bool>(const char* name, bool value)
{
    glUniform1i(GetUniform(name), value ? 1 : 0);
}

template <>
void Shader::SetUniform<Vec2>(const char* name, Vec2 value)
{
    glUniform2f(GetUniform(name), value.x, value.y);
}

template <>
void Shader::SetUniform<Vec3>(const char* name, Vec3 value)
{
    glUniform3f(GetUniform(name), value.x, value.y, value.z);
}

template <>
void Shader::SetUniform<Vec4>(const char* name, Vec4 value)
{
    glUniform4f(GetUniform(name), value.x, value.y, value.z, value.w);
}

template <>
void Shader::SetUniform<Mat3>(const char* name, Mat3 value)
{
    glUniformMatrix3fv(GetUniform(name), 1, GL_FALSE, value.m);
}

template <>
void Shader::SetUniform<Mat4>(const char* name, Mat4 value)
{
    glUniformMatrix4fv(GetUniform(name), 1, GL_FALSE, value.m);
}
