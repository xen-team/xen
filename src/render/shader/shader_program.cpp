#include "shader_program.hpp"

#include <render/renderer.hpp>

#include <tracy/Tracy.hpp>

namespace xen {
namespace {
inline void check_program_used([[maybe_unused]] ShaderProgram const& program)
{
#if defined(XEN_CONFIG_DEBUG)
    if (!program.is_used()) {
        Log::error("The current shader program must be defined as used before sending uniforms to it.");
    }
#endif
}

ImageInternalFormat recover_image_texture_format(Texture const& texture)
{
    TextureColorspace const colorspace = texture.get_colorspace();
    TextureDataType const data_type = texture.get_data_type();

    switch (colorspace) {
    case TextureColorspace::GRAY:
        if (data_type == TextureDataType::FLOAT32)
            return ImageInternalFormat::R32F;

#if !defined(USE_OPENGL_ES)
        return (data_type == TextureDataType::BYTE ? ImageInternalFormat::R8 : ImageInternalFormat::R16F);
#else
        break;
#endif

#if !defined(USE_OPENGL_ES)
    case TextureColorspace::RG:
        return (
            data_type == TextureDataType::BYTE ?
                ImageInternalFormat::RG8 :
                (data_type == TextureDataType::FLOAT16 ? ImageInternalFormat::RG16F : ImageInternalFormat::RG32F)
        );
#endif

    case TextureColorspace::RGB:
    case TextureColorspace::RGBA:
    case TextureColorspace::SRGB:
    case TextureColorspace::SRGBA:
        return (
            data_type == TextureDataType::BYTE ?
                ImageInternalFormat::RGBA8 :
                (data_type == TextureDataType::FLOAT16 ? ImageInternalFormat::RGBA16F : ImageInternalFormat::RGBA32F)
        );

    default:
        break;
    }

    throw std::invalid_argument("[ShaderProgram] The given image texture is not supported");
}
}

ShaderProgram::ShaderProgram() : index{Renderer::create_program()} {}

bool ShaderProgram::has_attribute(std::string const& uniform_name) const
{
    return (attributes.find(uniform_name) != attributes.cend());
}

bool ShaderProgram::has_texture(Texture const& texture) const
{
    return std::any_of(textures.cbegin(), textures.cend(), [&texture](auto const& element) {
        return (element.first->get_index() == texture.get_index());
    });
}

bool ShaderProgram::has_texture(std::string const& uniform_name) const
{
    return std::any_of(textures.cbegin(), textures.cend(), [&uniform_name](auto const& element) {
        return (element.second == uniform_name);
    });
}

Texture const& ShaderProgram::get_texture(std::string const& uniform_name) const
{
    auto const texture_it = std::find_if(textures.begin(), textures.end(), [&uniform_name](auto const& element) {
        return (element.second == uniform_name);
    });

    if (texture_it == textures.cend()) {
        throw std::invalid_argument("[ShaderProgram] The given attribute uniform name does not exist");
    }

    return *texture_it->first;
}

#if !defined(USE_WEBGL)
bool ShaderProgram::has_image_texture(const Texture& texture) const
{
    return std::any_of(image_textures.cbegin(), image_textures.cend(), [&texture](auto const& element) {
        return (element.first->get_index() == texture.get_index());
    });
}

bool ShaderProgram::has_image_texture(std::string const& uniform_name) const
{
    return std::any_of(image_textures.cbegin(), image_textures.cend(), [&uniform_name](auto const& element) {
        return (element.second.uniform_name == uniform_name);
    });
}

Texture const& ShaderProgram::get_image_texture(std::string const& uniform_name) const
{
    auto const texture_it =
        std::find_if(image_textures.begin(), image_textures.end(), [&uniform_name](auto const& element) {
            return (element.second.uniform_name == uniform_name);
        });

    if (texture_it == image_textures.cend()) {
        throw std::invalid_argument("[ShaderProgram] The given attribute uniform name does not exist");
    }

    return *texture_it->first;
}
#endif

void ShaderProgram::set_texture(TexturePtr texture, const std::string& uniform_name)
{
    auto const texture_it = std::find_if(textures.begin(), textures.end(), [&uniform_name](auto const& element) {
        return (element.second == uniform_name);
    });

    if (texture_it != textures.end()) {
        texture_it->first = std::move(texture);
    }
    else {
        textures.emplace_back(std::move(texture), uniform_name);
    }
}

#if !defined(USE_WEBGL)
void ShaderProgram::set_image_texture(TexturePtr texture, const std::string& uniform_name, ImageTextureUsage usage)
{
    if (
#if !defined(USE_OPENGL_ES)
        !Renderer::check_version(4, 2)
#else
        !Renderer::check_version(3, 1)
#endif
    ) {
        throw std::runtime_error("[ShaderProgram] Using image textures requires OpenGL 4.2+ or OpenGL ES 3.1+");
    }

    if (texture->get_colorspace() == TextureColorspace::INVALID ||
        texture->get_colorspace() == TextureColorspace::DEPTH)
        throw std::invalid_argument("[ShaderProgram] The given image texture's colorspace is invalid");

    if (texture->get_colorspace() == TextureColorspace::SRGB || texture->get_colorspace() == TextureColorspace::SRGBA) {
        // See: https://www.khronos.org/opengl/wiki/Image_Load_Store#Format_compatibility
        throw std::invalid_argument(
            "[ShaderProgram] Textures with an sRGB(A) colorspace cannot be used as image textures"
        );
    }

    auto image_texture_it =
        std::find_if(image_textures.begin(), image_textures.end(), [&uniform_name](auto const& element) {
            return (element.second.uniform_name == uniform_name);
        });

    if (image_texture_it != image_textures.end()) {
        image_texture_it->first = std::move(texture);
    }
    else {
        image_textures.emplace_back(std::move(texture), ImageTextureAttachment{uniform_name});
        image_texture_it = image_textures.end() - 1;
    }

    image_texture_it->second.access =
        (usage == ImageTextureUsage::READ ?
             ImageAccess::READ :
             (usage == ImageTextureUsage::WRITE ? ImageAccess::WRITE : ImageAccess::READ_WRITE));
    image_texture_it->second.format = recover_image_texture_format(*image_texture_it->first);
}
#endif

void ShaderProgram::link()
{
    ZoneScopedN("ShaderProgram::link");

    Log::debug("[ShaderProgram] Linking (ID: " + std::to_string(index) + ")...");

    Renderer::link_program(index);
    update_attributes_locations();

    Log::debug("[ShaderProgram] Linked");
}

bool ShaderProgram::is_linked() const
{
    return Renderer::is_program_linked(index);
}

void ShaderProgram::update_shaders()
{
    ZoneScopedN("ShaderProgram::update_shaders");

    Log::debug("[ShaderProgram] Updating shaders...");

    load_shaders();
    compile_shaders();
    link();
    send_attributes();
    init_textures();
#if !defined(USE_WEBGL)
    init_image_textures();
#endif

    Log::debug("[ShaderProgram] Updated shaders");
}

void ShaderProgram::use() const
{
    Renderer::use_program(index);
}

bool ShaderProgram::is_used() const
{
    return (Renderer::get_current_program() == index);
}

void ShaderProgram::send_attributes() const
{
    ZoneScopedN("ShaderProgram::send_attributes");

    if (attributes.empty()) {
        return;
    }

    use();

    for (auto const& [name, attrib] : attributes) {
        if (attrib.location == -1) {
            continue;
        }

        std::visit(
            [this, location = attrib.location](auto const& value) { send_uniform(location, value); }, attrib.value
        );
    }
}

void ShaderProgram::remove_attribute(std::string const& uniform_name)
{
    auto const attrib_it = attributes.find(uniform_name);

    if (attrib_it == attributes.end()) {
        throw std::invalid_argument("[ShaderProgram] The given attribute uniform name does not exist");
    }

    attributes.erase(attrib_it);
}

void ShaderProgram::init_textures() const
{
    ZoneScopedN("ShaderProgram::init_textures");

    if (textures.empty()) {
        return;
    }

    use();

    // TODO: binding indices should be user-definable to allow the same texture to be bound to multiple uniforms
    int binding_index = 0;

    for (auto const& [texture, name] : textures) {
        send_uniform(name, binding_index++);
    }
}

void ShaderProgram::bind_textures() const
{
    ZoneScopedN("ShaderProgram::bind_textures");

    use();

    uint texture_index = 0;

    for (auto const& [texture, _] : textures) {
        Renderer::activate_texture(texture_index++);
        texture->bind();
    }
}

void ShaderProgram::remove_texture(Texture const& texture)
{
    textures.erase(
        std::remove_if(
            textures.begin(), textures.end(),
            [&texture](auto const& element) { return (element.first->get_index() == texture.get_index()); }
        ),
        textures.end()
    );
}

void ShaderProgram::remove_texture(std::string const& uniform_name)
{
    for (auto texture_it = textures.begin(); texture_it != textures.end(); ++texture_it) {
        if (texture_it->second != uniform_name) {
            continue;
        }

        textures.erase(texture_it);
        return;
    }
}

#if !defined(USE_WEBGL)
void ShaderProgram::init_image_textures() const
{
    ZoneScopedN("ShaderProgram::init_image_textures");

    if (image_textures.empty()) {
        return;
    }

    use();

    // TODO: binding indices should be user-definable to allow the same texture to be bound to multiple uniforms
    int binding_index = 0;

    for (auto const& [texture, info] : image_textures) {
        send_uniform(info.uniform_name, binding_index++);
    }
}

void ShaderProgram::bind_image_textures() const
{
    ZoneScopedN("ShaderProgram::bind_image_textures");

    use();

    uint binding_index = 0;

    for (auto const& [texture, info] : image_textures) {
        Renderer::bind_image_texture(binding_index++, texture->get_index(), 0, false, 0, info.access, info.format);
    }
}

void ShaderProgram::remove_image_texture(Texture const& texture)
{
    image_textures.erase(
        std::remove_if(
            image_textures.begin(), image_textures.end(),
            [&texture](auto const& element) { return (element.first->get_index() == texture.get_index()); }
        ),
        image_textures.end()
    );
}

void ShaderProgram::remove_image_texture(std::string const& uniform_name)
{
    for (auto image_texture_it = image_textures.begin(); image_texture_it != image_textures.end(); ++image_texture_it) {
        if (image_texture_it->second.uniform_name != uniform_name) {
            continue;
        }

        image_textures.erase(image_texture_it);
        return;
    }
}
#endif

int ShaderProgram::recover_uniform_location(const std::string& uniform_name) const
{
    return Renderer::recover_uniform_location(index, uniform_name.c_str());
}

void ShaderProgram::send_uniform(int index, int value) const
{
    check_program_used(*this);
    Renderer::send_uniform(index, value);
}

void ShaderProgram::send_uniform(int index, uint value) const
{
    check_program_used(*this);
    Renderer::send_uniform(index, value);
}

void ShaderProgram::send_uniform(int index, float value) const
{
    check_program_used(*this);
    Renderer::send_uniform(index, value);
}

void ShaderProgram::send_uniform(int index, int const* values, size_t count) const
{
    check_program_used(*this);
    Renderer::send_uniform_vector1fi(index, values, static_cast<int>(count));
}

void ShaderProgram::send_uniform(int index, uint const* values, size_t count) const
{
    check_program_used(*this);
    Renderer::send_uniform_vector1fui(index, values, static_cast<int>(count));
}

void ShaderProgram::send_uniform(int index, float const* values, size_t count) const
{
    check_program_used(*this);
    Renderer::send_uniform_vector1f(index, values, static_cast<int>(count));
}

void ShaderProgram::send_uniform(int index, Vector2i const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector2i(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Vector3i const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector3i(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Vector4i const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector4i(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Vector2ui const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector2ui(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Vector3ui const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector3ui(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Vector4ui const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector4ui(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Vector2f const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector2f(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Vector3f const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector3f(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Vector4f const& vec) const
{
    check_program_used(*this);
    Renderer::send_uniform_Vector4f(index, &vec[0]);
}

void ShaderProgram::send_uniform(int index, Matrix2 const& mat) const
{
    check_program_used(*this);
    Renderer::send_uniform_Matrix2(index, &mat[0][0]);
}

void ShaderProgram::send_uniform(int index, Matrix3 const& mat) const
{
    check_program_used(*this);
    Renderer::send_uniform_Matrix3(index, &mat[0][0]);
}

void ShaderProgram::send_uniform(int index, Matrix4 const& mat) const
{
    check_program_used(*this);
    Renderer::send_uniform_Matrix4(index, &mat[0][0]);
}

void ShaderProgram::send_uniform(int index, Color const& color) const
{
    check_program_used(*this);
    Renderer::send_uniform_color(index, &color.r);
}

ShaderProgram::~ShaderProgram()
{
    ZoneScopedN("ShaderProgram::~ShaderProgram");

    if (!index.is_valid()) {
        return;
    }

    Log::debug("[ShaderProgram] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::delete_program(index);
    Log::debug("[ShaderProgram] Destroyed");
}

void ShaderProgram::update_attributes_locations()
{
    ZoneScopedN("ShaderProgram::update_attributes_locations");

    for (auto& [name, attrib] : attributes) {
        attrib.location = recover_uniform_location(name);
    }
}

void RenderShaderProgram::set_vertex_shader(VertexShader&& vert_shader)
{
    Log::debug(
        "[RenderShaderProgram] Setting vertex shader (ID: " + std::to_string(vert_shader.get_index()) +
        ", path: " + vert_shader.get_path() + ")"
    );

    if (Renderer::is_shader_attached(index, this->vert_shader.get_index())) {
        Renderer::detach_shader(index, this->vert_shader.get_index());
    }

    this->vert_shader = std::move(vert_shader);
    this->vert_shader.compile();

    Renderer::attach_shader(index, this->vert_shader.get_index());
}

#if !defined(USE_OPENGL_ES)
void RenderShaderProgram::set_tessellation_control_shader(TessellationControlShader&& tess_ctrl_shader)
{
    Log::debug(
        "[RenderShaderProgram] Setting tessellation control shader (ID: " +
        std::to_string(tess_ctrl_shader.get_index()) + ", path: " + tess_ctrl_shader.get_path() + ")"
    );

    if (this->tess_ctrl_shader && Renderer::is_shader_attached(index, this->tess_ctrl_shader->get_index())) {
        Renderer::detach_shader(index, this->tess_ctrl_shader->get_index());
    }

    this->tess_ctrl_shader = std::move(tess_ctrl_shader);
    this->tess_ctrl_shader->compile();

    Renderer::attach_shader(index, this->tess_ctrl_shader->get_index());
}

void RenderShaderProgram::set_tessellation_evaluation_shader(TessellationEvaluationShader&& tess_eval_shader)
{
    Log::debug(
        "[RenderShaderProgram] Setting tessellation evaluation shader (ID: " +
        std::to_string(tess_eval_shader.get_index()) + ", path: " + tess_eval_shader.get_path() + ")"
    );

    if (this->tess_eval_shader && Renderer::is_shader_attached(index, this->tess_eval_shader->get_index())) {
        Renderer::detach_shader(index, this->tess_eval_shader->get_index());
    }

    this->tess_eval_shader = std::move(tess_eval_shader);
    this->tess_eval_shader->compile();

    Renderer::attach_shader(index, this->tess_eval_shader->get_index());
}

void RenderShaderProgram::set_geometry_shader(GeometryShader&& geom_shader)
{
    Log::debug(
        "[RenderShaderProgram] Setting geometry control shader (ID: " + std::to_string(geom_shader.get_index()) +
        ", path: " + geom_shader.get_path() + ")"
    );

    if (this->geom_shader && Renderer::is_shader_attached(index, this->geom_shader->get_index())) {
        Renderer::detach_shader(index, this->geom_shader->get_index());
    }

    this->geom_shader = std::move(geom_shader);
    this->geom_shader->compile();

    Renderer::attach_shader(index, this->geom_shader->get_index());
}
#endif

void RenderShaderProgram::set_fragment_shader(FragmentShader&& frag_shader)
{
    Log::debug(
        "[RenderShaderProgram] Setting fragment control shader (ID: " + std::to_string(frag_shader.get_index()) +
        ", path: " + frag_shader.get_path() + ")"
    );

    if (Renderer::is_shader_attached(index, this->frag_shader.get_index())) {
        Renderer::detach_shader(index, this->frag_shader.get_index());
    }

    this->frag_shader = std::move(frag_shader);
    this->frag_shader.compile();

    Renderer::attach_shader(index, this->frag_shader.get_index());
}

void RenderShaderProgram::set_shaders(VertexShader&& vert_shader, FragmentShader&& frag_shader)
{
    set_vertex_shader(std::move(vert_shader));
    set_fragment_shader(std::move(frag_shader));

    link();
}

#if !defined(USE_OPENGL_ES)
void RenderShaderProgram::set_shaders(
    VertexShader&& vert_shader, GeometryShader&& geom_shader, FragmentShader&& frag_shader
)
{
    set_vertex_shader(std::move(vert_shader));
    set_geometry_shader(std::move(geom_shader));
    set_fragment_shader(std::move(frag_shader));

    link();
}

void RenderShaderProgram::set_shaders(
    VertexShader&& vert_shader, TessellationEvaluationShader&& tess_eval_shader, FragmentShader&& frag_shader
)
{
    set_vertex_shader(std::move(vert_shader));
    set_tessellation_evaluation_shader(std::move(tess_eval_shader));
    set_fragment_shader(std::move(frag_shader));

    link();
}

void RenderShaderProgram::set_shaders(
    VertexShader&& vert_shader, TessellationControlShader&& tess_ctrl_shader,
    TessellationEvaluationShader&& tess_eval_shader, FragmentShader&& frag_shader
)
{
    set_vertex_shader(std::move(vert_shader));
    set_tessellation_control_shader(std::move(tess_ctrl_shader));
    set_tessellation_evaluation_shader(std::move(tess_eval_shader));
    set_fragment_shader(std::move(frag_shader));

    link();
}
#endif

RenderShaderProgram RenderShaderProgram::clone() const
{
    RenderShaderProgram program;

    program.set_vertex_shader(vert_shader.clone());
#if !defined(USE_OPENGL_ES)
    if (tess_ctrl_shader) {
        program.set_tessellation_control_shader(tess_ctrl_shader->clone());
    }
    if (tess_eval_shader) {
        program.set_tessellation_evaluation_shader(tess_eval_shader->clone());
    }
    if (geom_shader) {
        program.set_geometry_shader(geom_shader->clone());
    }
#endif
    program.set_fragment_shader(frag_shader.clone());

    program.link();

    program.attributes = attributes;
    program.textures = textures;
#if !defined(USE_WEBGL)
    program.image_textures = image_textures;
#endif

    send_attributes();
    init_textures();
#if !defined(USE_WEBGL)
    init_image_textures();
#endif

    return program;
}

void RenderShaderProgram::load_shaders() const
{
    ZoneScopedN("RenderShaderProgram::load_shaders");

    Log::debug("[RenderShaderProgram] Loading shaders...");

    vert_shader.load();
#if !defined(USE_OPENGL_ES)
    if (tess_ctrl_shader) {
        tess_ctrl_shader->load();
    }
    if (tess_eval_shader) {
        tess_eval_shader->load();
    }
    if (geom_shader) {
        geom_shader->load();
    }
#endif
    frag_shader.load();

    Log::debug("[RenderShaderProgram] Loaded shaders");
}

void RenderShaderProgram::compile_shaders() const
{
    ZoneScopedN("RenderShaderProgram::compile_shaders");

    Log::debug("[RenderShaderProgram] Compiling shaders...");

    vert_shader.compile();
#if !defined(USE_OPENGL_ES)
    if (tess_ctrl_shader) {
        tess_ctrl_shader->compile();
    }
    if (tess_eval_shader) {
        tess_eval_shader->compile();
    }
    if (geom_shader) {
        geom_shader->compile();
    }
#endif
    frag_shader.compile();

    Log::debug("[RenderShaderProgram] Compiled shaders");
}

void RenderShaderProgram::destroy_vertex_shader()
{
    ZoneScopedN("RenderShaderProgram::destroy_vertex_shader");

    Renderer::detach_shader(index, vert_shader.get_index());
    vert_shader.destroy();
}

#if !defined(USE_OPENGL_ES)
void RenderShaderProgram::destroy_tessellation_control_shader()
{
    ZoneScopedN("RenderShaderProgram::destroy_tessellation_control_shader");

    if (!tess_ctrl_shader) {
        return;
    }

    Renderer::detach_shader(index, tess_ctrl_shader->get_index());
    tess_ctrl_shader->destroy();
    tess_ctrl_shader.reset();
}

void RenderShaderProgram::destroy_tessellation_evaluation_shader()
{
    ZoneScopedN("RenderShaderProgram::destroy_tessellation_evaluation_shader");

    if (!tess_eval_shader) {
        return;
    }

    Renderer::detach_shader(index, tess_eval_shader->get_index());
    tess_eval_shader->destroy();
    tess_eval_shader.reset();
}

void RenderShaderProgram::destroy_geometry_shader()
{
    ZoneScopedN("RenderShaderProgram::destroy_geometry_shader");

    if (!geom_shader) {
        return;
    }

    Renderer::detach_shader(index, geom_shader->get_index());
    geom_shader->destroy();
    geom_shader.reset();
}
#endif

void RenderShaderProgram::destroy_fragment_shader()
{
    ZoneScopedN("RenderShaderProgram::destroy_fragment_shader");

    Renderer::detach_shader(index, frag_shader.get_index());
    frag_shader.destroy();
}

#if !defined(USE_WEBGL)
void ComputeShaderProgram::set_shader(ComputeShader&& comp_shader)
{
    Log::debug(
        "[RenderShaderProgram] Setting shader (ID: " + std::to_string(comp_shader.get_index()) +
        ", path: " + comp_shader.get_path() + ")"
    );

    if (Renderer::is_shader_attached(index, comp_shader.get_index())) {
        Renderer::detach_shader(index, comp_shader.get_index());
    }

    comp_shader = std::move(comp_shader);
    comp_shader.compile();

    Renderer::attach_shader(index, comp_shader.get_index());

    link();
}

ComputeShaderProgram ComputeShaderProgram::clone() const
{
    ComputeShaderProgram program;

    program.set_shader(comp_shader.clone());

    program.attributes = attributes;
    program.textures = textures;
    program.image_textures = image_textures;

    send_attributes();
    init_textures();
    init_image_textures();

    return program;
}

void ComputeShaderProgram::load_shaders() const
{
    ZoneScopedN("ComputeShaderProgram::load_shaders");

    Log::debug("[ComputeShaderProgram] Loading shader...");
    comp_shader.load();
    Log::debug("[ComputeShaderProgram] Loaded shader");
}

void ComputeShaderProgram::compile_shaders() const
{
    ZoneScopedN("ComputeShaderProgram::compile_shaders");

    Log::debug("[ComputeShaderProgram] Compiling shader...");
    comp_shader.compile();
    Log::debug("[ComputeShaderProgram] Compiled shader");
}

void ComputeShaderProgram::execute(Vector3ui group_content) const
{
    ZoneScopedN("ComputeShaderProgram::execute");

    bind_image_textures();
    Renderer::dispatch_compute(group_content);
    Renderer::set_memory_barrier(BarrierType::ALL);
}

void ComputeShaderProgram::destroy_shader()
{
    ZoneScopedN("ComputeShaderProgram::destroy_shader");

    Renderer::detach_shader(index, comp_shader.get_index());
    comp_shader.destroy();
}
#endif
}
