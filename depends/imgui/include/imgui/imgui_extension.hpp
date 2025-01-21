#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <string>

namespace ImGui {
    void AspectRatioImage(ImTextureID user_texture_id, ImVec2 image_size, const float ratio = 16.f/9.f, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

    void TextCentered(std::string_view data);
}