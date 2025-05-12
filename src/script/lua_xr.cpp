#include <xr/xr_system.hpp>
#include <script/lua_wrapper.hpp>

#include "openxr/openxr.h"

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {
void LuaWrapper::register_xr_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<XrSystem> xr_system = state.new_usertype<XrSystem>(
            "XrSystem", sol::constructors<XrSystem(std::string const&)>(), sol::base_classes, sol::bases<System>()
        );
        xr_system["get_optimal_view_size"] = &XrSystem::get_optimal_view_size;
    }
}
}