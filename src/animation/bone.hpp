#pragma once

struct aiNodeAnim;

namespace xen {
struct KeyPosition {
    Vector3f position = Vector3f(0.f);
    float timestamp;

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(position, timestamp);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(position, timestamp);
    }
};

struct KeyRotation {
    Quaternion orientation = Quaternion(1.f);
    float timestamp;

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(orientation, timestamp);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(orientation, timestamp);
    }
};

struct KeyScale {
    Vector3f scale = Vector3f(1.f);
    float timestamp;

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(scale, timestamp);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(scale, timestamp);
    }
};

class Bone {
private:
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;

    Matrix4 local_transform = Matrix4(1.f);
    std::string name;
    uint32_t id;

public:
    Bone(std::string_view name, uint32_t id, aiNodeAnim const* channel);

    void update(float current_animation_time);

    [[nodiscard]] uint32_t get_position_index(float current_animation_time) const;
    [[nodiscard]] uint32_t get_rotation_index(float current_animation_time) const;
    [[nodiscard]] uint32_t get_scale_index(float current_animation_time) const;

    [[nodiscard]] std::string_view get_name() const { return name; }
    [[nodiscard]] Matrix4 const& get_local_transform() const { return local_transform; }

    template <class Archive>
    void save(Archive& ar) const
    {
        ar(positions, rotations, scales, local_transform, name, id);
    }
    template <class Archive>
    void load(Archive& ar)
    {
        ar(positions, rotations, scales, local_transform, name, id);
    }

private:
    [[nodiscard]] float get_normalized_interpolation_between_frames(
        float last_timestamp, float next_timestamp, float current_animation_time
    ) const;

    [[nodiscard]] Matrix4 interpolate_position(float current_animation_time) const;
    [[nodiscard]] Matrix4 interpolate_rotation(float current_animation_time) const;
    [[nodiscard]] Matrix4 interpolate_scale(float current_animation_time) const;
};
}