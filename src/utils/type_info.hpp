#pragma once

#include <typeindex>
#include <unordered_map>

namespace xen {
using TypeId = std::size_t;

template <typename T>
class TypeInfo {
public:
    TypeInfo() = delete;

    template <typename K>
    static TypeId GetTypeId() noexcept
        requires(std::is_convertible_v<K*, T*>)
    {
        std::type_index typeIndex(typeid(K));
        if (auto it = type_map.find(typeIndex); it != type_map.end()) {
            return it->second;
        }
        auto const id = next_type_id();
        type_map[typeIndex] = id;
        return id;
    }

private:
    static TypeId next_type_id() noexcept
    {
        auto const id = next_type_id_;
        ++next_type_id_;
        return id;
    }

    static TypeId next_type_id_;
    static std::unordered_map<std::type_index, TypeId> type_map;
};

template <typename K>
TypeId TypeInfo<K>::next_type_id_ = 0;

template <typename K>
std::unordered_map<std::type_index, TypeId> TypeInfo<K>::type_map = {};
}