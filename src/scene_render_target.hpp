#pragma once
#include <vulkan/vulkan.h>
#include <imgui.h>

namespace lavander
{

    class SceneRenderTarget
    {
    public:
        SceneRenderTarget() = default;
        ~SceneRenderTarget() { cleanup(); }

        void create(VkDevice device, VkPhysicalDevice phys, VkExtent2D extent, VkFormat colorFormat, VkRenderPass* outRenderPass);
        void cleanup();

        VkRenderPass   renderPass()  const { return renderPass_; }
        VkFramebuffer  framebuffer() const { return framebuffer_; }
        VkExtent2D     extent()      const { return extent_; }
        ImTextureID    imguiTexId()  const { return imguiTexId_; }

        void resize(VkDevice device, VkPhysicalDevice phys, VkExtent2D newExtent, VkFormat colorFormat);

        VkImageView    imageView_ = VK_NULL_HANDLE;
        VkSampler      sampler_ = VK_NULL_HANDLE;

    private:
        VkDevice       device_ = VK_NULL_HANDLE;
        VkPhysicalDevice phys_ = VK_NULL_HANDLE;
        VkExtent2D     extent_{};
        VkImage        image_ = VK_NULL_HANDLE;
        VkDeviceMemory imageMem_ = VK_NULL_HANDLE;
        VkRenderPass   renderPass_ = VK_NULL_HANDLE;
        VkFramebuffer  framebuffer_ = VK_NULL_HANDLE;
        ImTextureID    imguiTexId_ = 0;

        void createImage(VkFormat format);
        void createViewAndSampler(VkFormat format);
        void createRenderPass(VkFormat format);
        void createFramebuffer();
    };
} 