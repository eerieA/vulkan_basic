#include <GLFW/glfw3.h>
#include <glfw_initialization.h>
#include <precomp.h>

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