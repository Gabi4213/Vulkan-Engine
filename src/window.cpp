#include "window.hpp"

#include <stdexcept>

namespace lavander {

    c_window::c_window(int w, int h, std::string name):
        width{w}, height{h}, windowName{name}
    {
        initWindow();
    }

    c_window::~c_window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void c_window::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    }

    void c_window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface");
        }
    }
}