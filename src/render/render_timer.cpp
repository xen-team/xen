#include "render_timer.hpp"

#if !defined(USE_OPENGL_ES)
#include <render/renderer.hpp>
#endif

namespace xen {
RenderTimer::RenderTimer()
{
#if !defined(USE_OPENGL_ES)
    Renderer::generate_query(index);
#endif
}

void RenderTimer::start() const
{
#if !defined(USE_OPENGL_ES)
    Renderer::begin_query(QueryType::TIME_ELAPSED, index);
#endif
}

void RenderTimer::stop() const
{
#if !defined(USE_OPENGL_ES)
    Renderer::end_query(QueryType::TIME_ELAPSED);
#endif
}

float RenderTimer::recover_time() const
{
#if !defined(USE_OPENGL_ES)
    int64_t time{};
    Renderer::recover_query_result(index, time);

    return (static_cast<float>(time) / 1'000'000.f);
#else
    return 0;
#endif
}

RenderTimer::~RenderTimer()
{
#if !defined(USE_OPENGL_ES)
    Renderer::delete_query(index);
#endif
}
}