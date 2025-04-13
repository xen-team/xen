#include "animation/animation_clip.hpp"
#include "engine/engine.hpp"
#include "input/axes/button_axis.hpp"
#include "input/axes/combo_axis.hpp"
#include "input/axes/mouse_axis.hpp"
#include "input/axes/scroll_axis.hpp"
#include "input/buttons/mouse_button.hpp"
#include "physics/colliders/capsule_collider.hpp"
#include "physics/kinematic_character.hpp"
#include "render/mesh/common/cube.hpp"
#include "scene/components/fps_player.hpp"
#include "physics/kinematic_character.hpp"
#include "scene/components/light.hpp"
#include "scene/components/mesh.hpp"
#include "scene/components/pose_animator.hpp"
#include "scene/components/water.hpp"
#include "system/windows.hpp"
#include "scene/scenes.hpp"
#include "input/inputs.hpp"
#include "input/buttons/combo_button.hpp"
#include "input/buttons/keyboard_button.hpp"
#include <glm/glm.hpp>
#include <resource/resources.hpp>

using namespace xen;

void load_test_scene(Scene& scene)
{
    Resources& resources = *Resources::get();

    // Optional post fx resources
    // Arcane::Application::GetInstance().GetMasterRenderPass()->GetPostProcessPass()->SetBloomDirtTexture(assetManager.Load2DTextureAsync(std::string("res/textures/bloom-dirt.png")));
    // Arcane::Application::GetInstance().GetMasterRenderPass()->GetPostProcessPass()->SetVignetteTexture(assetManager.Load2DTextureAsync(std::string("res/textures/vignette_mask.jpg")));

    // Load some assets for the scene at startup
    auto cube = std::make_unique<Cube>();
    auto quad = std::make_unique<Quad>();

    // Initialize some entities and components at startup
    // {
    //     std::shared_ptr<Model> gunModel = std::make_shared<Model>(*resources.load_model_async(
    //         std::string("res/3d_models/Cerberus_Gun/Cerberus_LP.FBX"),
    //         [](Model* loadedModel) {
    //             Resources& resources = *Resources::get();
    //             auto& material = loadedModel->get_meshes()[0].get_material();

    //             material.set_normal_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Cerberus_Gun/Textures/Cerberus_N.tga")
    //             );
    //             material.set_metallic_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Cerberus_Gun/Textures/Cerberus_M.tga")
    //             );
    //             material.set_roughness_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Cerberus_Gun/Textures/Cerberus_R.tga")
    //             );
    //             material.set_ambient_occlusion_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Cerberus_Gun/Textures/Cerberus_AO.tga")
    //             );
    //         }
    //     ));

    //     auto gun = scene.create_entity("Cerberus Gun");
    //     auto& transformComponent = gun.get_component<TransformComponent>();
    //     transformComponent.position = {-32.60f, -9.28f, 48.48f};
    //     transformComponent.scale = {0.05f, 0.05f, 0.05f};
    //     auto& meshComponent = gun.add_component<MeshComponent>(gunModel);
    //     meshComponent.is_static = true;
    //     meshComponent.is_transparent = false;
    // }

    // {
    //     auto shieldModel = std::make_shared<Model>(*resources.load_model_async(
    //         std::string("res/3d_models/Hyrule_Shield/HShield.obj"),
    //         [](Model* loadedModel) {
    //             Resources& resources = *Resources::get();
    //             auto& material = loadedModel->get_meshes()[0].get_material();

    //             TextureSettings srgbTextureSettings;
    //             srgbTextureSettings.is_srgb = true;

    //             material.set_albedo_map(*resources.load_2d_texture_async(
    //                 "res/3d_models/Hyrule_Shield/HShield_[Albedo].tga", &srgbTextureSettings
    //             ));
    //             material.set_normal_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Hyrule_Shield/HShield_[Normal].tga")
    //             );
    //             material.set_metallic_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Hyrule_Shield/HShield_[Metallic].tga")
    //             );
    //             material.set_roughness_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Hyrule_Shield/HShield_[Roughness].tga")
    //             );
    //             material.set_ambient_occlusion_map(
    //                 *resources.load_2d_texture_async("res/3d_models/Hyrule_Shield/HShield_[Occlusion].tga")
    //             );
    //         }
    //     ));

    //     auto shield = scene.create_entity("Hyrule Shield");
    //     auto& transformComponent = shield.get_component<TransformComponent>();
    //     transformComponent.position = {-7.4f, -7.6f, -31.4f};
    //     auto& meshComponent = shield.add_component<MeshComponent>(shieldModel);
    //     meshComponent.is_static = true;
    //     meshComponent.is_transparent = false;
    // }

    {
        // auto cubeModel = std::make_shared<Model>(*cube);

        Model cube_model = Model(Cube());

        auto cube = scene.create_entity("Cube");
        auto& transformComponent = cube.get_component<TransformComponent>();
        transformComponent.start();
        transformComponent.scale = {5.0f, 5.0f, 5.0f};

        auto& meshComponent = cube.add_component<MeshComponent>(std::move(cube_model));
        meshComponent.start();
        meshComponent.is_static = true;
        meshComponent.is_transparent = false;
    }

    {
        // auto quadModel = std::make_shared<Model>(*quad);

        Model quad_model = Model(Quad());

        TextureSettings srgbTextureSettings;
        srgbTextureSettings.is_srgb = true;

        quad_model.get_meshes()[0].get_material().set_albedo_map(
            *resources.load_2d_texture_async("res/textures/window.png", &srgbTextureSettings)
        );

        auto window = scene.create_entity("Window");
        auto& transformComponent = window.get_component<TransformComponent>();
        transformComponent.position = {-32.60f, 10.0f, 48.48f};
        transformComponent.rotation = {0.0f, glm::radians(180.0f), 0.0f};
        transformComponent.scale = {10.0f, 10.0f, 10.0f};

        auto& meshComponent = window.add_component<MeshComponent>(std::move(quad_model));
        meshComponent.is_static = true;
        meshComponent.is_transparent = true;
        meshComponent.should_backface_cull = false;
    }

    {
        Model cube_model = Model(Cube());

        auto playa = scene.create_entity("player");
        auto& kinematic = playa.add_component<KinematicCharacter>(std::make_unique<CapsuleCollider>());
        auto& fps = playa.add_component<FpsPlayer>();
        auto& mesh = playa.add_component<MeshComponent>(std::move(cube_model));
    }
    /*das

    {
        auto directionalLight = scene.create_entity("Directional Light");
        auto& transformComponent = directionalLight.get_component<TransformComponent>();
        transformComponent.rotation.x = glm::radians(-120.0f);
        auto& lightComponent = directionalLight.add_component<LightComponent>();
        lightComponent.type = LightType::Directional;
        lightComponent.intensity = 2.0f;
        lightComponent.color = Color(1.0f, 1.0f, 1.0f);
        lightComponent.is_static = true;
        lightComponent.cast_shadows = true;
        lightComponent.shadow_resolution = ShadowQuality::Ultra;
    }

    {
        auto pointLight = scene.create_entity("Point Light1");
        auto& transformComponent = pointLight.get_component<TransformComponent>();
        transformComponent.position = Vector3f(24.1f, 2.2f, 47.5f);
        auto& lightComponent = pointLight.add_component<LightComponent>();
        lightComponent.type = LightType::Point;
        lightComponent.intensity = 10.0f;
        lightComponent.color = Color(0.0f, 1.0f, 0.0f);
        lightComponent.attenuation_range = 30.0f;
        lightComponent.is_static = false;
    }

    {
        auto pointLight = scene.create_entity("Point Light2");
        auto& transformComponent = pointLight.get_component<TransformComponent>();
        transformComponent.position = Vector3f(-27.2f, -9.0f, 52.0f);
        auto& lightComponent = pointLight.add_component<LightComponent>();
        lightComponent.type = LightType::Point;
        lightComponent.intensity = 30.0f;
        lightComponent.color = Color(1.0f, 0.0f, 1.0f);
        lightComponent.attenuation_range = 30.0f;
        lightComponent.is_static = true;
    }

    {
        auto spotLight = scene.create_entity("Spot Light1");
        auto& transformComponent = spotLight.get_component<TransformComponent>();
        transformComponent.position = Vector3f(-86.9f, -5.0f, -28.2f);
        transformComponent.rotation.x = glm::radians(-50.0f);
        auto& lightComponent = spotLight.add_component<LightComponent>();
        lightComponent.type = LightType::Spot;
        lightComponent.intensity = 150.0f;
        lightComponent.attenuation_range = 50.0f;
        lightComponent.color = Color(1.0f, 1.0f, 1.0f);
        lightComponent.is_static = true;
    }

    {
        auto vampire = scene.create_entity("Animated Vampire");
        auto& transformComponent = vampire.get_component<TransformComponent>();
        transformComponent.position = {-70.88f, -9.22f, -39.02f};
        transformComponent.rotation.y = glm::radians(90.0f);
        transformComponent.scale = {0.05f, 0.05f, 0.05f};
        auto& poseAnimatorComponent = vampire.add_component<PoseAnimatorComponent>();

        auto animatedVampire = std::make_shared<Model>(*resources.load_model_async(
            std::string("res/3d_models/Vampire/Dancing_Vampire.dae"),
            [&poseAnimatorComponent](Model* loadedModel) {
                int animIndex = 0;
                auto clip = std::make_shared<AnimationClip>(
                    "res/3d_models/Vampire/Dancing_Vampire.dae", animIndex, *loadedModel
                );

                poseAnimatorComponent.pose_animator.set_animation_clip(clip);

                Resources& resources = *Resources::get();
                Material& meshMaterial = loadedModel->get_meshes()[0].get_material();
                meshMaterial.set_roughness_value(1.0f);
                meshMaterial.set_normal_map(
                    *resources.load_2d_texture_async("res/3d_models/Vampire/textures/Vampire_normal.png")
                );
                meshMaterial.set_metallic_map(
                    *resources.load_2d_texture_async("res/3d_models/Vampire/textures/Vampire_specular.png")
                );
            }
        ));

        auto& meshComponent = vampire.add_component<MeshComponent>(animatedVampire);
        meshComponent.is_static = false;
        meshComponent.is_transparent = false;
        meshComponent.should_backface_cull = false;
    }

    {
        auto water = scene.create_entity("Water");
        auto& transformComponent = water.get_component<TransformComponent>();
        transformComponent.position = {25.0f, -14.0f, -50.0f};
        transformComponent.rotation = {glm::radians(-90.0f), 0.0f, 0.0f};
        transformComponent.scale = {150.0f, 150.0f, 150.0f};
        auto& waterComponent = water.add_component<WaterComponent>();
        waterComponent.water_distortion_texture =
            std::make_shared<Texture>(*resources.load_2d_texture_async("res/water/dudv.png"));
        waterComponent.water_normal_map =
            std::make_shared<Texture>(*resources.load_2d_texture_async("res/water/normals.png"));
    }

    {
        auto brickModel = std::make_shared<Model>(*quad);

        auto bricks = scene.create_entity("Displaced Bricks");
        auto& transformComponent = bricks.get_component<TransformComponent>();
        transformComponent.position = {47.70f, -6.5f, 6.0f};
        transformComponent.rotation = {0.0f, glm::radians(210.0f), 0.0f};
        transformComponent.scale = {5.0f, 5.0f, 5.0f};
        auto& meshComponent = bricks.add_component<MeshComponent>(brickModel);
        meshComponent.is_static = true;

        TextureSettings srgbTextureSettings;
        srgbTextureSettings.is_srgb = true;

        Material& meshMaterial = meshComponent.model->get_meshes()[0].get_material();
        meshMaterial.set_albedo_map(*resources.load_2d_texture_async("res/textures/bricks2.jpg", &srgbTextureSettings));
        meshMaterial.set_normal_map(*resources.load_2d_texture_async("res/textures/bricks2_normal.jpg"));
        meshMaterial.set_displacement_map(*resources.load_2d_texture_async("res/textures/bricks2_disp.jpg"));
        meshMaterial.set_roughness_value(1.0f);
    }

    {
        auto flatModel = std::make_shared<Model>(*quad);

        auto emissionWall = scene.create_entity("Emission Lava");
        auto& transformComponent = emissionWall.get_component<TransformComponent>();
        transformComponent.position = {47.70f, 6.5f, 6.0f};
        transformComponent.rotation = {0.0f, glm::radians(210.0f), 0.0f};
        transformComponent.scale = {5.0f, 5.0f, 5.0f};
        auto& meshComponent = emissionWall.add_component<MeshComponent>(flatModel);
        meshComponent.is_static = true;

        TextureSettings srgbTextureSettings;
        srgbTextureSettings.is_srgb = true;

        Material& meshMaterial = meshComponent.model->get_meshes()[0].get_material();
        meshMaterial.set_albedo_map(*resources.get_black_srgb_texture());
        meshMaterial.set_emission_map(
            *resources.load_2d_texture_async("res/textures/circuitry-emission.png", &srgbTextureSettings)
        );
        meshMaterial.set_emission_intensity(45.0f);
        meshMaterial.set_metallic_value(1.0f);
        meshMaterial.set_roughness_value(1.0f);
    }

    {
        auto cubeModel = std::make_shared<Model>(*cube);

        auto cube = scene.create_entity("Emmissive Cube");
        auto& transformComponent = cube.get_component<TransformComponent>();
        transformComponent.position = {0.0f, 15.0f, 0.0f};
        transformComponent.scale = {5.0f, 5.0f, 5.0f};
        auto& meshComponent = cube.add_component<MeshComponent>(cubeModel);
        meshComponent.is_static = true;
        meshComponent.is_transparent = false;

        Material& meshMaterial = meshComponent.model->get_meshes()[0].get_material();
        meshMaterial.set_emission_color(Color(1.0f, 0.0f, 0.0f));
        meshMaterial.set_emission_intensity(15.0f);
    }

    {
        auto brickModel = std::make_shared<Model>(*quad);

        auto bricks = scene.create_entity("Emmissive Bricks");
        auto& transformComponent = bricks.get_component<TransformComponent>();
        transformComponent.position = {47.70f, 19.5f, 6.0f};
        transformComponent.rotation = {0.0f, glm::radians(210.0f), 0.0f};
        transformComponent.scale = {5.0f, 5.0f, 5.0f};
        auto& meshComponent = bricks.add_component<MeshComponent>(brickModel);
        meshComponent.is_static = true;

        TextureSettings srgbTextureSettings;
        srgbTextureSettings.is_srgb = true;

        Material& meshMaterial = meshComponent.model->get_meshes()[0].get_material();
        meshMaterial.set_albedo_map(*resources.load_2d_texture_async("res/textures/bricks2.jpg", &srgbTextureSettings));
        meshMaterial.set_normal_map(*resources.load_2d_texture_async("res/textures/bricks2_normal.jpg"));
        meshMaterial.set_displacement_map(*resources.load_2d_texture_async("res/textures/bricks2_disp.jpg"));
        meshMaterial.set_emission_map(
            *resources.load_2d_texture_async("res/textures/bricks2_emiss.png", &srgbTextureSettings)
        );
        meshMaterial.set_emission_intensity(5.0f);
        meshMaterial.set_roughness_value(1.0f);
    }
    */
}

int main()
{
    {
        Engine engine;

        auto* window0 = Windows::get()->add_window();
        window0->set_title("Game");
        window0->set_cursor_hidden(true);
        window0->on_close.connect([]() { Engine::get()->request_close(); });

        Render::get()->init_renderer();

        auto* input_scheme =
            Inputs::get()->add_scheme("Test", std::make_unique<InputScheme>("./test_scheme2.json"), true);
        input_scheme->read();

        auto scroll = std::make_unique<ScrollInputAxis>(Axis::Y);

        input_scheme->add_axis("scroll", std::move(scroll));

        input_scheme->get_button("exit")->on_button.connect([](InputAction action, InputMods) {
            // Do some useful workaround with input actions
            if (action != InputAction::Press) {
                return;
            }
            Engine::get()->request_close();
            std::exit(1);
        });

        input_scheme->get_button("capture_mouse")->on_button.connect([&window0](InputAction action, InputMods) {
            if (action != InputAction::Press) {
                return;
            }
            window0->set_cursor_hidden(!window0->is_cursor_hidden());
        });

        input_scheme->get_button("fullscreen")->on_button.connect([&window0](InputAction action, InputMods) {
            if (action != InputAction::Press) {
                return;
            }
            window0->set_fullscreen(!window0->is_fullscreen());
        });

        auto* scene = Scenes::get()->set_scene(std::make_unique<Scene>());

        input_scheme->get_button("save")->on_button.connect([&scene](InputAction action, InputMods) {
            if (action != InputAction::Press) {
                return;
            }
            scene->serialize("pivo.org");
        });

        input_scheme->get_button("load")->on_button.connect([&scene](InputAction action, InputMods) {
            if (action != InputAction::Press) {
                return;
            }
            scene->deserialize("pivo.org");
        });

        input_scheme->get_axis("scroll")->on_axis.connect([&scene](float amount) {
            auto& current_camera = scene->get_camera();
            current_camera.set_fov(current_camera.get_fov() - amount);
        });

        // scene->serialize("lol.scene");
        // scene->deserialize("lol.scene");

        // for (auto ent : scene->get_root().recursive_hierarchy()) {
        // std::cerr << ent.get_name() << '\n';
        // }

        Render::get()->init_scene(*scene);

        load_test_scene(*scene);

        // Something when destoyed blockes program exit scenario, find the culprit
        auto exit_code = engine.run();
    }
    return 1;
}