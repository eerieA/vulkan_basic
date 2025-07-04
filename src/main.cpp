#include <precomp.h>
#include <GLFW/glfw3.h>
#include <glfw_initialization.h>
#include <glfw_monitor.h>
#include <glfw_window.h>
#include <graphics.h>

std::int32_t main(std::int32_t argc, gsl::zstring* argv) {

    const veng::GlfwInitialization _glfw;

    veng::Window window("Vulkan Engine", {800, 600});
    window.TryMoveToMonitor(0);     // default to 0, change to other if needed

    veng::Graphics graphics(&window);
    
    while (!window.ShouldClose()) {
        glfwPollEvents();   // not window specific
        graphics.BeginFrame();
        graphics.RenderTriangle();
        graphics.EndFrame();
    }

    return EXIT_SUCCESS;
}