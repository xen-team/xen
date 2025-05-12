#pragma once
namespace xen {
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(NonCopyable const&) = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;
    ~NonCopyable() = default;
};

class NonMovable {
public:
    NonMovable() = default;
    NonMovable(NonMovable const&) = default;
    NonMovable& operator=(NonMovable const&) = default;
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
    ~NonMovable() = default;
};

class NonCopyableMovable : public NonCopyable, public NonMovable {
protected:
    NonCopyableMovable() = default;
};
}