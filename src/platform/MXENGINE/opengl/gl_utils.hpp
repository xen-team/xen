#pragma once

#if defined(GLEW_BUILD)
#undef GLEW_BUILD
#endif

namespace xen {
#if defined(XEN_DEBUG)
#define GL_CALL(x)                                                                                                     \
    gl_clear_errors();                                                                                                 \
    x;                                                                                                                 \
    gl_log_call(#x, __FILE__, __LINE__)
#define PUSH_DEBUG_GROUP(name) gl_push_debug_group(name)
#define POP_DEBUG_GROUP() gl_pop_debug_group()
#else
#define GL_CALL(x) x
#define PUSH_DEBUG_GROUP(name)
#define POP_DEBUG_GROUP()
#endif

void gl_clear_errors();

bool gl_log_call(char const* function, char const* file, int line);

void gl_push_debug_group(char const* name);
void gl_pop_debug_group();

template <typename T>
char const* type_to_string();

template <typename T>
unsigned int get_gl_type();

void APIENTRY print_debug_info(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message,
    void const* user_param
);
}