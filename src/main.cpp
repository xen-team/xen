#include <iostream>

#include "engine/engine.hpp"
#include "system/window_manager.hpp"
using namespace xen;

int main()
{
	Engine engine;

	auto window0 = Windows::get()->add_window();
	[[maybe_unused]] auto window1 = Windows::get()->add_window();
	window0->set_title("Game");
	window0->on_close.connect([]() { Engine::get()->request_close(); });

	auto exit_code = engine.run();
	return exit_code;
}