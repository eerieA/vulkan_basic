#include <GLFW/glfw3.h>
#include <glfw_initialization.h>
#include <glfw_monitor.h>

std::int32_t main(std::int32_t argc, gsl::zstring* argv) {

    const veng::GlfwInitialization _glfw;

    gsl::not_null<GLFWwindow *> window = glfwCreateWindow(800, 600, "Vulkan Engine", nullptr, nullptr);
    gsl::final_action _cleanup_window([window]() { glfwDestroyWindow(window); });

    glm::ivec2 window_size;
    glfwGetWindowSize(window, &window_size.x, &window_size.y);
    
    gsl::span<GLFWmonitor*> monitors = veng::GetMonitors();
    if (monitors.size() > 0) {
        veng::MoveWindowToMonitor(window, monitors[0]);
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}