#include <glad/glad.h>
#include <GLFW/glfw3.h>

class app {
    GLFWwindow* window = nullptr;

    // render::RenderManager* renderer = nullptr;
    // editor::EditorManager* editor = nullptr;
public:
    app();
    ~app();

    void set_window(GLFWwindow* new_window) {
        window = new_window;
    }

    void process();

    static app& get_app() {
        static app single_app;
        return single_app;
    }
};