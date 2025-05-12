#pragma once

namespace xen::TypeUtils {
template <typename... Args>
struct ConstOverload {
    template <typename RetT, typename T>
    constexpr auto operator()(RetT (T::*ptr)(Args...) const) const
    {
        return ptr;
    }
};

template <typename... Args>
struct NonConstOverload {
    template <typename RetT, typename T>
    constexpr auto operator()(RetT (T::*ptr)(Args...)) const
    {
        return ptr;
    }
};

template <typename... Args>
struct Overload : ConstOverload<Args...>, NonConstOverload<Args...> {
    using ConstOverload<Args...>::operator();
    using NonConstOverload<Args...>::operator();

    template <typename RetT>
    constexpr auto operator()(RetT (*ptr)(Args...)) const
    {
        return ptr;
    }
};

template <typename... Args>
constexpr ConstOverload<Args...> PickConstOverload{};
template <typename... Args>
constexpr NonConstOverload<Args...> PickNonConstOverload{};
template <typename... Args>
constexpr Overload<Args...> PickOverload{};

/// Recovers a string of the given type's name at compile-time.
/// \tparam T Type to recover the name of.
/// \return String representing the type's name.
template <typename T>
constexpr std::string_view get_type_str()
{
#if defined(__clang__)
    // Has the form "std::string_view Raz::TypeUtils::get_type_str() [T = ...]"

    constexpr std::string_view func_signature = __PRETTY_FUNCTION__;
    constexpr size_t start_stride = std::size("std::string_view Raz::TypeUtils::get_type_str() [T = ") - 1;

    return std::string_view(func_signature.data() + start_stride, func_signature.size() - start_stride - 1);

    // The following implementation would be ideal, but can't be resolved at compile-time (yet?)
    // See:
    // https://stackoverflow.com/questions/56484834/constexpr-stdstring-viewfind-last-of-doesnt-work-on-clang-8-with-libstdc

//  constexpr size_t first_pos = func_signature.find_first_of('=');
//  static_assert(first_pos != std::string_view::npos, "Error: Character '=' not found in the function's signature.");
//
//  constexpr size_t last_pos = func_signature.find_last_of(']');
//  static_assert(last_pos != std::string_view::npos, "Error: Character ']' not found in the function's signature.");
//
//  static_assert(first_pos < last_pos, "Error: Trailing character found before leading one in the function's
//  signature.");
//
//  return std::string_view(func_signature.data() + first_pos + 2, last_pos - first_pos - 2);
#elif defined(__GNUC__)
    // Has the form "constexpr std::string_view Raz::TypeUtils::get_type_str() [with T = ...; std::string_view =
    // std::basic_string_view<char>]"

    constexpr std::string_view func_signature = __PRETTY_FUNCTION__;

    constexpr size_t first_pos = func_signature.find_first_of('=');
    static_assert(first_pos != std::string_view::npos, "Error: Character '=' not found in the function's signature.");

    constexpr size_t last_pos = func_signature.find_first_of(';', first_pos);
    static_assert(last_pos != std::string_view::npos, "Error: Character ';' not found in the function's signature.");

    static_assert(
        first_pos < last_pos, "Error: Trailing character found before leading one in the function's signature."
    );

    return std::string_view(func_signature.data() + first_pos + 2, last_pos - first_pos - 2);
#elif defined(_MSC_VER)
    // Has the form "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl
    // Raz::TypeUtils::get_type_str<...>(void)"

    constexpr std::string_view func_signature = __FUNCSIG__;

    constexpr size_t start_stride = std::size("class std::basic_string_view<char,struct std::char_traits<char> > "
                                              "__cdecl Raz::TypeUtils::get_type_str<") -
                                    1;
    constexpr size_t end_stride = std::size(">(void)") - 1;

    return std::string_view(func_signature.data() + start_stride, func_signature.size() - start_stride - end_stride);

    // The following implementation would be ideal, but can't be resolved at compile-time because of a bug fixed
    // only recently See:
    // https://developercommunity.visualstudio.com/content/problem/331234/stdbasic-string-viewfind-is-not-always-constexpr.html

//  constexpr std::string_view funcName = __FUNCTION__;
//
//  constexpr size_t first_pos = func_signature.find(funcName);
//  static_assert(first_pos != std::string_view::npos, "Error: Function name not found in the function's signature.");
//
//  constexpr size_t last_pos = func_signature.find_last_of('>');
//  static_assert(last_pos != std::string_view::npos, "Error: Character '>' not found in the function's signature.");
//
//  static_assert(first_pos < last_pos, "Error: Trailing character found before leading one in the function's
//  signature.");
//
//  return std::string_view(func_signature.data() + first_pos + funcName.size(), last_pos - first_pos -
//  funcName.size());
#else
#pragma message("Warning: The current compiler is not supported.")
    return {};
#endif
}

/// Recovers a string of the given enumeration value's name at compile-time.
/// \tparam Enum Enumeration value to recover the name of.
/// \return String representing the enum value's name.
template <auto Enum>
constexpr std::string_view get_enum_str()
{
    static_assert(std::is_enum_v<decltype(Enum)>, "Error: The type of the given value must be an enum.");

#if defined(__clang__)
    // Has the form "std::string_view Raz::TypeUtils::get_enum_str() [Enum = ...]"

    constexpr std::string_view func_signature = __PRETTY_FUNCTION__;
    constexpr size_t start_stride = std::size("std::string_view Raz::TypeUtils::get_enum_str() [Enum = ") - 1;

    return std::string_view(func_signature.data() + start_stride, func_signature.size() - start_stride - 1);
#elif defined(__GNUC__)
    // Has the form "constexpr std::string_view Raz::TypeUtils::get_enum_str() [with auto Enum = ...; std::string_view
    // = std::basic_string_view<char>]"

    constexpr std::string_view func_signature = __PRETTY_FUNCTION__;

    constexpr size_t first_pos = func_signature.find_first_of('=');
    static_assert(first_pos != std::string_view::npos, "Error: Character '=' not found in the function's signature.");

    constexpr size_t last_pos = func_signature.find_first_of(';', first_pos);
    static_assert(last_pos != std::string_view::npos, "Error: Character ';' not found in the function's signature.");

    static_assert(
        first_pos < last_pos, "Error: Trailing character found before leading one in the function's signature."
    );

    return std::string_view(func_signature.data() + first_pos + 2, last_pos - first_pos - 2);
#elif defined(_MSC_VER)
    // Has the form "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl
    // Raz::TypeUtils::get_enum_str<...>(void)"

    constexpr std::string_view func_signature = __FUNCSIG__;

    constexpr size_t start_stride =
        std::size(
            "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl Raz::TypeUtils::get_enum_str<"
        ) -
        1;
    constexpr size_t end_stride = std::size(">(void)") - 1;

    return std::string_view(func_signature.data() + start_stride, func_signature.size() - start_stride - end_stride);
#else
#pragma message("Warning: The current compiler is not supported.")
    return {};
#endif
}

// Custom implementation of std::experimental::is_detected
// See: https://en.cppreference.com/w/cpp/experimental/is_detected

// TODO: this implementation doesn't work with MSVC (which always returns true for every attribute)
// See: https://stackoverflow.com/a/35755737/3292304

#if !defined(_MSC_VER) || defined(__clang__) // Ignoring exclusively MSVC, not clang-cl
namespace Details {
template <typename Default, typename AlwaysVoid, template <typename...> typename Attr, typename... Args>
struct Detector {
    using ValueT = std::false_type;
    using Type = Default;
};

template <typename Default, template <typename...> typename Attr, typename... Args>
struct Detector<Default, std::void_t<Attr<Args...>>, Attr, Args...> {
    using ValueT = std::true_type;
    using Type = Attr<Args...>;
};

struct Nonesuch {
    Nonesuch(Nonesuch const&) = delete;
    void operator=(Nonesuch const&) = delete;
    ~Nonesuch() = delete;
};

template <template <typename...> typename Attr, typename... Args>
using IsDetectedT = typename Details::Detector<Nonesuch, void, Attr, Args...>::ValueT;

template <template <typename...> typename Attr, typename... Args>
using DetectedT = typename Details::Detector<Nonesuch, void, Attr, Args...>::Type;

template <typename Default, template <typename...> typename Attr, typename... Args>
using DetectedOr = Details::Detector<Default, void, Attr, Args...>;

template <typename Default, template <typename...> typename Attr, typename... Args>
using DetectedOrT = typename DetectedOr<Default, Attr, Args...>::Type;

template <typename Expected, template <typename...> typename Attr, typename... Args>
using IsDetectedExactT = std::is_same<Expected, DetectedT<Attr, Args...>>;

template <typename To, template <typename...> typename Attr, typename... Args>
using IsDetectedConvertibleT = std::is_convertible<DetectedT<Attr, Args...>, To>;
}

namespace Attribute {
template <typename T>
using DefaultConstructor = decltype(T());

template <typename T>
using CopyConstructor = decltype(T(std::declval<T const&>()));

template <typename T>
using MoveConstructor = decltype(T(std::declval<T&&>()));

template <typename T>
using CopyAssignmentOperator = decltype(std::declval<T&>() = std::declval<T const&>());

template <typename T>
using MoveAssignmentOperator = decltype(std::declval<T&>() = std::declval<T&&>());

template <typename T, typename U = T>
using EqualityOperator = decltype(std::declval<T>() == std::declval<U>());

template <typename T, typename U = T>
using InequalityOperator = decltype(std::declval<T>() != std::declval<U>());

template <typename T, typename U = T>
using PlusOperator = decltype(std::declval<T>() + std::declval<U>());

template <typename T, typename U = T>
using MinusOperator = decltype(std::declval<T>() - std::declval<U>());

template <typename T, typename U = T>
using MultiplicationOperator = decltype(std::declval<T>() * std::declval<U>());

template <typename T, typename U = T>
using DivisionOperator = decltype(std::declval<T>() / std::declval<U>());

template <typename T>
using DefaultDestructor = decltype(std::declval<T&>().~T());
}

/// Checks if the given types make available an attribute.
/// \note Non-accessible attributes (private/protected) cannot be detected.
/// \tparam Attr Attribute to be checked.
/// \tparam Args Types in which to check the attribute's existence.
/// \return True if the attribute exists & is accessible, false otherwise.
template <template <typename...> typename Attr, typename... Args>
constexpr bool has_attribute()
{
    return Details::IsDetectedT<Attr, Args...>::value;
}

/// Checks if the given types' attribute returns the specific expected type.
/// \tparam Expected Return type to be checked.
/// \tparam Attr Attribute to be checked.
/// \tparam Args Types for which to check the attribute's return type.
/// \return True if the attribute's return type is the same as expected, false otherwise.
template <typename Expected, template <typename...> typename Attr, typename... Args>
constexpr bool has_return_type()
{
    return Details::IsDetectedExactT<Expected, Attr, Args...>::value;
}

/// Checks if the given types' attribute return type is convertible to the given one.
/// \tparam To Convertible return type to be checked.
/// \tparam Attr Attribute to be checked.
/// \tparam Args Types for which to check the attribute's return type.
/// \return True if the attribute's return type is convertible, false otherwise.
template <typename To, template <typename...> typename Attr, typename... Args>
constexpr bool has_return_type_convertible()
{
    return Details::IsDetectedConvertibleT<To, Attr, Args...>::value;
}

/// Checks if the default constructor is available for the given type.
/// \tparam T Type to check the attribute for.
/// \return True if the default constructor is available, false otherwise.
template <typename T>
constexpr bool has_default_constructor()
{
    return has_attribute<Attribute::DefaultConstructor, T>();
}

/// Checks if the copy constructor is available for the given type.
/// \tparam T Type to check the attribute for.
/// \return True if the copy constructor is available, false otherwise.
template <typename T>
constexpr bool has_copy_constructor()
{
    return has_attribute<Attribute::CopyConstructor, T>();
}

/// Checks if the move constructor is available for the given type.
/// \tparam T Type to check the attribute for.
/// \return True if the move constructor is available, false otherwise.
template <typename T>
constexpr bool has_move_constructor()
{
    return has_attribute<Attribute::MoveConstructor, T>();
}

/// Checks if the copy assignment operator is available for the given type.
/// \tparam T Type to check the attribute for.
/// \return True if the copy assignment operator is available, false otherwise.
template <typename T>
constexpr bool has_copy_assignment_operator()
{
    return has_attribute<Attribute::CopyAssignmentOperator, T>();
}

/// Checks if the move assignment operator is available for the given type.
/// \tparam T Type to check the attribute for.
/// \return True if the move assignment operator is available, false otherwise.
template <typename T>
constexpr bool has_move_assignment_operator()
{
    return has_attribute<Attribute::MoveAssignmentOperator, T>();
}

/// Checks if the equality operator is available for the given type.
/// \tparam T Type to check the attribute for.
/// \return True if the equality operator is available, false otherwise.
template <typename T>
constexpr bool has_equality_operator()
{
    return has_attribute<Attribute::EqualityOperator, T>();
}

/// Checks if the inequality operator is available for the given type.
/// \tparam T Type to check the attribute for.
/// \return True if the inequality operator is available, false otherwise.
template <typename T>
constexpr bool has_inequality_operator()
{
    return has_attribute<Attribute::InequalityOperator, T>();
}

/// Checks if the default destructor is available for the given type.
/// \tparam T Type to check the attribute for.
/// \return True if the default destructor is available, false otherwise.
template <typename T>
constexpr bool has_default_destructor()
{
    return has_attribute<Attribute::DefaultDestructor, T>();
}
#endif
}