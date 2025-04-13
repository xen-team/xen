#include "swapchain.hpp"
#include "system/window.hpp"

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include <utility>

namespace xen {
Swapchain::Swapchain(std::shared_ptr<Window> window, Vector2ui size) : window(std::move(window)), size(std::move(size))
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<int>(size.x), static_cast<int>(size.y));

    textures.resize(2);
    for (GLuint i = 0; i < 2; i++) {
        glGenTextures(1, &textures[i]);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, static_cast<int>(size.x), static_cast<int>(size.y), 0, GL_RGBA, GL_UNSIGNED_BYTE,
            nullptr
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        // Handle error
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Swapchain::acquire_next_image()
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void Swapchain::queue_present()
{
    glfwSwapBuffers(window->get_window());
}
}