#pragma once

#include "utils/classes.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <typeindex>

namespace xen {
template<typename Base>
class ModuleFactory {
public:
	class ModuleData {
	public:
		std::function<std::unique_ptr<Base>()> create;
		typename Base::Stage stage;
		std::vector<std::type_index> depends;
	};

	virtual ~ModuleFactory() = default;

    using RegistryMapT = std::unordered_map<std::type_index, ModuleData>;

	static auto& registry() {
		static RegistryMapT impl;
		return impl;
	}

	template<typename... Args>
	class Depends {
	public:
		constexpr std::vector<std::type_index> get() const {
			return { std::type_index(typeid(Args))... };
		}
	};

	template<typename T>
	class Registrar : public Base {
	public:
		virtual ~Registrar() {
			if (static_cast<T*>(this) == module_instance)
				module_instance = nullptr;
		}

		static T* get() { return module_instance; }

	protected:
		template<typename ... Args>
		static bool Register(typename Base::Stage stage, Depends<Args...>&& depends = {}) {
			ModuleFactory::registry()[std::type_index(typeid(T))] = {[]() {
				module_instance = new T();
				return std::unique_ptr<Base>(module_instance);
			}, stage, depends.get()};
			return true;
		}
		
		inline static T* module_instance = nullptr;
	};
};


class Module : public ModuleFactory<Module>, NonCopyable {
public:
	enum class Stage : uint8_t {
		Never, Always, Pre, Normal, Post, Render
	};

	using StageIndex = std::pair<Stage, std::type_index>;

	virtual ~Module() = default;

	virtual void update() = 0;
};
}