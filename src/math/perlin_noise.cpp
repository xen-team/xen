#include "perlin_noise.hpp"

namespace xen::PerlinNoise {
namespace {
constexpr std::array<uint32_t, 512> permutations = {
    151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225, 140, 36,  103, 30,  69,  142, 8,
    99,  37,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203, 117, 35,
    11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175, 74,  165, 71,  134, 139,
    48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,  55,  46,  245, 40,
    244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169, 200, 196, 135,
    130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250, 124, 123, 5,   202, 38,  147,
    118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183, 170, 213, 119,
    248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253, 19,  98,  108, 110,
    79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,  191, 179, 162,
    241, 81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157, 184, 84,  204, 176, 115, 121,
    50,  45,  127, 4,   150, 254, 138, 236, 205, 93,  222, 114, 67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215,
    61,  156, 180, 151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225, 140, 36,  103, 30,
    69,  142, 8,   99,  37,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219,
    203, 117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175, 74,  165,
    71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,  55,
    46,  245, 40,  244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,  18,  169,
    200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250, 124, 123, 5,
    202, 38,  147, 118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189, 28,  42,  223, 183,
    170, 213, 119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,   129, 22,  39,  253, 19,
    98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228, 251, 34,  242, 193, 238, 210, 144, 12,
    191, 179, 162, 241, 81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,  181, 199, 106, 157, 184, 84,  204,
    176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,  222, 114, 67,  29,  24,  72,  243, 141, 128, 195,
    78,  66,  215, 61,  156, 180
};

constexpr std::array<Vector2f, 8> gradients_2d = {
    Vector2f(1.f, 0.f),
    Vector2f(-1.f, 0.f),
    Vector2f(0.f, 1.f),
    Vector2f(0.f, -1.f),
    Vector2f(0.7071067691f, 0.7071067691f),
    Vector2f(-0.7071067691f, 0.7071067691f),
    Vector2f(0.7071067691f, -0.7071067691f),
    Vector2f(-0.7071067691f, -0.7071067691f)
};

// Only 12 gradients are necessary; however, 16 are defined to avoid dividing by 12. These form a regular tetrahedron,
// thus no bias is introduced
constexpr std::array<Vector3f, 16> gradients_3d = {
    Vector3f(0.7071067691f, 0.7071067691f, 0.f),  Vector3f(-0.7071067691f, 0.7071067691f, 0.f),
    Vector3f(0.7071067691f, -0.7071067691f, 0.f), Vector3f(-0.7071067691f, -0.7071067691f, 0.f),
    Vector3f(0.7071067691f, 0.f, 0.7071067691f),  Vector3f(-0.7071067691f, 0.f, 0.7071067691f),
    Vector3f(0.7071067691f, 0.f, -0.7071067691f), Vector3f(-0.7071067691f, 0.f, -0.7071067691f),
    Vector3f(0.f, 0.7071067691f, 0.7071067691f),  Vector3f(0.f, -0.7071067691f, 0.7071067691f),
    Vector3f(0.f, 0.7071067691f, -0.7071067691f), Vector3f(0.f, -0.7071067691f, -0.7071067691f),
    Vector3f(0.7071067691f, 0.7071067691f, 0.f),  Vector3f(-0.7071067691f, 0.7071067691f, 0.f),
    Vector3f(0.f, -0.7071067691f, 0.7071067691f), Vector3f(0.f, -0.7071067691f, -0.7071067691f)
};

constexpr float get_gradient_1d(uint32_t x)
{
    return (permutations[x] % 2 == 0 ? 1.f : -1.f);
}

constexpr float compute_value(float x)
{
    // Determining coordinates on the line
    //
    //  x0---------x0+1

    auto const int_x = static_cast<uint32_t>(x);

    uint32_t const x0 = int_x & 255u;

    float const left_grad = get_gradient_1d(x0);
    float const right_grad = get_gradient_1d(x0 + 1);

    // Computing the distance to the coordinate
    //
    //  |------X--|
    //      x_weight

    float const x_weight = x - static_cast<float>(int_x);

    float const left_dot = x_weight * left_grad;
    float const right_dot = (x_weight - 1) * right_grad;

    float const smooth_x = Math::smooth_step(x_weight);

    return std::lerp(left_dot, right_dot, smooth_x);
}

constexpr Vector2f const& get_gradient_2d(Vector2ui const& pos)
{
    return gradients_2d[permutations[permutations[pos.x] + pos.y] % gradients_2d.size()];
}

constexpr float compute_value(Vector2f const& pos)
{
    // Recovering integer coordinates on the quad
    //
    //  y0+1______x0+1/y0+1
    //     |      |
    //     |      |
    // x0/y0______x0+1

    auto const int_x = static_cast<uint32_t>(pos.x);
    auto const int_y = static_cast<uint32_t>(pos.y);

    uint32_t const x0 = int_x & 255u;
    uint32_t const y0 = int_y & 255u;

    // Recovering pseudo-random gradients at each corner of the quad
    Vector2f const& bot_left_grad = get_gradient_2d(Vector2ui(x0, y0));
    Vector2f const& bot_right_grad = get_gradient_2d(Vector2ui(x0 + 1, y0));
    Vector2f const& top_left_grad = get_gradient_2d(Vector2ui(x0, y0 + 1));
    Vector2f const& top_right_grad = get_gradient_2d(Vector2ui(x0 + 1, y0 + 1));

    // Computing the distance to the coordinates
    //  _____________
    //  |           |
    //  | x_weight   |
    //  |---------X |
    //  |         | y_weight
    //  |_________|_|

    float const x_weight = pos.x - static_cast<float>(int_x);
    float const y_weight = pos.y - static_cast<float>(int_y);

    float const bot_left_dot = Vector2f(x_weight, y_weight).dot(bot_left_grad);
    float const bot_right_dot = Vector2f(x_weight - 1, y_weight).dot(bot_right_grad);
    float const top_left_dot = Vector2f(x_weight, y_weight - 1).dot(top_left_grad);
    float const top_right_dot = Vector2f(x_weight - 1, y_weight - 1).dot(top_right_grad);

    float const smooth_x = Math::smooth_step(x_weight);
    float const smooth_y = Math::smooth_step(y_weight);

    float const bot_coeff = std::lerp(bot_left_dot, bot_right_dot, smooth_x);
    float const top_coeff = std::lerp(top_left_dot, top_right_dot, smooth_x);

    return std::lerp(bot_coeff, top_coeff, smooth_y);
}

constexpr Vector3f const& get_gradient_3d(Vector3ui const& pos)
{
    return gradients_3d[permutations[permutations[permutations[pos.x] + pos.y] + pos.z] % gradients_3d.size()];
}

constexpr float compute_value(Vector3f const& pos)
{
    // Recovering integer coordinates on the cube

    auto const int_x = static_cast<uint32_t>(pos.x);
    auto const int_y = static_cast<uint32_t>(pos.y);
    auto const int_z = static_cast<uint32_t>(pos.z);

    uint32_t const x0 = int_x & 255u;
    uint32_t const y0 = int_y & 255u;
    uint32_t const z0 = int_z & 255u;

    // Recovering pseudo-random gradients at each corner of the quad
    Vector3f const& bot_left_back_grad = get_gradient_3d(Vector3ui(x0, y0, z0));
    Vector3f const& bot_left_front_grad = get_gradient_3d(Vector3ui(x0, y0, z0 + 1));
    Vector3f const& bot_right_back_grad = get_gradient_3d(Vector3ui(x0 + 1, y0, z0));
    Vector3f const& bot_right_front_grad = get_gradient_3d(Vector3ui(x0 + 1, y0, z0 + 1));
    Vector3f const& top_left_back_grad = get_gradient_3d(Vector3ui(x0, y0 + 1, z0));
    Vector3f const& top_left_front_grad = get_gradient_3d(Vector3ui(x0, y0 + 1, z0 + 1));
    Vector3f const& top_right_back_grad = get_gradient_3d(Vector3ui(x0 + 1, y0 + 1, z0));
    Vector3f const& top_right_front_grad = get_gradient_3d(Vector3ui(x0 + 1, y0 + 1, z0 + 1));

    // Computing the distance to the coordinates
    //     _____________
    //    /           /|
    //   /           / |
    //  /___________/ X|
    //  |           |/ |
    //  | x_weight   / z_weight
    //  |---------X |  /
    //  |         | y_weight
    //  |_________|_|/

    float const x_weight = pos.x - static_cast<float>(int_x);
    float const y_weight = pos.y - static_cast<float>(int_y);
    float const z_weight = pos.z - static_cast<float>(int_z);

    float const bot_left_back_dot = Vector3f(x_weight, y_weight, z_weight).dot(bot_left_back_grad);
    float const bot_left_front_dot = Vector3f(x_weight, y_weight, z_weight - 1).dot(bot_left_front_grad);
    float const right_bot_back_dot = Vector3f(x_weight - 1, y_weight, z_weight).dot(bot_right_back_grad);
    float const right_bot_front_dot = Vector3f(x_weight - 1, y_weight, z_weight - 1).dot(bot_right_front_grad);
    float const top_left_back_dot = Vector3f(x_weight, y_weight - 1, z_weight).dot(top_left_back_grad);
    float const top_left_front_dot = Vector3f(x_weight, y_weight - 1, z_weight - 1).dot(top_left_front_grad);
    float const top_right_back_dot = Vector3f(x_weight - 1, y_weight - 1, z_weight).dot(top_right_back_grad);
    float const top_right_front_dot = Vector3f(x_weight - 1, y_weight - 1, z_weight - 1).dot(top_right_front_grad);

    float const smooth_x = Math::smooth_step(x_weight);
    float const smooth_y = Math::smooth_step(y_weight);
    float const smooth_z = Math::smooth_step(z_weight);

    float const bot_back_coeff = std::lerp(bot_left_back_dot, right_bot_back_dot, smooth_x);
    float const bot_front_coeff = std::lerp(bot_left_front_dot, right_bot_front_dot, smooth_x);
    float const top_back_coeff = std::lerp(top_left_back_dot, top_right_back_dot, smooth_x);
    float const top_front_coeff = std::lerp(top_left_front_dot, top_right_front_dot, smooth_x);

    float const back_coeff = std::lerp(bot_back_coeff, top_back_coeff, smooth_y);
    float const front_coeff = std::lerp(bot_front_coeff, top_front_coeff, smooth_y);

    return std::lerp(back_coeff, front_coeff, smooth_z);
}
}

float compute_1d(float x, uint8_t octave_count, bool normalize)
{
    float frequency = 1.f;
    float amplitude = 1.f;
    float total = 0.f;

    for (uint8_t i = 0; i < octave_count; ++i) {
        total += compute_value(x * frequency) * amplitude;

        amplitude *= 0.5f;
        frequency *= 2.f;
    }

    if (normalize) {
        return (total + 1) / 2; // Scaling between [0; 1]
    }

    return total;
}

float compute_2d(Vector2f const& pos, uint8_t octave_count, bool normalize)
{
    float frequency = 1.f;
    float amplitude = 1.f;
    float total = 0.f;

    for (uint8_t i = 0; i < octave_count; ++i) {
        total += compute_value(pos * frequency) * amplitude;

        amplitude *= 0.5f;
        frequency *= 2.f;
    }

    if (normalize) {
        return (total + 1) / 2; // Scaling between [0; 1]
    }

    return total;
}

float compute_3d(Vector3f const& pos, uint8_t octave_count, bool normalize)
{
    float frequency = 1.f;
    float amplitude = 1.f;
    float total = 0.f;

    for (uint8_t i = 0; i < octave_count; ++i) {
        total += compute_value(pos * frequency) * amplitude;

        amplitude *= 0.5f;
        frequency *= 2.f;
    }

    if (normalize) {
        return (total + 1) / 2; // Scaling between [0; 1]
    }

    return total;
}
}