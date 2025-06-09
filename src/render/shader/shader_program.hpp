#pragma once

#include <data/owner_value.hpp>
#include <render/shader/shader.hpp>
#include <render/texture.hpp>

#if !defined(USE_WEBGL) && defined(EMSCRIPTEN)
#define USE_WEBGL
#endif

#if !defined(USE_OPENGL_ES) && (defined(EMSCRIPTEN) || defined(USE_WEBGL))
#define USE_OPENGL_ES
#endif

namespace xen {

enum class ImageAccess : uint32_t;
enum class ImageInternalFormat : uint32_t;

enum class ImageTextureUsage : uint8_t { READ = 0, WRITE, READ_WRITE };

/// ShaderProgram class, holding shaders & handling data transmission to the graphics card with uniforms.
class ShaderProgram {
public:
    ShaderProgram();
    ShaderProgram(ShaderProgram const&) = delete;
    ShaderProgram(ShaderProgram&&) noexcept = default;

    ShaderProgram& operator=(ShaderProgram const&) = delete;
    ShaderProgram& operator=(ShaderProgram&&) noexcept = default;

    virtual ~ShaderProgram();

    [[nodiscard]] uint32_t get_index() const { return index; }

    /// Checks if an attribute has been set with the given uniform name.
    /// \param uniform_name Uniform name to be checked.
    /// \return True if an attribute exists with the given name, false otherwise.
    [[nodiscard]] bool has_attribute(std::string const& uniform_name) const;

    /// Checks if an attribute has been set with the given uniform name and type.
    /// \tparam T Type to be checked.
    /// \param uniform_name Uniform name to be checked.
    /// \return True if an attribute exists with the given name and type, false otherwise.
    template <typename T>
    [[nodiscard]] bool has_attribute(std::string const& uniform_name) const;

    [[nodiscard]] size_t get_attribute_count() const { return attributes.size(); }

    /// Fetches an attribute's value from its uniform name.
    /// \tparam T Type of the attribute to get. It MUST be the same type the uniform has been set with.
    /// \param uniform_name Uniform name of the attribute to get.
    /// \return Attribute found.
    template <typename T>
    [[nodiscard]] T const& get_attribute(std::string const& uniform_name) const;

    /// Checks if there is a texture entry with the given texture.
    /// \param texture Texture to find.
    /// \return True if an entry has been found, false otherwise.
    [[nodiscard]] bool has_texture(Texture const& texture) const;

    /// Checks if there is a texture entry with the given uniform name.
    /// \param uniform_name Uniform name to find.
    /// \return True if an entry has been found, false otherwise.
    [[nodiscard]] bool has_texture(std::string const& uniform_name) const;

    [[nodiscard]] std::vector<std::pair<TexturePtr, std::string>> const& get_textures() const { return textures; }

    [[nodiscard]] size_t get_texture_count() const { return textures.size(); }

    [[nodiscard]] Texture const& get_texture(size_t index) const { return *textures[index].first; }

    [[nodiscard]] Texture const& get_texture(std::string const& uniform_name) const;

#if !defined(USE_WEBGL)
    /// Checks if there is an image texture entry with the given texture.
    /// \param texture Texture to find.
    /// \return True if an entry has been found, false otherwise.
    [[nodiscard]] bool has_image_texture(Texture const& texture) const;

    /// Checks if there is an image texture entry with the given uniform name.
    /// \param uniform_name Uniform name to find.
    /// \return True if an entry has been found, false otherwise.
    [[nodiscard]] bool has_image_texture(std::string const& uniform_name) const;

    [[nodiscard]] size_t get_image_texture_count() const { return image_textures.size(); }

    [[nodiscard]] Texture const& get_image_texture(size_t index) const { return *image_textures[index].first; }

    [[nodiscard]] Texture const& get_image_texture(std::string const& uniform_name) const;
#endif

    /// Sets an attribute to be sent to the shaders. If the uniform name already exists, replaces the attribute's value.
    /// \tparam T Type of the attribute to set. Must be a type handled by ShaderProgram::send_uniform().
    /// \param attrib_val Attribute to set.
    /// \param uniform_name Uniform name of the attribute to set.
    template <typename T>
    void set_attribute(T&& attrib_val, std::string const& uniform_name);

    /// Sets a texture to be bound to the shaders. If the uniform name already exists, replaces the texture.
    /// \param texture Texture to set.
    /// \param uniform_name Uniform name to bind the texture to.
    void set_texture(TexturePtr texture, std::string const& uniform_name);

#if !defined(USE_WEBGL)
    /// Sets an image texture to be bound to the shaders. If the uniform name already exists, replaces the texture.
    /// \param texture Texture to set.
    /// \param uniform_name Uniform name to bind the texture to.
    /// \param usage Usage made of the texture.
    /// \see https://www.khronos.org/opengl/wiki/Image_Load_Store
    void set_image_texture(
        TexturePtr texture, std::string const& uniform_name, ImageTextureUsage usage = ImageTextureUsage::READ_WRITE
    );
#endif

    /// Loads all the shaders contained by the program.
    virtual void load_shaders() const = 0;

    /// Compiles all the shaders contained by the program.
    virtual void compile_shaders() const = 0;

    /// Links the program to the graphics card.
    /// \note Linking a program resets all its attributes' values and textures' bindings;
    ///   you may want to call send_attributes(), init_textures() & init_image_textures() afterward.
    void link();

    /// Checks if the program has been successfully linked.
    /// \return True if the program is linked, false otherwise.
    bool is_linked() const;

    /// Loads & compiles all the shaders contained by the program, links it and initializes its attributes & textures.
    void update_shaders();

    /// Marks the program as used.
    void use() const;

    /// Checks if the program is currently defined as used.
    bool is_used() const;

    /// Sends the program's attributes as uniforms.
    void send_attributes() const;

    /// Removes an attribute given its uniform name.
    /// \param uniform_name Uniform name of the attribute to remove.
    void remove_attribute(std::string const& uniform_name);

    /// Removes all attributes in the program.
    void clear_attributes() { attributes.clear(); }

    /// Sets the program's textures' binding points.
    void init_textures() const;

    /// Binds the program's textures.
    void bind_textures() const;

    /// Removes all textures associated with the given texture.
    /// \param texture Texture to remove the entries for.
    void remove_texture(Texture const& texture);

    /// Removes the texture associated with the given uniform name.
    /// \param uniform_name Uniform name to remove the entry for.
    void remove_texture(std::string const& uniform_name);

    /// Removes all textures associated to the program.
    void clear_textures() { textures.clear(); }

#if !defined(USE_WEBGL)
    /// Sets the program's image textures' binding points.
    void init_image_textures() const;

    /// Binds the program's image textures.
    void bind_image_textures() const;

    /// Removes all image textures associated with the given texture.
    /// \param texture Texture to remove the entries for.
    void remove_image_texture(Texture const& texture);

    /// Removes the image texture associated with the given uniform name.
    /// \param uniform_name Uniform name to remove the entry for.
    void remove_image_texture(std::string const& uniform_name);

    /// Removes all image textures associated to the program.
    void clear_image_textures() { image_textures.clear(); }
#endif
    /// Gets the uniform's location (ID) corresponding to the given name.
    /// \note Location will be -1 if the name is incorrect or if the uniform isn't used in the shader(s) (will be
    /// optimized out).
    /// \param name Name of the uniform to recover the location from.
    /// \return Location (ID) of the uniform.
    int recover_uniform_location(std::string const& name) const;

    /// Sends an integer as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param value Integer to be sent.
    void send_uniform(int index, int value) const;

    /// Sends an uinteger as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param value Unsigned integer to be sent.
    void send_uniform(int index, uint32_t value) const;

    /// Sends a floating-point value as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param value Floating-point value to be sent.
    void send_uniform(int index, float value) const;

    /// Sends a list of integers as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param values Integers to be sent.
    /// \param count Number of values to be sent.
    void send_uniform(int index, int const* values, size_t count) const;

    /// Sends a list of uintegers as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param values Unsigned integers to be sent.
    /// \param count Number of values to be sent.
    void send_uniform(int index, uint32_t const* values, size_t count) const;

    /// Sends a list of floating-point values as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param values Floating-point values to be sent.
    /// \param count Number of values to be sent.
    void send_uniform(int index, float const* values, size_t count) const;

    /// Sends a list of integers as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param values Integers to be sent.
    void send_uniform(int index, std::vector<int> const& values) const
    {
        send_uniform(index, values.data(), values.size());
    }

    /// Sends a list of uintegers as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param values Unsigned integers to be sent.
    void send_uniform(int index, std::vector<uint32_t> const& values) const
    {
        send_uniform(index, values.data(), values.size());
    }

    /// Sends a list of floating-point values as uniform.
    /// \param index Index of the uniform to send the data to.
    /// \param values Floating-point values to be sent.
    void send_uniform(int index, std::vector<float> const& values) const
    {
        send_uniform(index, values.data(), values.size());
    }

    /// Sends an integer 2D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector2i const& vec) const;

    /// Sends an integer 3D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector3i const& vec) const;

    /// Sends an integer 4D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector4i const& vec) const;

    /// Sends an uinteger 2D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector2ui const& vec) const;

    /// Sends an uinteger 3D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector3ui const& vec) const;

    /// Sends an uinteger 4D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector4ui const& vec) const;

    /// Sends a floating-point 2D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector2f const& vec) const;

    /// Sends a floating-point 3D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector3f const& vec) const;

    /// Sends a floating-point 4D vector as uniform.
    /// \param index Index of the uniform to send the vector's data to.
    /// \param vec Vector to be sent.
    void send_uniform(int index, Vector4f const& vec) const;

    /// Sends a floating-point 2x2 matrix as uniform.
    /// \param index Index of the uniform to send the matrix's data to.
    /// \param mat Matrix to be sent.
    void send_uniform(int index, Matrix2 const& mat) const;

    /// Sends a floating-point 3x3 matrix as uniform.
    /// \param index Index of the uniform to send the matrix's data to.
    /// \param mat Matrix to be sent.
    void send_uniform(int index, Matrix3 const& mat) const;

    /// Sends a floating-point 4x4 matrix as uniform.
    /// \param index Index of the uniform to send the matrix's data to.
    /// \param mat Matrix to be sent.
    void send_uniform(int index, Matrix4 const& mat) const;

    /// Sends a floating-point 4x4 matrix as uniform.
    /// \param index Index of the uniform to send the matrix's data to.
    /// \param mat Matrix to be sent.
    void send_uniform(int index, Color const& color) const;

    /// Sends an integer as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param value Integer to be sent.
    void send_uniform(std::string const& name, int value) const { send_uniform(recover_uniform_location(name), value); }

    /// Sends an uinteger as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param value Unsigned integer to be sent.
    void send_uniform(std::string const& name, uint32_t value) const
    {
        send_uniform(recover_uniform_location(name), value);
    }

    /// Sends a floating-point value as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param value Floating-point value to be sent.
    void send_uniform(std::string const& name, float value) const
    {
        send_uniform(recover_uniform_location(name), value);
    }

    /// Sends a list of integers as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param values Integers to be sent.
    /// \param count Number of values to be sent.
    void send_uniform(std::string const& name, int const* values, size_t count) const
    {
        send_uniform(recover_uniform_location(name), values, count);
    }

    /// Sends a list of uintegers as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param values Unsigned integers to be sent.
    /// \param count Number of values to be sent.
    void send_uniform(std::string const& name, uint32_t const* values, size_t count) const
    {
        send_uniform(recover_uniform_location(name), values, count);
    }

    /// Sends a list of floating-point values as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param values Floating-point values to be sent.
    /// \param count Number of values to be sent.
    void send_uniform(std::string const& name, float const* values, size_t count) const
    {
        send_uniform(recover_uniform_location(name), values, count);
    }

    /// Sends a list of integers as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param values Integers to be sent.
    void send_uniform(std::string const& name, std::vector<int> const& values) const
    {
        send_uniform(recover_uniform_location(name), values);
    }

    /// Sends a list of uintegers as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param values Unsigned integers to be sent.
    void send_uniform(std::string const& name, std::vector<uint32_t> const& values) const
    {
        send_uniform(recover_uniform_location(name), values);
    }

    /// Sends a list of floating-point values as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param values Floating-point values to be sent.
    void send_uniform(std::string const& name, std::vector<float> const& values) const
    {
        send_uniform(recover_uniform_location(name), values);
    }

    /// Sends an integer 2D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector2i const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends an integer 3D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector3i const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends an integer 4D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector4i const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends an uinteger 2D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector2ui const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends an uinteger 3D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector3ui const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends an uinteger 4D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector4ui const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends a floating-point 2D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector2f const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends a floating-point 3D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector3f const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends a floating-point 4D vector as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param vec Vector to be sent.
    void send_uniform(std::string const& name, Vector4f const& vec) const
    {
        send_uniform(recover_uniform_location(name), vec);
    }

    /// Sends a floating-point 2x2 matrix as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param mat Matrix to be sent.
    void send_uniform(std::string const& name, Matrix2 const& mat) const
    {
        send_uniform(recover_uniform_location(name), mat);
    }

    /// Sends a floating-point 3x3 matrix as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param mat Matrix to be sent.
    void send_uniform(std::string const& name, Matrix3 const& mat) const
    {
        send_uniform(recover_uniform_location(name), mat);
    }

    /// Sends a floating-point 4x4 matrix as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param mat Matrix to be sent.
    void send_uniform(std::string const& name, Matrix4 const& mat) const
    {
        send_uniform(recover_uniform_location(name), mat);
    }

    /// Sends a floating-point 4x4 matrix as uniform, which location will be retrieved with its name.
    /// \param name Name of the uniform to retrieve the location from.
    /// \param mat Matrix to be sent.
    void send_uniform(std::string const& name, Color const& color) const
    {
        send_uniform(recover_uniform_location(name), color);
    }

protected:
    struct Attribute {
        int location = -1;
        std::variant<
            int, uint32_t, float, Vector2i, Vector3i, Vector4i, Vector2ui, Vector3ui, Vector4ui, Vector2f, Vector3f,
            Vector4f, Matrix2, Matrix3, Matrix4, Color, std::vector<int>, std::vector<uint32_t>, std::vector<float>>
            value{};
    };

    struct ImageTextureAttachment {
        std::string uniform_name;
        ImageAccess access{};
        ImageInternalFormat format{};
    };

    OwnerValue<uint32_t> index{};

    std::unordered_map<std::string, Attribute> attributes{};
    std::vector<std::pair<TexturePtr, std::string>> textures{};
#if !defined(USE_WEBGL)
    std::vector<std::pair<TexturePtr, ImageTextureAttachment>> image_textures{};
#endif

private:
    /// Updates all attributes' uniform locations.
    void update_attributes_locations();
};

class RenderShaderProgram final : public ShaderProgram {
public:
    RenderShaderProgram() = default;
    RenderShaderProgram(VertexShader&& vert_shader, FragmentShader&& frag_shader) : RenderShaderProgram()
    {
        set_shaders(std::move(vert_shader), std::move(frag_shader));
    }
#if !defined(USE_OPENGL_ES)
    RenderShaderProgram(VertexShader&& vert_shader, FragmentShader&& frag_shader, GeometryShader&& geom_shader) :
        RenderShaderProgram()
    {
        set_shaders(std::move(vert_shader), std::move(geom_shader), std::move(frag_shader));
    }
#endif

    const VertexShader& get_vertex_shader() const { return vert_shader; }
#if !defined(USE_OPENGL_ES)
    bool has_tessellation_control_shader() const { return tess_ctrl_shader.has_value(); }
    TessellationControlShader const& get_tessellation_control_shader() const
    {
        Log::rt_assert(has_tessellation_control_shader());
        return *tess_ctrl_shader;
    }
    bool has_tessellation_evaluation_shader() const { return tess_eval_shader.has_value(); }
    TessellationEvaluationShader const& get_tessellation_evaluation_shader() const
    {
        Log::rt_assert(has_tessellation_evaluation_shader());
        return *tess_eval_shader;
    }
    bool has_geometry_shader() const { return geom_shader.has_value(); }
    GeometryShader const& get_geometry_shader() const
    {
        Log::rt_assert(has_geometry_shader());
        return *geom_shader;
    }
#endif
    const FragmentShader& get_fragment_shader() const { return frag_shader; }

    void set_vertex_shader(VertexShader&& vert_shader);

#if !defined(USE_OPENGL_ES)
    void set_tessellation_control_shader(TessellationControlShader&& tess_ctrl_shader);

    void set_tessellation_evaluation_shader(TessellationEvaluationShader&& tess_eval_shader);

    void set_geometry_shader(GeometryShader&& geom_shader);
#endif

    void set_fragment_shader(FragmentShader&& frag_shader);

    void set_shaders(VertexShader&& vert_shader, FragmentShader&& frag_shader);

#if !defined(USE_OPENGL_ES)
    void set_shaders(VertexShader&& vert_shader, GeometryShader&& geom_shader, FragmentShader&& frag_shader);

    void set_shaders(
        VertexShader&& vert_shader, TessellationEvaluationShader&& tess_eval_shader, FragmentShader&& frag_shader
    );

    void set_shaders(
        VertexShader&& vert_shader, TessellationControlShader&& tess_ctrl_shader,
        TessellationEvaluationShader&& tess_eval_shader, FragmentShader&& frag_shader
    );
#endif

    RenderShaderProgram clone() const;

    /// Loads all the shaders contained by the program.
    void load_shaders() const override;

    /// Compiles all the shaders contained by the program.
    void compile_shaders() const override;

    /// Destroys the vertex shader, detaching it from the program & deleting it.
    void destroy_vertex_shader();

#if !defined(USE_OPENGL_ES)
    /// Destroys the tessellation control shader (if any), detaching it from the program & deleting it.
    void destroy_tessellation_control_shader();
    /// Destroys the tessellation evaluation shader (if any), detaching it from the program & deleting it.
    void destroy_tessellation_evaluation_shader();
    /// Destroys the geometry shader (if any), detaching it from the program & deleting it.
    void destroy_geometry_shader();
#endif
    /// Destroys the fragment shader, detaching it from the program & deleting it.
    void destroy_fragment_shader();

private:
    VertexShader vert_shader{};
#if !defined(USE_OPENGL_ES)
    std::optional<TessellationControlShader> tess_ctrl_shader{};
    std::optional<TessellationEvaluationShader> tess_eval_shader{};
    std::optional<GeometryShader> geom_shader{};
#endif
    FragmentShader frag_shader{};
};

#if !defined(USE_WEBGL)
class ComputeShaderProgram final : public ShaderProgram {
public:
    ComputeShaderProgram() = default;
    explicit ComputeShaderProgram(ComputeShader&& comp_shader) : ComputeShaderProgram()
    {
        set_shader(std::move(comp_shader));
    }

    ComputeShader const& get_shader() const { return comp_shader; }

    void set_shader(ComputeShader&& comp_shader);

    ComputeShaderProgram clone() const;

    /// Loads the compute shader contained by the program.
    void load_shaders() const override;

    /// Compiles the compute shader contained by the program.
    void compile_shaders() const override;

    void execute(Vector3ui group_content = Vector3ui(1)) const;

    /// Destroys the compute shader, detaching it from the program & deleting it.
    void destroy_shader();

private:
    ComputeShader comp_shader{};
};
#endif

}

#include "shader_program.inl"