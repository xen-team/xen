#pragma once

namespace sol {
class state;
}

namespace xen {
class FilePath;

class LuaWrapper {
    friend class LuaEnvironment;

public:
    static void register_types();

    /// Executes a script from a string.
    /// \param code Code to be executed.
    /// \return True if the script has been executed without errors, false otherwise.
    static bool execute(std::string const& code);

    /// Executes a script from a file.
    /// \param filepath Path to the script to be executed. Must be valid ASCII; special characters are not handled.
    /// \return True if the script has been executed without errors, false otherwise.
    static bool execute_from_file(FilePath const& filepath);

    /// Runs the garbage collector, releasing all unreferenced variables.
    static void collect_garbage();

private:
    static void register_animation_types();
    static void register_audio_types();
    static void register_core_types();
    static void register_data_types();
    static void register_entity_types();
    static void register_file_format_types();
    static void register_image_types();
    static void register_math_types();
    static void register_matrix_types();
    static void register_mesh_types();
    static void register_mesh_renderer_types();
    static void register_overlay_base_types();
    static void register_overlay_widget_types();
    static void register_physics_types();
    static void register_render_types();
    static void register_render_graph_types();
    static void register_render_system_types();
    static void register_shader_types();
    static void register_shader_program_types();
    static void register_shape_types();
    static void register_texture_types();
    static void register_utils_types();
    static void register_vector_types();
    static void register_window_types();
    static void register_xr_types();

    static sol::state& get_state();
};
}