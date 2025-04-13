#include "fps_player.hpp"

#include <physics/physics.hpp>
#include <scene/scenes.hpp>
#include <physics/kinematic_character.hpp>
#include <input/inputs.hpp>

namespace xen {
constexpr float WALK_SPEED = 3.1f;
constexpr float RUN_SPEED = 5.7f;
constexpr float CROUCH_SPEED = 1.2f;
constexpr float JUMP_SPEED = 4.1f;
constexpr float NOCLIP_SPEED = 3.0f;

void FpsPlayer::start()
{
    // auto collisionObject = GetParent()->GetComponent<CollisionObject>();
    // collisionObject->GetCollisionEvents().Subscribe([&](CollisionObject *other){ Log::Out("Player collided with ",
    // std::quoted(other->GetParent()->GetName()), '\n';}));
    // collisionObject->GetSeparationEvents().Subscribe([&](CollisionObject *other){ Log::Out("Player seperated with ",
    // std::quoted(other->GetParent()->GetName()), '\n';}));
    Inputs::get()->get_button("noclip")->on_button.connect([this](InputAction action, InputMods) {
        if (action != InputAction::Press) {
            return;
        }

        auto* scene = Scenes::get()->get_scene();
        auto entity = scene->get_entity_by_component(this);
        auto* character = entity.try_get_component<KinematicCharacter>();

        noclip = !noclip;

        if (noclip) {
            character->set_gravity(Vector3f(0.f, 0.f, 0.f));
            character->set_linear_velocity(Vector3f(1.f));
        }
        else {
            character->set_gravity(scene->get_system<Physics>()->get_gravity());
        }

        Log::out("Player Noclip: ", std::boolalpha, noclip);
    });
}

void FpsPlayer::update()
{
    auto* scene = Scenes::get()->get_scene();
    auto entity = scene->get_entity_by_component(this);
    auto* character = entity.try_get_component<KinematicCharacter>();

    if (!character || !character->is_shape_created()) {
        return;
    }

    Vector3f direction;

    if (!Scenes::get()->get_scene()->is_paused()) {
        direction.x = Inputs::get()->get_axis("strafe")->get_amount();
        direction.z = Inputs::get()->get_axis("forward")->get_amount();

        if (noclip) {
            if (Inputs::get()->get_button("jump")->is_down()) {
                direction.y = 1.0f;
            }
            else if (Inputs::get()->get_button("crouch")->is_down()) {
                direction.y = -1.0f;
            }
        }
        else {
            if (Inputs::get()->get_button("jump")->was_down() && character->is_on_ground()) {
                character->jump({0.0f, JUMP_SPEED, 0.0f});
            }
        }
    }

    auto cameraRotation = scene->get_camera().get_rotation();

    auto& transform = entity.get_component<TransformComponent>();
    transform.rotation = Vector3f({0.0f, cameraRotation.y, 0.0f});

    auto walkDirection = direction;
    walkDirection.x = -(direction.z * std::sin(cameraRotation.y) + direction.x * std::cos(cameraRotation.y));
    walkDirection.z = direction.z * std::cos(cameraRotation.y) - direction.x * std::sin(cameraRotation.y);

    // walkDirection = walkDirection.Normalize();
    walkDirection *= Inputs::get()->get_button("sprint")->is_down() ? RUN_SPEED :
                     Inputs::get()->get_button("crouch")->is_down() ? CROUCH_SPEED :
                                                                      WALK_SPEED;
    walkDirection *= noclip ? NOCLIP_SPEED : 1.0f;
    character->set_walk_direction(0.02f * walkDirection);
}
}