#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"

#include <GLFW/glfw3.h>

#include <cstdio>
#include <memory>

namespace {

class TriangleRenderer {
public:
    TriangleRenderer() = default;
    TriangleRenderer(const TriangleRenderer&) = delete;
    TriangleRenderer& operator=(const TriangleRenderer&) = delete;

    ~TriangleRenderer() {
        glDeleteBuffers(1, &index_buffer_);
        glDeleteBuffers(1, &vertex_buffer_);
        glDeleteVertexArrays(1, &vertex_array_);
        glDeleteProgram(program_);
    }

    bool initialize() {
        constexpr char vertex_shader_source[] = R"(
            #version 330 core
            layout (location = 0) in vec2 position;
            layout (location = 1) in vec3 color;
            out vec3 vertex_color;

            void main() {
                gl_Position = vec4(position, 0.0, 1.0);
                vertex_color = color;
            }
        )";
        constexpr char fragment_shader_source[] = R"(
            #version 330 core
            in vec3 vertex_color;
            out vec4 fragment_color;

            void main() {
                fragment_color = vec4(vertex_color, 1.0);
            }
        )";
        constexpr float vertices[] = {
            0.15f, -0.55f, 1.0f, 0.2f, 0.2f,
            0.85f, -0.55f, 0.2f, 1.0f, 0.2f,
            0.50f,  0.65f, 0.2f, 0.4f, 1.0f,
        };
        constexpr unsigned int indices[] = {0, 1, 2};
        constexpr GLenum static_draw = 0x88E4;

        const GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
        if (vertex_shader == 0) {
            return false;
        }

        const GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
        if (fragment_shader == 0) {
            glDeleteShader(vertex_shader);
            return false;
        }

        program_ = glCreateProgram();
        glAttachShader(program_, vertex_shader);
        glAttachShader(program_, fragment_shader);
        glLinkProgram(program_);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        GLint link_status = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &link_status);
        if (link_status != GL_TRUE) {
            print_program_error(program_);
            glDeleteProgram(program_);
            program_ = 0;
            return false;
        }

        glGenVertexArrays(1, &vertex_array_);
        glGenBuffers(1, &vertex_buffer_);
        glGenBuffers(1, &index_buffer_);
        glBindVertexArray(vertex_array_);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, static_draw);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, static_draw);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
            reinterpret_cast<void*>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
        return true;
    }

    void draw() const {
        glUseProgram(program_);
        glBindVertexArray(vertex_array_);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);
    }

private:
    static GLuint compile_shader(GLenum type, const char* source) {
        const GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint compile_status = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status != GL_TRUE) {
            GLint log_length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
            auto log = std::make_unique<GLchar[]>(static_cast<std::size_t>(log_length));
            glGetShaderInfoLog(shader, log_length, nullptr, log.get());
            std::fprintf(stderr, "OpenGL shader compilation failed: %s\n", log.get());
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    static void print_program_error(GLuint program) {
        GLint log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        auto log = std::make_unique<GLchar[]>(static_cast<std::size_t>(log_length));
        glGetProgramInfoLog(program, log_length, nullptr, log.get());
        std::fprintf(stderr, "OpenGL shader program linking failed: %s\n", log.get());
    }

    GLuint program_ = 0;
    GLuint vertex_array_ = 0;
    GLuint vertex_buffer_ = 0;
    GLuint index_buffer_ = 0;
};

class GlfwLifetime {
public:
    GlfwLifetime() = default;
    GlfwLifetime(const GlfwLifetime&) = delete;
    GlfwLifetime& operator=(const GlfwLifetime&) = delete;

    ~GlfwLifetime() {
        glfwTerminate();
    }
};

void glfw_error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW.\n");
        return 1;
    }
    const GlfwLifetime glfw_lifetime;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window{
        glfwCreateWindow(1280, 800, "Dear ImGui - C++23 demo", nullptr, nullptr),
        glfwDestroyWindow,
    };
    if (!window) {
        std::fprintf(stderr, "Failed to create a GLFW window.\n");
        return 1;
    }

    glfwMakeContextCurrent(window.get());
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window.get(), true)) {
        std::fprintf(stderr, "Failed to initialize the ImGui GLFW backend.\n");
        ImGui::DestroyContext();
        return 1;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::fprintf(stderr, "Failed to initialize the ImGui OpenGL backend.\n");
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return 1;
    }

    TriangleRenderer triangle_renderer;
    if (!triangle_renderer.initialize()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return 1;
    }

    bool show_demo_window = true;
    float slider_value = 0.5f;
    float clear_color[3] = {0.10f, 0.18f, 0.24f};

    while (!glfwWindowShouldClose(window.get())) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello, ImGui!");
        ImGui::Text("Dear ImGui is running with GLFW and OpenGL 3.");
        ImGui::Text("This project uses C++23 and CMake FetchContent.");
        ImGui::Checkbox("Show ImGui demo window", &show_demo_window);
        ImGui::SliderFloat("Example slider", &slider_value, 0.0f, 1.0f);
        ImGui::ColorEdit3("Clear color", clear_color);
        ImGui::ProgressBar(slider_value, ImVec2(-1.0f, 0.0f));
        ImGui::Text("The OpenGL triangle is to the right of this window.");
        ImGui::End();

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::Render();
        int display_width = 0;
        int display_height = 0;
        glfwGetFramebufferSize(window.get(), &display_width, &display_height);
        glViewport(0, 0, display_width, display_height);
        glClearColor(clear_color[0], clear_color[1], clear_color[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        triangle_renderer.draw();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window.get());
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
