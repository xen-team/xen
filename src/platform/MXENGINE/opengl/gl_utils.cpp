#include "gl_utils.hpp"
#include "debug/log.hpp"

#include <fstream>
#include <string>
#include <set>
#include <sstream>

namespace xen {
static std::set<int> existing_errors = {131154};

void gl_clear_errors()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

void gl_push_debug_group(char const* name)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, -1, -1, name);
}

void gl_pop_debug_group()
{
    glPopDebugGroup();
}

bool gl_log_call(char const* function, char const* file, int line)
{
    bool success = true;
    while (GLenum error = glGetError()) {
        success = false;
        if (existing_errors.find(error) != existing_errors.end())
            continue;
        existing_errors.insert(error);
        setlocale(LC_ALL, "");

        Log::error("OpenGL::ErrorHandler ", "error #", error, " ", function, " in file: ", file, ", line: ", line);
    }
    return success;
}

void print_debug_info(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message,
    void const* user_param
)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    if (existing_errors.find(id) != existing_errors.end())
        return;
    existing_errors.insert(id);

    std::stringstream out;
    out << "device message [errcode " << id << "]: " << message << "\n        ";

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        out << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        out << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        out << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        out << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        out << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        out << "Source: Other";
        break;
    }
    out << "\n        ";

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        out << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        out << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        out << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        out << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        out << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        out << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        out << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        out << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        out << "Type: Other";
        break;
    }
    out << "\n        ";

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        out << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        out << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        out << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        out << "Severity: notification";
        break;
    }

    Log::error("OpenGL::ErrorHandler ", out.str());
}

template <>
char const* type_to_string<unsigned char>()
{
    return "ubyte";
}

template <>
char const* type_to_string<unsigned int>()
{
    return "uint";
}

template <>
char const* type_to_string<float>()
{
    return "float";
}

template <>
unsigned int get_gl_type<float>()
{
    return GL_FLOAT;
}

template <>
unsigned int get_gl_type<unsigned int>()
{
    return GL_UNSIGNED_INT;
}

template <>
unsigned int get_gl_type<unsigned char>()
{
    return GL_UNSIGNED_BYTE;
}
}