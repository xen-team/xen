#pragma once

#include <imgui.h>
struct GLFWwindow;

namespace xen {
class OverlayWindow;
class Texture2D;

enum class OverlayElementType {
    LABEL,
    COLORED_LABEL,
    BUTTON,
    CHECKBOX,
    SLIDER,
    TEXTBOX,
    TEXT_AREA,
    LIST_BOX,
    DROPDOWN,
    COLOR_PICKER,
    TEXTURE,
    PROGRESS_BAR,
    PLOT,
    SEPARATOR,
    FRAME_TIME,
    FPS_COUNTER
};

enum class OverlayPlotType { LINE, SHADED };

/// Overlay class, used to render GUI elements (labels, buttons, checkboxes, ...) into a Window.
class Overlay {
    friend OverlayWindow;
    friend class Window;

public:
    Overlay() = default;
    Overlay(Overlay const&) = delete;
    Overlay(Overlay&&) noexcept = default;
    Overlay& operator=(Overlay const&) = delete;
    Overlay& operator=(Overlay&&) noexcept = default;

    bool empty() const { return windows.empty(); }

    /// Adds a new overlay window.
    /// \param title Window title.
    /// \param init_size Initial window size. If both X & Y are strictly lower than 0, automatically resizes the window
    /// from its content.
    /// \param init_pos Initial window position.
    /// \return The newly added window.
    [[nodiscard]] OverlayWindow*
    add_window(std::string title, Vector2f const& init_size = Vector2f(0.f), Vector2f const& init_pos = Vector2f(0.f));

    [[nodiscard]] OverlayWindow* add_window(std::unique_ptr<OverlayWindow>&& window);

    /// Checks if the overlay is currently requesting the keyboard inputs.
    /// \return True if the keyboard focus is taken, false otherwise.
    bool has_keyboard_focus() const;

    /// Checks if the overlay is currently requesting the mouse inputs.
    /// \return True if the mouse focus is taken, false otherwise.
    bool has_mouse_focus() const;

    /// Renders the overlay.
    void render() const;

    static auto& get_fonts()
    {
        static std::vector<ImFont*> fonts;
        return fonts;
    }

private:
    std::vector<std::unique_ptr<OverlayWindow>> windows{};

private:
    /// Initializes ImGui with the containing window.
    /// \param window_handle Handle to initialize the overlay with.
    static void init(GLFWwindow* window_handle);

    /// Destroys the overlay.
    static void destroy();
};

class OverlayElement {
    friend OverlayWindow;

public:
    explicit OverlayElement(std::string label) : label{std::move(label)} {}

    OverlayElement() = default;
    OverlayElement(OverlayElement const&) = delete;
    OverlayElement(OverlayElement&&) = delete;
    OverlayElement& operator=(OverlayElement const&) = delete;
    OverlayElement& operator=(OverlayElement&&) = delete;

    virtual ~OverlayElement() = default;

    virtual OverlayElementType get_type() const = 0;

    bool is_enabled() const { return enabled; }

    void enable(bool enabled = true) { this->enabled = enabled; }

    void disable() { enable(false); }

protected:
    std::string label{};
    bool enabled = true;
};

class OverlayLabel final : public OverlayElement {
    friend OverlayWindow;

public:
    explicit OverlayLabel(std::string label) : OverlayElement(std::move(label)) {}

    OverlayElementType get_type() const override { return OverlayElementType::LABEL; }

    std::string const& get_label() const { return label; }

    void set_label(std::string text) { label = std::move(text); }
};

class OverlayColoredLabel final : public OverlayElement {
    friend OverlayWindow;

public:
    explicit OverlayColoredLabel(std::string label, Color const& color, float alpha = 1.f) :
        OverlayElement(std::move(label)), color{color}, alpha{alpha}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::COLORED_LABEL; }

    std::string const& get_label() const { return label; }

    Color const& get_color() const { return color; }

    float get_alpha() const { return alpha; }

    void set_label(std::string text) { label = std::move(text); }

    void set_color(Color const& color, float alpha = 1.f);

    void set_alpha(float alpha) { this->alpha = alpha; }

private:
    Color color{};
    float alpha = 1.f;
};

class OverlayButton final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayButton(std::string label, std::function<void()> action_click) :
        OverlayElement(std::move(label)), action_click{std::move(action_click)}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::BUTTON; }

private:
    std::function<void()> action_click{};
};

class OverlayCheckbox final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayCheckbox(
        std::string label, std::function<void()> action_on, std::function<void()> action_off, bool init_value
    ) :
        OverlayElement(std::move(label)), action_on{std::move(action_on)}, action_off{std::move(action_off)},
        is_checked{init_value}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::CHECKBOX; }

private:
    std::function<void()> action_on{};
    std::function<void()> action_off{};
    bool is_checked{};
};

class OverlaySlider final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlaySlider(std::string label, std::function<void(float)> action_slide, Vector2f const& range, float init_value) :
        OverlayElement(std::move(label)), action_slide{std::move(action_slide)}, range{range}, value{init_value}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::SLIDER; }

private:
    std::function<void(float)> action_slide{};
    Vector2f range{};
    float value{};
};

class OverlayTextbox final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayTextbox(std::string label, std::function<void(std::string const&)> callback, std::string init_text = {}) :
        OverlayElement(std::move(label)), text{std::move(init_text)}, callback{std::move(callback)}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::TEXTBOX; }

    std::string const& get_label() const { return text; }

    void set_label(std::string text);

    OverlayTextbox& append(std::string const& text);

    void clear();

    OverlayTextbox& operator+=(std::string const& text) { return append(text); }

private:
    std::string text{};
    std::function<void(std::string const&)> callback{};
};

class OverlayTextArea final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayTextArea(
        std::string label, std::function<void(std::string const&)> callback, std::string init_text = {},
        float max_height = -1.f
    ) :
        OverlayElement(std::move(label)), text{std::move(init_text)}, callback{std::move(callback)},
        max_height{max_height}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::TEXT_AREA; }

    std::string const& get_label() const { return text; }

    void set_label(std::string text);

    OverlayTextArea& append(std::string const& text);

    void clear();

    OverlayTextArea& operator+=(std::string const& text) { return append(text); }

private:
    std::string text{};
    std::function<void(std::string const&)> callback{};
    float max_height{};
};

class OverlayListBox final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayListBox(
        std::string label, std::vector<std::string> entries,
        std::function<void(std::string const&, size_t)> action_changed, size_t init_id = 0
    ) :
        OverlayElement(std::move(label)), entries{std::move(entries)}, action_changed{std::move(action_changed)},
        current_id{init_id}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::LIST_BOX; }

private:
    std::vector<std::string> entries{};
    std::function<void(std::string const&, size_t)> action_changed{};
    size_t current_id{};
};

class OverlayDropdown final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayDropdown(
        std::string label, std::vector<std::string> entries,
        std::function<void(std::string const&, size_t)> action_changed, size_t init_id = 0
    ) :
        OverlayElement(std::move(label)), entries{std::move(entries)}, action_changed{std::move(action_changed)},
        current_id{init_id}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::DROPDOWN; }

private:
    std::vector<std::string> entries{};
    std::function<void(std::string const&, size_t)> action_changed{};
    size_t current_id{};
};

class OverlayColorPicker final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayColorPicker(std::string label, std::function<void(Color const&)> action_changed, Color const& init_color) :
        OverlayElement(std::move(label)), action_changed{std::move(action_changed)},
        color{init_color.r, init_color.g, init_color.b}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::COLOR_PICKER; }

private:
    std::function<void(Color const&)> action_changed{};
    std::array<float, 3> color{};
};

class OverlayTexture final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayTexture(Texture2D const& texture, Vector2ui const& max_size) { set_texture(texture, max_size); }
    explicit OverlayTexture(Texture2D const& texture);

    OverlayElementType get_type() const override { return OverlayElementType::TEXTURE; }

    void set_texture(Texture2D const& texture, Vector2ui const& max_size);

    void set_texture(Texture2D const& texture);

private:
    uint index{};
    Vector2f size;
};

class OverlayProgressBar final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayProgressBar(Vector2i const& range, bool show_values = false) : range{range}, show_values{show_values} {}

    OverlayElementType get_type() const override { return OverlayElementType::PROGRESS_BAR; }

    int get_value() const { return value; }

    bool has_started() const { return value > range.x; }

    bool has_finished() const { return value >= range.y; }

    void set_value(int value) { this->value = value; }

    int operator++() { return ++value; }
    int operator++(int) { return value++; }
    int& operator+=(int val) { return value += val; }
    int operator--() { return --value; }
    int operator--(int) { return value--; }
    int& operator-=(int val) { return value -= val; }

private:
    Vector2i range{};
    int value{};
    bool show_values{};
};

class OverlayPlotEntry {
    friend class OverlayPlot;
    friend OverlayWindow;

public:
    void push(float value)
    {
        values.erase(values.begin());
        values.emplace_back(value);
    }

private:
    std::string name{};
    OverlayPlotType type{};
    std::vector<float> values{};

private:
    OverlayPlotEntry(std::string name, OverlayPlotType type, size_t max_value_count) :
        name{std::move(name)}, type{type}, values(max_value_count)
    {
    }
};

class OverlayPlot final : public OverlayElement {
    friend OverlayWindow;

public:
    OverlayPlot(
        std::string label, size_t max_value_count, std::string x_axis_label = {}, std::string y_axis_label = {},
        Vector2f value_range = {0.f, 100.f}, bool lock_y_axis = false, float max_height = -1.f
    ) :
        OverlayElement(std::move(label)), max_value_count{max_value_count}, x_axis_label{std::move(x_axis_label)},
        y_axis_label{std::move(y_axis_label)}, value_range{value_range}, lock_y{lock_y_axis}, max_height{max_height}
    {
    }

    OverlayElementType get_type() const override { return OverlayElementType::PLOT; }

    OverlayPlotEntry& add_entry(std::string name, OverlayPlotType type = OverlayPlotType::LINE);

private:
    std::vector<std::unique_ptr<OverlayPlotEntry>> entries{};
    size_t max_value_count{};
    std::string x_axis_label{};
    std::string y_axis_label{};
    Vector2f value_range{};
    bool lock_y{};
    float max_height{};
};

class OverlaySeparator final : public OverlayElement {
public:
    OverlayElementType get_type() const override { return OverlayElementType::SEPARATOR; }
};

class OverlayFrameTime final : public OverlayElement {
public:
    explicit OverlayFrameTime(std::string formatted_label) : OverlayElement(std::move(formatted_label)) {}

    OverlayElementType get_type() const override { return OverlayElementType::FRAME_TIME; }
};

class OverlayFpsCounter final : public OverlayElement {
public:
    explicit OverlayFpsCounter(std::string formatted_label) : OverlayElement(std::move(formatted_label)) {}

    OverlayElementType get_type() const override { return OverlayElementType::FPS_COUNTER; }
};

/// OverlayWindow class, representing a specific window in the Overlay.
class OverlayWindow {
public:
    /// Creates an overlay window.
    /// \param title Window title.
    /// \param init_size Initial window size. If both X & Y are strictly lower than 0, automatically resizes the window
    /// from its content.
    /// \param init_pos Initial window position.
    explicit OverlayWindow(
        std::string title, Vector2f const& init_size = Vector2f(0.f), Vector2f const& init_pos = Vector2f(0.f)
    );

    OverlayWindow(OverlayWindow const&) = delete;
    OverlayWindow(OverlayWindow&&) noexcept = default;
    OverlayWindow& operator=(OverlayWindow const&) = delete;
    OverlayWindow& operator=(OverlayWindow&&) noexcept = default;

    bool is_enabled() const { return enabled; }

    /// Changes the window's visibility state.
    /// \param enabled True if the window should be shown, false otherwise.
    void enable(bool enabled = true) { this->enabled = enabled; }

    /// Hides the window.
    void disable() { enable(false); }

    /// Adds a label on the overlay window.
    /// \param label Text to be displayed.
    /// \return Reference to the newly added label.
    OverlayLabel& add_label(std::string label);

    /// Adds a colored label on the overlay window.
    /// \param label Text to be displayed.
    /// \param red Text color's red component.
    /// \param green Text color's green component.
    /// \param blue Text color's blue component.
    /// \param alpha Text color's alpha component.
    /// \return Reference to the newly added colored label.
    OverlayColoredLabel& add_colored_label(std::string label, Color const& color = Color::White);

    /// Adds a button on the overlay window.
    /// \param label Text to be displayed beside the button.
    /// \param action_click Action to be executed when clicked.
    /// \return Reference to the newly added button.
    OverlayButton& add_button(std::string label, std::function<void()> action_click);

    /// Adds a checkbox on the overlay window.
    /// \param label Text to be displayed beside the checkbox.
    /// \param action_on Action to be executed when toggled on.
    /// \param action_off Action to be executed when toggled off.
    /// \param init_value Initial value, checked or not.
    /// \return Reference to the newly added checkbox.
    OverlayCheckbox&
    add_checkbox(std::string label, std::function<void()> action_on, std::function<void()> action_off, bool init_value);

    /// Adds a floating-point slider on the overlay window.
    /// \param label Text to be displayed beside the slider.
    /// \param action_slide Action to be executed on a value change.
    /// \param minValue Lower value bound.
    /// \param maxValue Upper value bound.
    /// \param init_value Initial value.
    /// \return Reference to the newly added slider.
    OverlaySlider&
    add_slider(std::string label, std::function<void(float)> action_slide, Vector2f const& range, float init_value);

    /// Adds a textbox on the overlay window.
    /// \param label Text to be displayed beside the textbox.
    /// \param callback Function to be called every time the content is modified.
    /// \param init_text Initial text to be set.
    /// \return Reference to the newly added textbox.
    OverlayTextbox&
    add_textbox(std::string label, std::function<void(std::string const&)> callback, std::string init_text = {});

    /// Adds a text area on the overlay window.
    /// \param label Text to be displayed beside the text area.
    /// \param callback Function to be called every time the content is modified.
    /// \param init_text Initial text to be set.
    /// \param max_height Maximum height. If strictly lower than 0, automatically resizes the widget to fit the window's
    /// content.
    /// \return Reference to the newly added text area.
    OverlayTextArea& add_text_area(
        std::string label, std::function<void(std::string const&)> callback, std::string init_text = {},
        float max_height = -1.f
    );

    /// Adds a list box on the overlay window.
    /// \param label Text to be displayed beside the list.
    /// \param entries Texts to fill the list with.
    /// \param action_changed Action to be executed when a different element is selected. Receives the currently
    /// selected text & index.
    /// \param init_id Index of the element to select at initialization. Must be less than the entry count.
    /// \return Reference to the newly added listbox.
    OverlayListBox& add_list_box(
        std::string label, std::vector<std::string> entries,
        std::function<void(std::string const&, size_t)> action_changed, size_t init_id = 0
    );

    /// Adds a dropdown list on the overlay window.
    /// \param label Text to be displayed beside the dropdown.
    /// \param entries Texts to fill the dropdown with.
    /// \param action_changed Action to be executed when a different element is selected. Receives the currently
    /// selected text & index.
    /// \param init_id Index of the element to select at initialization. Must be less than the entry count.
    /// \return Reference to the newly added dropdown list.
    OverlayDropdown& add_dropdown(
        std::string label, std::vector<std::string> entries,
        std::function<void(std::string const&, size_t)> action_changed, size_t init_id = 0
    );

    /// Adds a color picker on the overlay window.
    /// \param label Text to be displayed beside the color picker.
    /// \param action_changed Action to be executed when a color is selected.
    /// \param init_color Initial color.
    /// \return Reference to the newly added color picker.
    OverlayColorPicker&
    add_color_picker(std::string label, std::function<void(Color const&)> action_changed, Color const& init_color);

    /// Adds a texture on the overlay window.
    /// \param texture Texture to be displayed.
    /// \param maxWidth Maximum texture's width.
    /// \param max_height Maximum texture's height.
    /// \return Reference to the newly added texture.
    OverlayTexture& add_texture(Texture2D const& texture, Vector2ui const& max_size);

    /// Adds a texture on the overlay window. The maximum width & height will be those of the texture.
    /// \param texture Texture to be displayed.
    /// \return Reference to the newly added texture.
    OverlayTexture& add_texture(Texture2D const& texture);

    /// Adds a progress bar on the overlay window.
    /// \param minVal Minimum value.
    /// \param maxVal Maximum value.
    /// \param show_values Show values ("<current>/<maximum>") instead of percentage.
    /// \return Reference to the newly added progress bar.
    [[nodiscard]] OverlayProgressBar& add_progress_bar(Vector2i const& range, bool show_values = false);

    /// Adds a plot on the overlay window.
    /// \param label Text to be displayed beside the plot.
    /// \param max_value_count Maximum number of values to plot.
    /// \param x_axis_label Label to be displayed on the X (horizontal) axis.
    /// \param y_axis_label Label to be displayed on the Y (vertical) axis.
    /// \param min_yVal Minimum value on the Y (vertical) axis.
    /// \param max_yVal Maximum value on the Y (vertical) axis.
    /// \param lock_y_axis Whether to allow panning & zooming on the Y (vertical) axis.
    /// \param max_height Maximum height. If strictly lower than 0, automatically resizes the widget to fit the window's
    /// content.
    /// \return Reference to the newly added plot.
    [[nodiscard]] OverlayPlot& add_plot(
        std::string label, size_t max_value_count, std::string x_axis_label = {}, std::string y_axis_label = {},
        Vector2f value_range = {0.f, 100.f}, bool lock_y_axis = false, float max_height = -1.f
    );

    /// Adds an horizontal separator on the overlay window.
    /// \return Reference to the newly added separator.
    OverlaySeparator& add_separator();

    /// Adds a frame time display on the overlay window.
    /// \param formatted_label Text with a formatting placeholder to display the frame time (%.Xf, X being the precision
    /// after the comma).
    /// \return Reference to the newly added frame time.
    OverlayFrameTime& add_frame_time(std::string formatted_label);

    /// Adds a FPS (frames per second) counter on the overlay window.
    /// \param formatted_label Text with a formatting placeholder to display the FPS (%.Xf, X being the precision after
    /// the comma).
    /// \return Reference to the newly added FPS counter.
    OverlayFpsCounter& add_fps_counter(std::string formatted_label);

    /// Renders the window's elements.
    virtual void render();

protected:
    std::string title{};
    Vector2f current_size{};
    Vector2f current_pos{};
    bool enabled = true;
    std::vector<std::unique_ptr<OverlayElement>> elements{};
};
}