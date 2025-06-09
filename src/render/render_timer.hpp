#pragma once

#include <data/owner_value.hpp>

namespace xen {
class RenderTimer {
public:
    RenderTimer();
    RenderTimer(RenderTimer const&) = delete;
    RenderTimer(RenderTimer&&) noexcept = default;

    RenderTimer& operator=(RenderTimer const&) = delete;
    RenderTimer& operator=(RenderTimer&&) noexcept = default;

    ~RenderTimer();

    /// Starts the time measure.
    /// \note This action is not available with OpenGL ES and will do nothing.
    void start() const;

    /// Stops the time measure.
    /// \note This action is not available with OpenGL ES and will do nothing.
    void stop() const;

    /// Recovers the elapsed time (in milliseconds) of the latest measure.
    /// \warning The measure must be stopped for the actual time to be available.
    /// \see stop()
    /// \note This action is not available with OpenGL ES and will always return 0.
    /// \return Elapsed time in milliseconds.
    [[nodiscard]] float recover_time() const;

private:
#if !defined(USE_OPENGL_ES)
    OwnerValue<uint32_t, std::numeric_limits<uint32_t>::max()> index{};
#endif
};
}