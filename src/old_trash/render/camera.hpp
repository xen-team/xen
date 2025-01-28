#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class OrthographicCamera {
public:
    glm::mat4 GetView() const {
        return glm::translate(glm::mat4(1.0f), -pos);
    }

    glm::mat4 GetProjection() const {
        return glm::ortho(left, right, bottom, top, near, far);
    }

// private:
    glm::vec3 pos{0.f, 0.f, 0.0f};

    float left = -5.f;
    float right = 5.f;
    float bottom = -5.f;
    float top = 5.f;
    float near = 0.1f;
    float far = 100.f;

};

class PerspectiveCamera {
public:
    PerspectiveCamera() {
        update();
    }

    glm::mat4 GetView() const {
        return glm::lookAt(pos, pos + direction, up);
    }

    glm::mat4 GetProjection() const {
        return glm::perspective(glm::radians(fov), aspect_ratio, near, far);
    }

    glm::vec3 right() const {
        return glm::cross(direction, up);
    }

    void update() {
        glm::vec3 dir_tmp;
        dir_tmp.x = cos(glm::radians(euler[1])) * cos(glm::radians(euler[0]));
        dir_tmp.y = sin(glm::radians(euler[0]));
        dir_tmp.z = sin(glm::radians(euler[1])) * cos(glm::radians(euler[0]));
        direction = glm::normalize(dir_tmp);

        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(euler[2]), direction);
        up = glm::vec3(rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    }

// private:
    glm::vec3 pos{0.f, 0.f, 3.0f};
    glm::vec3 direction{0.f, 0.f, -1.f};
    glm::vec3 up{0.f, 1.f, 0.f};
    glm::vec3 euler{0.f, -90.f, 0.f};

    float fov = 45.f;
    float near = 0.1f;
    float far = 100.f;
    float aspect_ratio = 1920.f/1080.f;

};