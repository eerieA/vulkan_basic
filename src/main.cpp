#include <precomp.h>
#include <GLFW/glfw3.h>
#include <glfw_initialization.h>
#include <glfw_monitor.h>
#include <glfw_window.h>
#include <graphics.h>
#include <glm/gtc/matrix_transform.hpp>

std::int32_t main(std::int32_t argc, gsl::zstring* argv) {

    const veng::GlfwInitialization _glfw;

    veng::Window window("Vulkan Engine", {800, 600});
    window.TryMoveToMonitor(0);     // default to 0, change to other if needed

    veng::Graphics graphics(&window);

    std::array<veng::Vertex, 3> vertices = {
        veng::Vertex{glm::vec3{0.0f, -0.5f, 0.0f}, glm::vec3{1.0f, 0.0f, 0.0f}},
        veng::Vertex{glm::vec3{0.5f, 0.5f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}},
        veng::Vertex{glm::vec3{-0.5f, 0.5f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}},
    };

    veng::BufferHandle buffer = graphics.CreateVertexBuffer(vertices);

    std::array<std::uint32_t, 3> indices = {
        0, 1, 2
    };

    veng::BufferHandle index_buffer = graphics.CreateIndexBuffer(indices);

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    while (!window.ShouldClose()) {
        glfwPollEvents();   // not window specific
        if (graphics.BeginFrame()) {
            graphics.RenderIndexedBuffer(buffer, index_buffer, indices.size());
            graphics.EndFrame();
        }
    }

    graphics.DestroyBuffer(buffer);
    graphics.DestroyBuffer(index_buffer);

    return EXIT_SUCCESS;
}