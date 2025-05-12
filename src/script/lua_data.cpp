#include <data/bitset.hpp>
#include <data/bvh.hpp>
#include <data/bvh_system.hpp>
#include <data/image.hpp>
#include <data/mesh_distance_field.hpp>
#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_data_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<Bitset> bitset =
            state.new_usertype<Bitset>("Bitset", sol::constructors<Bitset(), Bitset(size_t), Bitset(size_t, bool)>());
        bitset["get_byte_size"] = &Bitset::get_byte_size;
        bitset["empty"] = &Bitset::empty;
        bitset["get_enabled_bit_count"] = &Bitset::get_enabled_bit_count;
        bitset["get_disabled_bit_count"] = &Bitset::get_disabled_bit_count;
        bitset["set_bit"] =
            sol::overload([](Bitset& b, size_t p) { b.set_bit(p); }, PickOverload<size_t, bool>(&Bitset::set_bit));
        bitset["resize"] = &Bitset::resize;
        bitset["reset"] = &Bitset::reset;
        bitset["clear"] = &Bitset::clear;
        bitset.set_function(sol::meta_function::bitwise_not, &Bitset::operator~);
        bitset.set_function(sol::meta_function::bitwise_and, &Bitset::operator&);
        bitset.set_function(sol::meta_function::bitwise_or, &Bitset::operator|);
        bitset.set_function(sol::meta_function::bitwise_xor, &Bitset::operator^);
        bitset.set_function(sol::meta_function::bitwise_left_shift, &Bitset::operator<<);
        bitset.set_function(sol::meta_function::bitwise_right_shift, &Bitset::operator>>);
        bitset.set_function(sol::meta_function::index, &Bitset::operator[]);
    }

    {
        {
            sol::usertype<BoundingVolumeHierarchyNode> bvh_node = state.new_usertype<BoundingVolumeHierarchyNode>(
                "BoundingVolumeHierarchyNode", sol::constructors<BoundingVolumeHierarchyNode()>()
            );
            bvh_node["get_bounding_box"] = &BoundingVolumeHierarchyNode::get_bounding_box;
            bvh_node["has_left_child"] = &BoundingVolumeHierarchyNode::has_left_child;
            bvh_node["get_left_child"] = [](BoundingVolumeHierarchyNode const& n) { return &n.get_left_child(); };
            bvh_node["has_right_child"] = &BoundingVolumeHierarchyNode::has_right_child;
            bvh_node["get_right_child"] = [](BoundingVolumeHierarchyNode const& n) { return &n.get_right_child(); };
            bvh_node["get_triangle"] = &BoundingVolumeHierarchyNode::get_triangle;
            bvh_node["is_leaf"] = &BoundingVolumeHierarchyNode::is_leaf;
            bvh_node["query"] = sol::overload(
                [](BoundingVolumeHierarchyNode const& n, Ray const& r) { return n.query(r); },
                PickOverload<Ray const&, RayHit*>(&BoundingVolumeHierarchyNode::query)
            );
        }

        {
            sol::usertype<BoundingVolumeHierarchy> bvh = state.new_usertype<BoundingVolumeHierarchy>(
                "BoundingVolumeHierarchy", sol::constructors<BoundingVolumeHierarchy()>()
            );
            bvh["get_root_node"] = [](BoundingVolumeHierarchy const& b) { return &b.get_root_node(); };
            // Sol doesn't seem to be able to bind a constant reference to std::vector; leaving a copy here as it is
            // "cheap"
            bvh["build"] = [](BoundingVolumeHierarchy& b, std::vector<Entity*> e) { b.build(e); };
            bvh["query"] = sol::overload(
                [](BoundingVolumeHierarchy const& b, Ray const& r) { return b.query(r); },
                PickOverload<Ray const&, RayHit*>(&BoundingVolumeHierarchy::query)
            );
        }

        {
            sol::usertype<BoundingVolumeHierarchySystem> bvh_system = state.new_usertype<BoundingVolumeHierarchySystem>(
                "BoundingVolumeHierarchySystem", sol::constructors<BoundingVolumeHierarchySystem()>(),
                sol::base_classes, sol::bases<System>()
            );
            bvh_system["get_bvh"] = [](BoundingVolumeHierarchySystem& s) { return &s.get_bvh(); };
        }
    }

    {
        sol::usertype<Color> color = state.new_usertype<Color>(
            "Color", sol::constructors<Color(), Color(float, float, float), Color(float, float, float, float)>()
        );
        color["r"] = sol::property(&Color::r);
        color["g"] = sol::property(&Color::r);
        color["b"] = sol::property(&Color::r);

        sol::table color_preset = state["Color"].get_or_create<sol::table>();
        color_preset["Black"] = sol::var(&Color::Black);
        color_preset["Gray"] = sol::var(&Color::Grey);
        color_preset["Red"] = sol::var(&Color::Red);
        color_preset["Green"] = sol::var(&Color::Green);
        color_preset["Blue"] = sol::var(&Color::Blue);
        color_preset["Aqua"] = sol::var(&Color::Aqua);
        color_preset["Yellow"] = sol::var(&Color::Yellow);
        color_preset["White"] = sol::var(&Color::White);
    }

    {
        sol::usertype<MeshDistanceField> mdf = state.new_usertype<MeshDistanceField>(
            "MeshDistanceField", sol::constructors<MeshDistanceField(xen::AABB const&, Vector3ui const&)>()
        );
        mdf["get_distance"] = &MeshDistanceField::get_distance;
        mdf["set_bvh"] = &MeshDistanceField::set_bvh;
        mdf["compute"] = &MeshDistanceField::compute;
        mdf["recover_slices"] = &MeshDistanceField::recover_slices;
    }
}

}
