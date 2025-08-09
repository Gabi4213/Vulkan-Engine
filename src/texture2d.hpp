// texture2d.hpp
#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include "device.hpp"

namespace lavander {

    class Texture2D {
    public:
        // Load from file (RGBA via stb), or make a 1x1 color
        Texture2D(c_device& device, const std::string& path);
        Texture2D(c_device& device, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        ~Texture2D();

        // Create (and cache) a descriptor set for this texture (set = material set)
        VkDescriptorSet allocateDescriptor(VkDescriptorPool pool, VkDescriptorSetLayout layout);

        // Getters
        VkDescriptorSet descriptorSet() const { return descriptorSet_; }
        VkImageView     imageView()     const { return imageView_; }
        VkSampler       sampler()       const { return sampler_; }
        uint32_t        width()         const { return width_; }
        uint32_t        height()        const { return height_; }

    private:
        void createImage(uint32_t w, uint32_t h, VkFormat fmt);
        void upload(const void* pixels, size_t size, uint32_t w, uint32_t h);
        void createViewAndSampler(VkFormat fmt);

        c_device& device_;
        VkImage        image_ = VK_NULL_HANDLE;
        VkDeviceMemory memory_ = VK_NULL_HANDLE;
        VkImageView    imageView_ = VK_NULL_HANDLE;
        VkSampler      sampler_ = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;
        uint32_t       width_ = 0, height_ = 0;
    };

} // namespace lavander
