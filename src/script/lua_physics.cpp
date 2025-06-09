#include "physics/kinematic_character.hpp"
#include <script/lua_wrapper.hpp>
#include <utils/shape.hpp>
#include <utils/type_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_physics_types()
{
    sol::state& state = get_state();

    sol::usertype<KinematicCharacter> kinematic_character =
        state.new_usertype<KinematicCharacter>("KinematicCharacter", sol::base_classes, sol::bases<Component>());
    kinematic_character["set_mass"] = &KinematicCharacter::set_mass;
    kinematic_character["set_collision_check"] = &KinematicCharacter::set_collision_check;
    kinematic_character["is_on_ground"] = &KinematicCharacter::is_on_ground;
    kinematic_character["jump"] = &KinematicCharacter::jump;
    kinematic_character["set_walk_direction"] = &KinematicCharacter::set_walk_direction;
}

}
