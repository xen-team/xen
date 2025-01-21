#pragma once

#include <rocket.hpp>

namespace xen {
class App : public virtual rocket::trackable {
	friend class Engine;
public:
	explicit App(std::string name) : name(std::move(name)) {}

	virtual ~App() = default;

	virtual void start() = 0;

	virtual void update() = 0;

	const std::string &get_name() const { return name; }

	void set_name(const std::string &name) { this->name = name; }

private:
	std::string name;
	bool started = false;
};
}