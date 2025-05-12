#include <animation/skeleton.hpp>

#include <script/lua_wrapper.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
using namespace TypeUtils;

void LuaWrapper::register_animation_types()
{
    /*
    sol::state& state = get_state();

    {
        sol::usertype<SkeletonJoint> skeletonJoint = state.new_usertype<SkeletonJoint>(
            "SkeletonJoint", sol::constructors<SkeletonJoint(), SkeletonJoint(Quaternion const&, Vector3f const&)>(),
            sol::base_classes, sol::bases<GraphNode<SkeletonJoint>>()
        );
        skeletonJoint["get_parent_count"] = &SkeletonJoint::get_parent_count;
        skeletonJoint["get_parent"] = PickNonConstOverload<size_t>(&SkeletonJoint::get_parent);
        skeletonJoint["get_child_count"] = &SkeletonJoint::get_child_count;
        skeletonJoint["get_child"] = PickNonConstOverload<size_t>(&SkeletonJoint::get_child);
        skeletonJoint["is_root"] = &SkeletonJoint::is_root;
        skeletonJoint["is_leaf"] = &SkeletonJoint::is_leaf;
        skeletonJoint["is_isolated"] = &SkeletonJoint::is_isolated;
        skeletonJoint["add_parents"] = [](SkeletonJoint& j, sol::variadic_args args) {
            for (auto parent : args)
                j.add_parents(parent);
        };
        skeletonJoint["remove_parents"] = [](SkeletonJoint& j, sol::variadic_args args) {
            for (auto parent : args)
                j.remove_parents(parent);
        };
        skeletonJoint["add_children"] = [](SkeletonJoint& j, sol::variadic_args args) {
            for (auto child : args)
                j.add_children(child);
        };
        skeletonJoint["remove_children"] = [](SkeletonJoint& j, sol::variadic_args args) {
            for (auto child : args)
                j.remove_children(child);
        };

        skeletonJoint["rotation"] = sol::property(&SkeletonJoint::get_rotation, &SkeletonJoint::set_rotation);
        skeletonJoint["translation"] = sol::property(&SkeletonJoint::get_translation, &SkeletonJoint::set_translation);
        skeletonJoint["rotate"] = &SkeletonJoint::rotate;
        skeletonJoint["translate"] = &SkeletonJoint::translate;
        skeletonJoint["compute_transform"] = &SkeletonJoint::compute_transform;
    }

    {
        sol::usertype<Skeleton> skeleton =
            state.new_usertype<Skeleton>("Skeleton", sol::constructors<Skeleton(), Skeleton(size_t)>());
        skeleton["get_node_count"] = &Skeleton::get_node_count;
        skeleton["get_node"] = PickNonConstOverload<size_t>(&Skeleton::get_node);
        skeleton["add_node"] = &Skeleton::add_node<>;
        skeleton["remove_node"] = &Skeleton::remove_node;
    }
        */
}
}