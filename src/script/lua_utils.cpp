#include "utils/health.hpp"
#include <script/lua_wrapper.hpp>
#include <utils/filepath.hpp>
#include <utils/file_utils.hpp>

#include <utils/ray.hpp>
#include <utils/shape.hpp>

#include <utils/trigger_system.hpp>
#include <utils/trigger_volume.hpp>
#include <utils/type_utils.hpp>
#include <utils/str_utils.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

namespace xen {

using namespace TypeUtils;

void LuaWrapper::register_utils_types()
{
    sol::state& state = get_state();

    {
        sol::usertype<FilePath> filepath = state.new_usertype<FilePath>(
            "FilePath", sol::constructors<FilePath(), FilePath(char const*), FilePath(std::string const&)>()
        );
        filepath["empty"] = &FilePath::empty;
        filepath["recover_path_to_file"] = PickOverload<>(&FilePath::recover_path_to_file);
        filepath["recover_filename"] = sol::overload(
            [](FilePath const& f) { return f.recover_filename(); }, PickOverload<bool>(&FilePath::recover_filename)
        );
        filepath["recover_extension"] = PickOverload<>(&FilePath::recover_extension);
        filepath["to_utf8"] = &FilePath::to_utf8;
        filepath.set_function(
            sol::meta_function::concatenation,
            sol::overload(
                PickOverload<char const*>(&FilePath::operator+), PickOverload<std::string const&>(&FilePath::operator+),
                PickOverload<FilePath const&>(&FilePath::operator+),
                [](char const* s, FilePath const& p) { return s + p; },
                [](std::string const& s, FilePath const& p) { return s + p; }
            )
        );
    }

    {
        sol::table fileUtils = state["FileUtils"].get_or_create<sol::table>();
        fileUtils["is_readable"] = &FileUtils::is_readable;
        fileUtils["read_file_to_array"] = &FileUtils::read_file_to_array;
        fileUtils["read_file_to_string"] = &FileUtils::read_file_to_string;
    }

    {
        // sol::table logger = state["Log"].get_or_create<sol::table>();
        // logger["error"] = &Log::error;
        // logger["warning"] = &Log::warning;
        // logger["info"] = &Log::info;
        // logger["debug"] =
        // sol::overload(PickOverload<char const*>(&Log::debug), PickOverload<std::string const&>(&Log::debug));
    }

    {
        {
            sol::usertype<RayHit> rayHit = state.new_usertype<RayHit>("RayHit", sol::constructors<RayHit()>());
            rayHit["position"] = &RayHit::position;
            rayHit["normal"] = &RayHit::normal;
            rayHit["distance"] = &RayHit::distance;
        }

        {
            sol::usertype<Ray> ray =
                state.new_usertype<Ray>("Ray", sol::constructors<Ray(Vector3f const&, Vector3f const&)>());
            ray["get_origin"] = &Ray::get_origin;
            ray["get_direction"] = &Ray::get_direction;
            ray["get_inverse_direction"] = &Ray::get_inverse_direction;
            ray["intersects"] = sol::overload(
                [](Ray const& r, Vector3f const& p) { r.intersects(p); },
                PickOverload<Vector3f const&, RayHit*>(&Ray::intersects),
                [](Ray const& r, AABB const& s) { r.intersects(s); },
                PickOverload<AABB const&, RayHit*>(&Ray::intersects),
                //[] (const Ray& r, const Line& s) { r.intersects(s); },
                // PickOverload<const Line&, RayHit*>(&Ray::intersects),
                //[] (const Ray& r, const OBB& s) { r.intersects(s); },
                // PickOverload<const OBB&, RayHit*>(&Ray::intersects),
                [](Ray const& r, Plane const& s) { r.intersects(s); },
                PickOverload<Plane const&, RayHit*>(&Ray::intersects),
                //[] (const Ray& r, const Quad& s) { r.intersects(s); },
                // PickOverload<const Quad&, RayHit*>(&Ray::intersects),
                [](Ray const& r, Sphere const& s) { r.intersects(s); },
                PickOverload<Sphere const&, RayHit*>(&Ray::intersects),
                [](Ray const& r, Triangle const& s) { r.intersects(s); },
                PickOverload<Triangle const&, RayHit*>(&Ray::intersects)
            );
            ray["compute_projection"] = &Ray::compute_projection;
        }
    }

    {
        sol::usertype<Health> health = state.new_usertype<Health>("Health", sol::constructors<Health(float)>());
        health["get"] = &Health::get;
        health["get_max"] = &Health::get_max;
        health["apply_damage"] = &Health::apply_damage;
        health["apply_heal"] = &Health::apply_heal;
    }

    {
        sol::table strUtils = state["StrUtils"].get_or_create<sol::table>();
        // strUtils["starts_with"] = PickOverload<std::string const&, std::string const&>(&std::string::starts_with);
        // strUtils["ends_with"] = PickOverload<std::string const&, std::string const&>(&std::string::ends_with);
        strUtils["to_lower_copy"] = PickOverload<std::string>(&StrUtils::to_lower_copy);
        strUtils["to_upper_copy"] = PickOverload<std::string>(&StrUtils::to_upper_copy);
        strUtils["trim_left_copy"] = PickOverload<std::string>(&StrUtils::trim_left_copy);
        strUtils["trim_right_copy"] = PickOverload<std::string>(&StrUtils::trim_right_copy);
        strUtils["trim_copy"] = PickOverload<std::string>(&StrUtils::trim_copy);
        strUtils["split"] = PickOverload<std::string, char>(&StrUtils::split);
    }

    {
        state.new_usertype<Triggerer>("Triggerer", sol::constructors<Triggerer()>());
    }

    {
        state.new_usertype<TriggerSystem>("TriggerSystem", sol::constructors<TriggerSystem()>());
    }

    {
        sol::usertype<TriggerVolume> trigger_volume = state.new_usertype<TriggerVolume>(
            "TriggerVolume", sol::constructors<TriggerVolume(AABB const&), TriggerVolume(Sphere const&)>()
        );
        trigger_volume["set_enter_action"] = &TriggerVolume::set_enter_action;
        trigger_volume["set_stay_action"] = &TriggerVolume::set_stay_action;
        trigger_volume["set_leave_action"] = &TriggerVolume::set_leave_action;
        trigger_volume["enable"] =
            sol::overload([](TriggerVolume& v) { v.enable(); }, PickOverload<bool>(&TriggerVolume::enable));
        trigger_volume["disable"] = &TriggerVolume::disable;
        trigger_volume["reset_enter_action"] = &TriggerVolume::reset_enter_action;
        trigger_volume["reset_stay_action"] = &TriggerVolume::reset_stay_action;
        trigger_volume["reset_leave_action"] = &TriggerVolume::reset_leave_action;
    }
}

}
