#include <glfw_initialization.h>
#include <cstdlib>
#include <GLFW/glfw3.h>

namespace veng {
    GlfwInitialization::GlfwInitialization() {
        if (glfwInit() != GLFW_TRUE) {
            std::exit(EXIT_FAILURE);
        }
    }

    GlfwInitialization::~GlfwInitialization() {
        glfwTerminate();
    }
}