#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace lavander {
    
    class c_window {
        public:
        c_window(int w, int h, std::string name);
        ~c_window();

        c_window(const c_window &) = delete;
        c_window &operator=(const c_window &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); }
        
        VkExtent2D getExtent() {
            return {
                static_cast<uint32_t>(width), 
                static_cast<uint32_t>(height)
            };
        }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        private:
        void initWindow();

        const int width;
        const int height;

        std::string windowName;
        GLFWwindow *window;
    };
}