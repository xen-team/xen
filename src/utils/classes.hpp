#pragma once

class NonCopyable
{
protected:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

class NonMovable
{
protected:
    NonMovable() = default;
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

class NonCopyableMovable : public NonCopyable, public NonMovable
{
protected:
    NonCopyableMovable() = default;
};