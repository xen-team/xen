#include "engine.hpp"

#include "debug/log.hpp"
// #include "Config.hpp"

namespace xen {
Engine* Engine::instance = nullptr;

Engine::Engine() :
	// version{ACID_VERSION_MAJOR, ACID_VERSION_MINOR, ACID_VERSION_PATCH},
	fps_limit(-1.0f),
	running(true),
	elapsed_update(Time::seconds(1.f / 60.f)),
	elapsed_render(-1s) {
	instance = this;
	Log::open_log(Time::get_date_time("Logs/%Y-%m-%d-%H-%M-%S.txt"));

#ifdef XEN_DEBUG
	// Log::out("Version: ", ACID_VERSION, '\n');
	// Log::out("Git: ", ACID_COMPILED_COMMIT_HASH, " on ", ACID_COMPILED_BRANCH, '\n');
	// Log::out("Compiled on: ", ACID_COMPILED_SYSTEM, " from: ", ACID_COMPILED_GENERATOR, " with: ", ACID_COMPILED_COMPILER, "\n\n");
#endif

	Log::out("Engine module creation started.\n");
	for (auto it = Module::registry().cbegin(); it != Module::registry().cend(); ++it)
		create_module(it);
	Log::out("Engine module creation ended.\n");
}

Engine::~Engine() {
	app = nullptr;

	for (auto it = modules.rbegin(); it != modules.rend(); ++it)
		destroy_module(it->first);
	
	Log::close_log();
}

void Engine::process_updates() {
    if (elapsed_update.get_elapsed() != 0) {
		std::cerr << "U/" << '\n';

        ups.update(Time::now());

        update_stage(Module::Stage::Pre);
        update_stage(Module::Stage::Normal);
        update_stage(Module::Stage::Post);

        delta_update.update();
    }
}

void Engine::process_rendering() {
    if (elapsed_render.get_elapsed() != 0) {
		std::cerr << "R/" << '\n';
        fps.update(Time::now());

        update_stage(Module::Stage::Render);

        delta_render.update();
    }
}

int32_t Engine::run() {
	while (running) {
		if (app) {
			if (!app->started) {
				app->start();
				app->started = true;
			}
			
			app->update();
		}

		update_stage(Module::Stage::Always);

		process_updates();

		// Prioritize updates over rendering.
		//if (!Maths::AlmostEqual(elapsedUpdate.GetInterval().AsSeconds(), deltaUpdate.change.AsSeconds(), 0.8f)) {
		//	continue;
		//}

		process_rendering();
	}

	return EXIT_SUCCESS;
}

void Engine::create_module(Module::RegistryMapT::const_iterator it) {
	auto const& [module_id, module_data] = *it;
	if (modules.contains(module_id))
		return;

	Log::out("Created module ", module_id.name(), '\n');

	for (auto const depend_id : module_data.depends) {
		if (depend_id != module_id) {
			Log::out("Detected dependency: ", depend_id.name(), " from ", module_id.name(), '\n');
			create_module(Module::registry().find(depend_id));
		}
	}

   	auto&& module_instance = module_data.create();
    modules[module_id] = std::move(module_instance);
    module_stages[module_data.stage].emplace_back(module_id);
}

void Engine::destroy_module(std::type_index id) {
	if (!modules[id])
		return;

	for (auto const& [registrar_id, registrar] : Module::registry()) {
		if (id == registrar_id)
			continue;

		if (std::find(registrar.depends.begin(), registrar.depends.end(), id) != registrar.depends.end())
			destroy_module(registrar_id);
	}
	
	modules[id] = nullptr;
}

void Engine::update_stage(Module::Stage stage) {
	for (auto& module_id : module_stages[stage])
		modules[module_id]->update();
}

void Engine::set_fps_limit(float fps_limit) {
	this->fps_limit = fps_limit;
	elapsed_render.set_interval(Time::seconds(1.0f / fps_limit));
}
}