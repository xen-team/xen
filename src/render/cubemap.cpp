#include "cubemap.hpp"

#include <data/image.hpp>
#include <data/mesh.hpp>
#include <render/mesh_renderer.hpp>
#include <render/renderer.hpp>

#include <tracy/Tracy.hpp>
#include <GL/glew.h> // Needed by TracyOpenGL.hpp
#include <tracy/TracyOpenGL.hpp>

namespace xen {
namespace {
constexpr std::string_view vert_source = R"(
  layout(location = 0) in vec3 vertPosition;

  layout(std140) uniform uboCameraInfo {
    mat4 uniViewMat;
    mat4 uniInvViewMat;
    mat4 uniProjectionMat;
    mat4 uniInvProjectionMat;
    mat4 uniViewProjectionMat;
    vec3 uniCameraPos;
  };

  out vec3 fragTexcoords;

  void main() {
    fragTexcoords = vertPosition;

    vec4 pos    = uniProjectionMat * (mat4(mat3(uniViewMat)) * vec4(vertPosition, 1.0));
    gl_Position = pos.xyww;
  }
)";

constexpr std::string_view frag_source = R"(
  in vec3 fragTexcoords;

  uniform samplerCube uniSkybox;

  layout(location = 0) out vec4 fragColor;

  void main() {
    fragColor = texture(uniSkybox, fragTexcoords);
  }
)";

inline MeshRenderer const& get_display_cube()
{
    static MeshRenderer const cube = []() {
        Mesh mesh;

        Submesh& submesh = mesh.add_submesh();

        submesh.get_vertices() = {
            Vertex{Vector3f(1.f, 1.f, -1.f)},   // Right top back
            Vertex{Vector3f(1.f, 1.f, 1.f)},    // Right top front
            Vertex{Vector3f(1.f, -1.f, -1.f)},  // Right bottom back
            Vertex{Vector3f(1.f, -1.f, 1.f)},   // Right bottom front
            Vertex{Vector3f(-1.f, 1.f, -1.f)},  // Left top back
            Vertex{Vector3f(-1.f, 1.f, 1.f)},   // Left top front
            Vertex{Vector3f(-1.f, -1.f, -1.f)}, // Left bottom back
            Vertex{Vector3f(-1.f, -1.f, 1.f)}   // Left bottom front
        };

        // Organizing the triangles to be in a clockwise order, since we will always be inside the cube
        submesh.get_triangle_indices() = {
            0, 2, 1, 1, 2, 3, // Right
            4, 5, 7, 4, 7, 6, // Left
            4, 0, 1, 4, 1, 5, // Top
            7, 3, 2, 7, 2, 6, // Bottom
            5, 1, 3, 5, 3, 7, // Front
            0, 4, 6, 0, 6, 2  // Back
        };

        MeshRenderer mesh_renderer;

        RenderShaderProgram& program = mesh_renderer.add_material().get_program();
        program.set_shaders(VertexShader::load_from_source(vert_source), FragmentShader::load_from_source(frag_source));
        program.set_attribute(0, "uniSkybox");
        program.send_attributes();

        mesh_renderer.get_data()->load(mesh, RenderMode::TRIANGLE);
        mesh_renderer.get_submesh_renderers().front().set_material_index(0);

#if !defined(USE_OPENGL_ES)
        if (Renderer::check_version(4, 3)) {
            Renderer::set_label(RenderObjectType::PROGRAM, program.get_index(), "Cubemap shader program");
            Renderer::set_label(
                RenderObjectType::SHADER, program.get_vertex_shader().get_index(), "Cubemap vertex shader"
            );
            Renderer::set_label(
                RenderObjectType::SHADER, program.get_fragment_shader().get_index(), "Cubemap fragment shader"
            );
        }
#endif

        return mesh_renderer;
    }();

    return cube;
}

TextureFormat recover_format(ImageColorspace colorspace)
{
    switch (colorspace) {
    case ImageColorspace::GRAY:
        return TextureFormat::RED;

    case ImageColorspace::GRAY_ALPHA:
        return TextureFormat::RG;

    case ImageColorspace::RGB:
    case ImageColorspace::SRGB:
        return TextureFormat::RGB;

    case ImageColorspace::RGBA:
    case ImageColorspace::SRGBA:
        return TextureFormat::RGBA;

    default:
        break;
    }

    throw std::invalid_argument("Error: Invalid image colorspace");
}

TextureInternalFormat recover_internal_format(ImageColorspace colorspace, ImageDataType data_type)
{
    if (data_type == ImageDataType::BYTE) {
        // RGB(A) images are supposed to be treated as sRGB(A) textures; this will be the case in the future

        if (/*colorspace == ImageColorspace::RGB || */ colorspace == ImageColorspace::SRGB)
            return TextureInternalFormat::SRGB8;

        if (/*colorspace == ImageColorspace::RGBA || */ colorspace == ImageColorspace::SRGBA)
            return TextureInternalFormat::SRGBA8;

        // If the image is of a byte data type and not an sRGB colorspace, its internal format is the same as its format
        return static_cast<TextureInternalFormat>(recover_format(colorspace));
    }

    // Floating-point sRGB(A) images are not treated as sRGB, which is necessarily an integer format; they are therefore
    // interpreted as floating-point RGB(A)
    switch (colorspace) {
    case ImageColorspace::GRAY:
        return TextureInternalFormat::R16F;

    case ImageColorspace::GRAY_ALPHA:
        return TextureInternalFormat::RG16F;

    case ImageColorspace::RGB:
    case ImageColorspace::SRGB:
        return TextureInternalFormat::RGB16F;

    case ImageColorspace::RGBA:
    case ImageColorspace::SRGBA:
        return TextureInternalFormat::RGBA16F;

    default:
        break;
    }

    throw std::invalid_argument("Error: Invalid image colorspace");
}
}

Cubemap::Cubemap()
{
    ZoneScopedN("Cubemap::Cubemap");

    Log::debug("[Cubemap] Creating...");
    Renderer::generate_texture(index);
    Log::debug("[Cubemap] Created (ID: " + std::to_string(index), ")");
}

RenderShaderProgram const& Cubemap::get_program() const
{
    return get_display_cube().get_materials().front().get_program();
}

void Cubemap::load(
    Image const& right, Image const& left, Image const& top, Image const& bottom, Image const& front, Image const& back
) const
{

    ZoneScopedN("Cubemap::load");

    bind();

    constexpr auto map_image = [](Image const& image, TextureType type) {
        if (image.empty()) {
            Log::error("[Cubemap] Empty image given as cubemap face.");
            return;
        }

        Renderer::send_image_data_2d(
            type, 0, recover_internal_format(image.get_colorspace(), image.get_data_type()), image.get_size(),
            recover_format(image.get_colorspace()),
            (image.get_data_type() == ImageDataType::FLOAT ? PixelDataType::FLOAT : PixelDataType::UBYTE), image.data()
        );
    };

    //            ______________________
    //           /|                   /|
    //          / |                  / |
    //         /  |       +Y        /  |
    //        /   |                /   |
    //       |--------------------|    |
    //       |    |        -Z     |    |
    //       | -X |               | +X |
    //       |    |               |    |
    //       |    |    +Z         |    |
    //       |   /----------------|---/
    //       |  /                 |  /
    //       | /        -Y        | /
    //       |/                   |/
    //       ----------------------

    map_image(right, TextureType::CUBEMAP_POS_X);
    map_image(left, TextureType::CUBEMAP_NEG_X);
    map_image(top, TextureType::CUBEMAP_POS_Y);
    map_image(bottom, TextureType::CUBEMAP_NEG_Y);
    map_image(front, TextureType::CUBEMAP_POS_Z);
    map_image(back, TextureType::CUBEMAP_NEG_Z);

    Renderer::set_texture_parameter(TextureType::CUBEMAP, TextureParam::MINIFY_FILTER, TextureParamValue::LINEAR);
    Renderer::set_texture_parameter(TextureType::CUBEMAP, TextureParam::MAGNIFY_FILTER, TextureParamValue::LINEAR);
    Renderer::set_texture_parameter(TextureType::CUBEMAP, TextureParam::WRAP_S, TextureParamValue::CLAMP_TO_EDGE);
    Renderer::set_texture_parameter(TextureType::CUBEMAP, TextureParam::WRAP_T, TextureParamValue::CLAMP_TO_EDGE);
    Renderer::set_texture_parameter(TextureType::CUBEMAP, TextureParam::WRAP_R, TextureParamValue::CLAMP_TO_EDGE);

    unbind();

#if !defined(USE_OPENGL_ES)
    // Setting the label right after creating the texture works, but generates an OpenGL error. This is thus done here
    // instead
    if (Renderer::check_version(4, 3)) {
        Renderer::set_label(RenderObjectType::TEXTURE, index, "Cubemap texture");
    }
#endif
}

void Cubemap::bind() const
{
    Renderer::bind_texture(TextureType::CUBEMAP, index);
}

void Cubemap::unbind() const
{
    Renderer::unbind_texture(TextureType::CUBEMAP);
}

void Cubemap::draw() const
{
    ZoneScopedN("Cubemap::draw");
    TracyGpuZone("Cubemap::draw");

    MeshRenderer const& display_cube = get_display_cube();

    display_cube.get_materials().front().get_program().use();
    Renderer::activate_texture(0);
    bind();

    Renderer::set_depth_function(DepthStencilFunction::LESS_EQUAL);
    display_cube.draw();
    Renderer::set_depth_function(DepthStencilFunction::LESS);
}

Cubemap::~Cubemap()
{
    ZoneScopedN("Cubemap::~Cubemap");

    if (!index.is_valid())
        return;

    Log::debug("[Cubemap] Destroying (ID: " + std::to_string(index) + ")...");
    Renderer::delete_texture(index);
    Log::debug("[Cubemap] Destroyed");
}
}
