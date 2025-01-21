
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui_extension.hpp>

void ImGui::AspectRatioImage(ImTextureID user_texture_id, ImVec2 image_size, const float ratio, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
    ImVec2 new_size = image_size;

    if (new_size.x / new_size.y > ratio) {
        new_size.x = new_size.y * ratio;
    } else {
        new_size.y = new_size.x / ratio;
    }

    ImVec2 pos = ImGui::GetCursorPos() + ImVec2((image_size.x - new_size.x) * 0.5f, (image_size.y - new_size.y) * 0.5f);
    ImGui::SetCursorPos(pos);
    
    ImGui::Image(user_texture_id, new_size, uv0, uv1, tint_col, border_col);
}

void ImGui::TextCentered(std::string_view text) {
    ImVec2 const winsize = ImGui::GetWindowSize();
    ImVec2 const textsize = ImGui::CalcTextSize(text.data());

    ImGui::SetCursorPos((winsize - textsize) * 0.5f);
    ImGui::Text("%s", text.data());
}