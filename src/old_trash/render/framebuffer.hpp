#pragma once
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../utils/math.hpp"

namespace render {

class FrameBuffer
{
public:
	FrameBuffer();
	~FrameBuffer();

	uint32_t get_texture() const;

	void rescale(br_size new_size);
	void bind() const;
	void unbind() const;

	br_size get_size() const {
		return size;
	};
private:
    br_size size{1.f, 1.f};
	uint32_t fbo;
	uint32_t texture;
	uint32_t rbo;
};
}