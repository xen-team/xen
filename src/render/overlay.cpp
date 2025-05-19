#include "overlay.hpp"

#include <render/renderer.hpp>
#include <render/texture.hpp>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#include "implot.h"

#include <tracy/Tracy.hpp>

namespace xen {
OverlayWindow& Overlay::add_window(std::string title, Vector2f const& init_size, Vector2f const& init_pos)
{
    return *windows.emplace_back(std::make_unique<OverlayWindow>(std::move(title), init_size, init_pos));
}

OverlayWindow& Overlay::add_window(std::unique_ptr<OverlayWindow>&& window)
{
    *windows.emplace_back(std::move(window));
}

bool Overlay::has_keyboard_focus() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool Overlay::has_mouse_focus() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

void Overlay::render() const
{
    ZoneScopedN("Overlay::render");

#if !defined(USE_OPENGL_ES) && defined(XEN_CONFIG_DEBUG)
    if (Renderer::check_version(4, 3)) {
        Renderer::push_debug_group("Overlay pass");
    }
#endif

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    GImGui->IO.IniFilename = nullptr;

    for (std::unique_ptr<OverlayWindow> const& window : windows) {
        window->render();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#if !defined(USE_OPENGL_ES) && defined(XEN_CONFIG_DEBUG)
    if (Renderer::check_version(4, 3)) {
        Renderer::pop_debug_group();
    }
#endif
}

void Overlay::init(GLFWwindow* window_handle)
{
    ZoneScopedN("Overlay::initialize");

    if (ImGui::GetCurrentContext() != nullptr) {
        return; // The overlay has already been initialized
    }

    Log::debug("[Overlay] Initializing...");

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window_handle, false);

#if !defined(XEN_IS_PLATFORM_EMSCRIPTEN)
    ImGui_ImplOpenGL3_Init("#version 330 core");
#else
    ImGui_ImplOpenGL3_Init("#version 300 es");
#endif

    Log::debug("[Overlay] Initialized");
}

void Overlay::destroy()
{
    ZoneScopedN("Overlay::destroy");

    if (ImGui::GetCurrentContext() == nullptr) {
        return; // The overlay has already been destroyed
    }

    Log::debug("[Overlay] Destroying...");

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    Log::debug("[Overlay] Destroyed");
}

void OverlayColoredLabel::set_color(Color const& color, float alpha)
{
    this->color = color;
    this->alpha = alpha;
}

void OverlayTextbox::set_label(std::string text)
{
    this->text = std::move(text);
    callback(this->text);
}

OverlayTextbox& OverlayTextbox::append(std::string const& text)
{
    this->text += text;
    callback(this->text);

    return *this;
}

void OverlayTextbox::clear()
{
    text.clear();
    callback(text);
}

void OverlayTextArea::set_label(std::string text)
{
    this->text = std::move(text);
    callback(this->text);
}

OverlayTextArea& OverlayTextArea::append(std::string const& text)
{
    this->text += text;
    callback(this->text);

    return *this;
}

void OverlayTextArea::clear()
{
    text.clear();
    callback(text);
}

OverlayTexture::OverlayTexture(Texture2D const& texture) : OverlayTexture(texture, texture.get_size()) {}

void OverlayTexture::set_texture(Texture2D const& texture, Vector2ui const& max_size)
{
    index = texture.get_index();
    size = static_cast<Vector2f>(max_size);
}

void OverlayTexture::set_texture(Texture2D const& texture)
{
    set_texture(texture, texture.get_size());
}

OverlayPlotEntry& OverlayPlot::add_entry(std::string name, OverlayPlotType type)
{
    return *entries.emplace_back(
        std::make_unique<OverlayPlotEntry>(OverlayPlotEntry(std::move(name), type, max_value_count))
    );
}

OverlayWindow::OverlayWindow(std::string title, Vector2f const& init_size, Vector2f const& init_pos) :
    title{std::move(title)}, current_size{init_size}, current_pos{init_pos}
{
    Log::rt_assert(!this->title.empty(), "Error: The overlay window title cannot be empty.");
}

OverlayLabel& OverlayWindow::add_label(std::string label)
{
    return static_cast<OverlayLabel&>(*elements.emplace_back(std::make_unique<OverlayLabel>(std::move(label))));
}

OverlayColoredLabel& OverlayWindow::add_colored_label(std::string label, Color const& color)
{
    return add_colored_label(std::move(label), color);
}

OverlayButton& OverlayWindow::add_button(std::string label, std::function<void()> action_click)
{
    return static_cast<OverlayButton&>(
        *elements.emplace_back(std::make_unique<OverlayButton>(std::move(label), std::move(action_click)))
    );
}

OverlayCheckbox& OverlayWindow::add_checkbox(
    std::string label, std::function<void()> action_on, std::function<void()> action_off, bool init_value
)
{
    return static_cast<OverlayCheckbox&>(*elements.emplace_back(
        std::make_unique<OverlayCheckbox>(std::move(label), std::move(action_on), std::move(action_off), init_value)
    ));
}

OverlaySlider& OverlayWindow::add_slider(
    std::string label, std::function<void(float)> action_slide, Vector2f const& range, float init_value
)
{
    return static_cast<OverlaySlider&>(*elements.emplace_back(
        std::make_unique<OverlaySlider>(std::move(label), std::move(action_slide), range, init_value)
    ));
}

OverlayTextbox&
OverlayWindow::add_textbox(std::string label, std::function<void(std::string const&)> callback, std::string init_text)
{
    auto& textbox = static_cast<OverlayTextbox&>(*elements.emplace_back(
        std::make_unique<OverlayTextbox>(std::move(label), std::move(callback), std::move(init_text))
    ));
    textbox.text.reserve(std::min(textbox.text.size(), static_cast<size_t>(64)));
    return textbox;
}

OverlayTextArea& OverlayWindow::add_text_area(
    std::string label, std::function<void(std::string const&)> callback, std::string init_text, float max_height
)
{
    auto& textArea = static_cast<OverlayTextArea&>(*elements.emplace_back(
        std::make_unique<OverlayTextArea>(std::move(label), std::move(callback), std::move(init_text), max_height)
    ));
    textArea.text.reserve(std::min(textArea.text.size(), static_cast<size_t>(2048)));
    return textArea;
}

OverlayListBox& OverlayWindow::add_list_box(
    std::string label, std::vector<std::string> entries, std::function<void(std::string const&, size_t)> action_changed,
    size_t init_id
)
{
    if (entries.empty()) {
        throw std::invalid_argument("[Overlay] Cannot create a list box with no entry");
    }

    Log::rt_assert(
        init_id < entries.size(), "Error: A list box's initial index cannot reference a non-existing entry."
    );
    return static_cast<OverlayListBox&>(*elements.emplace_back(
        std::make_unique<OverlayListBox>(std::move(label), std::move(entries), std::move(action_changed), init_id)
    ));
}

OverlayDropdown& OverlayWindow::add_dropdown(
    std::string label, std::vector<std::string> entries, std::function<void(std::string const&, size_t)> action_changed,
    size_t init_id
)
{
    if (entries.empty()) {
        throw std::invalid_argument("[Overlay] Cannot create a dropdown list with no entry");
    }

    Log::rt_assert(
        init_id < entries.size(), "Error: A dropdown's initial index cannot reference a non-existing entry."
    );
    return static_cast<OverlayDropdown&>(*elements.emplace_back(
        std::make_unique<OverlayDropdown>(std::move(label), std::move(entries), std::move(action_changed), init_id)
    ));
}

OverlayColorPicker& OverlayWindow::add_color_picker(
    std::string label, std::function<void(Color const&)> action_changed, Color const& init_color
)
{
    return static_cast<OverlayColorPicker&>(*elements.emplace_back(
        std::make_unique<OverlayColorPicker>(std::move(label), std::move(action_changed), init_color)
    ));
}

OverlayTexture& OverlayWindow::add_texture(Texture2D const& texture, Vector2ui const& max_size)
{
    return static_cast<OverlayTexture&>(*elements.emplace_back(std::make_unique<OverlayTexture>(texture, max_size)));
}

OverlayTexture& OverlayWindow::add_texture(Texture2D const& texture)
{
    return static_cast<OverlayTexture&>(*elements.emplace_back(std::make_unique<OverlayTexture>(texture)));
}

OverlayProgressBar& OverlayWindow::add_progress_bar(Vector2i const& range, bool show_values)
{
    return static_cast<OverlayProgressBar&>(
        *elements.emplace_back(std::make_unique<OverlayProgressBar>(range, show_values))
    );
}

OverlayPlot& OverlayWindow::add_plot(
    std::string label, size_t max_value_count, std::string x_axis_label, std::string y_axis_label, Vector2f value_range,
    bool lock_y_axis, float max_height
)
{
    return static_cast<OverlayPlot&>(*elements.emplace_back(std::make_unique<OverlayPlot>(
        std::move(label), max_value_count, std::move(x_axis_label), std::move(y_axis_label), value_range, lock_y_axis,
        max_height
    )));
}

OverlaySeparator& OverlayWindow::add_separator()
{
    return static_cast<OverlaySeparator&>(*elements.emplace_back(std::make_unique<OverlaySeparator>()));
}

OverlayFrameTime& OverlayWindow::add_frame_time(std::string formatted_label)
{
    return static_cast<OverlayFrameTime&>(
        *elements.emplace_back(std::make_unique<OverlayFrameTime>(std::move(formatted_label)))
    );
}

OverlayFpsCounter& OverlayWindow::add_fps_counter(std::string formatted_label)
{
    return static_cast<OverlayFpsCounter&>(
        *elements.emplace_back(std::make_unique<OverlayFpsCounter>(std::move(formatted_label)))
    );
}

void OverlayWindow::render() const
{
    ZoneScopedN("OverlayWindow::render");

    if (!enabled) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(current_size.x, current_size.y), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(current_pos.x, current_pos.y), ImGuiCond_Once);
    ImGui::Begin(
        title.c_str(), nullptr,
        (current_size.x < 0.f && current_size.y < 0.f ? ImGuiWindowFlags_AlwaysAutoResize : ImGuiWindowFlags_None)
    );

    for (auto const& element : elements) {
        if (!element->is_enabled()) {
            continue;
        }

        switch (element->get_type()) {
        case OverlayElementType::LABEL:
            ImGui::PushTextWrapPos();
            ImGui::TextUnformatted(element->label.c_str());
            ImGui::PopTextWrapPos();
            break;

        case OverlayElementType::COLORED_LABEL: {
            auto const& label = static_cast<OverlayColoredLabel&>(*element);
            ImGui::PushTextWrapPos();

            Vector3f const& colorVec = label.color;
            ImGui::TextColored(ImVec4(colorVec.x, colorVec.y, colorVec.z, label.alpha), "%s", element->label.c_str());

            ImGui::PopTextWrapPos();
            break;
        }

        case OverlayElementType::BUTTON: {
            auto const& button = static_cast<OverlayButton&>(*element);

            if (ImGui::Button(button.label.c_str())) {
                button.action_click();
            }

            break;
        }

        case OverlayElementType::CHECKBOX: {
            auto& checkbox = static_cast<OverlayCheckbox&>(*element);
            bool const prevValue = checkbox.is_checked;

            ImGui::Checkbox(checkbox.label.c_str(), &checkbox.is_checked);

            if (checkbox.is_checked != prevValue) {
                if (checkbox.is_checked) {
                    checkbox.action_on();
                }
                else {
                    checkbox.action_off();
                }
            }

            break;
        }

        case OverlayElementType::SLIDER: {
            auto& slider = static_cast<OverlaySlider&>(*element);

            ImGui::SetNextItemWidth(std::min(ImGui::CalcItemWidth(), 210.f));
            if (ImGui::SliderFloat(slider.label.c_str(), &slider.value, slider.range.x, slider.range.y))
                slider.action_slide(slider.value);

            break;
        }

        case OverlayElementType::TEXTBOX: {
            auto& textbox = static_cast<OverlayTextbox&>(*element);

            if (ImGui::InputText(textbox.label.c_str(), &textbox.text))
                textbox.callback(textbox.text);

            break;
        }

        case OverlayElementType::TEXT_AREA: {
            auto& textArea = static_cast<OverlayTextArea&>(*element);

            if (ImGui::InputTextMultiline(
                    textArea.label.c_str(), &textArea.text, ImVec2(-1.f, textArea.max_height),
                    ImGuiInputTextFlags_AllowTabInput
                )) {
                textArea.callback(textArea.text);
            }

            break;
        }

        case OverlayElementType::LIST_BOX: {
            auto& listBox = static_cast<OverlayListBox&>(*element);

            // The list box will get a default width, while being automatically resized vertically up to 5 elements
            // The stride added is to avoid showing a scrollbar when having few entries. Its value is directly defined
            // here,
            //  but may be required to be ImGui::GetStyle().ItemSpacing.y / 2
            ImVec2 const dimensions(
                0,
                ImGui::GetTextLineHeightWithSpacing() * std::min(static_cast<float>(listBox.entries.size()), 5.f) + 2.f
            );

            if (ImGui::BeginListBox(listBox.label.c_str(), dimensions)) {
                for (size_t i = 0; i < listBox.entries.size(); ++i) {
                    bool const isSelected = (listBox.current_id == i);

                    if (ImGui::Selectable(listBox.entries[i].c_str(), isSelected)) {
                        if (!isSelected) { // If the item isn't already selected
                            listBox.action_changed(listBox.entries[i], i);
                            listBox.current_id = i;
                        }
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndListBox();
            }

            break;
        }

        case OverlayElementType::DROPDOWN: {
            auto& dropdown = static_cast<OverlayDropdown&>(*element);

            if (ImGui::BeginCombo(dropdown.label.c_str(), dropdown.entries[dropdown.current_id].c_str())) {
                for (size_t i = 0; i < dropdown.entries.size(); ++i) {
                    bool const isSelected = (dropdown.current_id == i);

                    if (ImGui::Selectable(dropdown.entries[i].c_str(), isSelected)) {
                        if (!isSelected) { // If the item isn't already selected
                            dropdown.action_changed(dropdown.entries[i], i);
                            dropdown.current_id = i;
                        }
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            break;
        }

        case OverlayElementType::COLOR_PICKER: {
            auto& colorPicker = static_cast<OverlayColorPicker&>(*element);

            if (ImGui::ColorEdit3(colorPicker.label.c_str(), colorPicker.color.data()))
                colorPicker.action_changed(Color(colorPicker.color[0], colorPicker.color[1], colorPicker.color[2]));

            break;
        }

        case OverlayElementType::TEXTURE: {
            auto& texture = static_cast<OverlayTexture&>(*element);
            Log::rt_assert(Renderer::is_texture(texture.index), "Error: The given texture is invalid.");

            float const minRatio =
                std::min(ImGui::GetWindowWidth() / texture.size.x, ImGui::GetWindowHeight() / texture.size.y);
            ImVec2 const textureSize(
                std::min(texture.size.x, texture.size.x * minRatio), std::min(texture.size.y, texture.size.y * minRatio)
            );

            // The UV's y component must be reverted, so that the texture isn't flipped upside down
            ImVec2 const topCoords(0.f, 1.f);
            ImVec2 const bottomCoords(1.f, 0.f);

            ImGui::Image(
                reinterpret_cast<void*>(static_cast<intptr_t>(texture.index)), textureSize, topCoords, bottomCoords
            );

            break;
        }

        case OverlayElementType::PROGRESS_BAR: {
            auto& progressBar = static_cast<OverlayProgressBar&>(*element);

            std::string const text =
                (progressBar.show_values ?
                     std::to_string(progressBar.value) + '/' + std::to_string(progressBar.range.y) :
                     std::string());
            ImGui::ProgressBar(
                static_cast<float>(progressBar.range.x + progressBar.value) / static_cast<float>(progressBar.range.y),
                ImVec2(-1.f, 0.f), (text.empty() ? nullptr : text.c_str())
            );

            break;
        }

        case OverlayElementType::PLOT: {
            auto& plot = static_cast<OverlayPlot&>(*element);

            if (ImPlot::BeginPlot(
                    plot.label.c_str(), ImVec2(-1, plot.max_height), ImPlotFlags_NoMenus | ImPlotFlags_NoBoxSelect
                )) {
                ImPlot::SetupAxes(
                    plot.x_axis_label.c_str(), plot.y_axis_label.c_str(), ImPlotAxisFlags_NoTickLabels,
                    (plot.lock_y ? ImPlotAxisFlags_Lock : ImPlotAxisFlags_None)
                );
                ImPlot::SetupAxisLimits(
                    ImAxis_X1, 0.0, static_cast<double>(plot.max_value_count - 1), ImPlotCond_Always
                );
                ImPlot::SetupAxisLimits(
                    ImAxis_Y1, static_cast<double>(plot.value_range.x), static_cast<double>(plot.value_range.y),
                    ImPlotCond_Once
                );
                ImPlot::SetupMouseText(ImPlotLocation_NorthEast);

                for (std::unique_ptr<OverlayPlotEntry> const& entry : plot.entries) {
                    if (entry->type == OverlayPlotType::SHADED) {
                        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
                        ImPlot::PlotShaded(
                            entry->name.c_str(), entry->values.data(), static_cast<int>(entry->values.size())
                        );
                    }
                    else {
                        ImPlot::PlotLine(
                            entry->name.c_str(), entry->values.data(), static_cast<int>(entry->values.size())
                        );
                    }
                }

                ImPlot::EndPlot();
            }

            break;
        }

        case OverlayElementType::SEPARATOR:
            ImGui::Separator();
            break;

        case OverlayElementType::FRAME_TIME:
            ImGui::Text(element->label.c_str(), static_cast<double>(1000.f / ImGui::GetIO().Framerate));
            break;

        case OverlayElementType::FPS_COUNTER:
            ImGui::Text(element->label.c_str(), static_cast<double>(ImGui::GetIO().Framerate));
            break;

        default:
            break;
        }
    }

    ImGui::End();
}

}
