// texture.cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "texture2d.hpp"
#include <stdexcept>
#include <cstring>

namespace lavander 
{

    static void transition(VkCommandBuffer cmd, VkImage img,VkImageLayout oldL, VkImageLayout newL) 
    {
        VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        b.oldLayout = oldL;
        b.newLayout = newL;
        b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.image = img;
        b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        b.subresourceRange.baseMipLevel = 0;
        b.subresourceRange.levelCount = 1;
        b.subresourceRange.baseArrayLayer = 0;
        b.subresourceRange.layerCount = 1;

        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &b);

        // next transition handled by caller similarly to SHADER_READ_ONLY
    }

    Texture2D::Texture2D(c_device& device, const std::string& path) : device_(device) {
        int w, h, comp;
        stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &comp, STBI_rgb_alpha);
        if (!pixels) throw std::runtime_error("failed to load texture: " + path);

        width_ = (uint32_t)w;
        height_ = (uint32_t)h;
        VkFormat fmt = VK_FORMAT_R8G8B8A8_SRGB;

        createImage(width_, height_, fmt);
        upload(pixels, size_t(width_ * height_ * 4), width_, height_);
        stbi_image_free(pixels);
        createViewAndSampler(fmt);
    }

    Texture2D::Texture2D(c_device& device, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        : device_(device) {
        width_ = height_ = 1;
        VkFormat fmt = VK_FORMAT_R8G8B8A8_SRGB;
        uint32_t px = (uint32_t(r)) |
            (uint32_t(g) << 8) |
            (uint32_t(b) << 16) |
            (uint32_t(a) << 24);
        createImage(1, 1, fmt);
        upload(&px, sizeof(px), 1, 1);
        createViewAndSampler(fmt);
    }

    Texture2D::~Texture2D() {
        auto dev = device_.device();
        if (sampler_)   vkDestroySampler(dev, sampler_, nullptr);
        if (imageView_) vkDestroyImageView(dev, imageView_, nullptr);
        if (image_)     vkDestroyImage(dev, image_, nullptr);
        if (memory_)    vkFreeMemory(dev, memory_, nullptr);
    }

    void Texture2D::createImage(uint32_t w, uint32_t h, VkFormat fmt) {
        VkImageCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        info.imageType = VK_IMAGE_TYPE_2D;
        info.extent = { w,h,1 };
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.format = fmt;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        device_.createImageWithInfo(info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image_, memory_);
    }

    void Texture2D::upload(const void* pixels, size_t size, uint32_t w, uint32_t h) {
        // staging buffer
        VkBuffer staging;
        VkDeviceMemory stagingMem;
        device_.createBuffer(size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging, stagingMem);

        void* data{};
        vkMapMemory(device_.device(), stagingMem, 0, size, 0, &data);
        std::memcpy(data, pixels, size);
        vkUnmapMemory(device_.device(), stagingMem);

        // transitions and copy
        VkCommandBuffer cmd = device_.beginSingleTimeCommands();
        transition(cmd, image_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkBufferImageCopy region{};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = { w,h,1 };
        vkCmdCopyBufferToImage(cmd, staging, image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // to shader read
        VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        b.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        b.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        b.image = image_;
        b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        b.subresourceRange.levelCount = 1;
        b.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &b);

        device_.endSingleTimeCommands(cmd);

        vkDestroyBuffer(device_.device(), staging, nullptr);
        vkFreeMemory(device_.device(), stagingMem, nullptr);
    }

    void Texture2D::createViewAndSampler(VkFormat fmt) {
        // view
        VkImageViewCreateInfo iv{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        iv.image = image_;
        iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv.format = fmt;
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv.subresourceRange.levelCount = 1;
        iv.subresourceRange.layerCount = 1;
        if (vkCreateImageView(device_.device(), &iv, nullptr, &imageView_) != VK_SUCCESS)
            throw std::runtime_error("image view failed");

        // sampler
        VkSamplerCreateInfo s{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        s.magFilter = VK_FILTER_LINEAR;
        s.minFilter = VK_FILTER_LINEAR;
        s.addressModeU = s.addressModeV = s.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        s.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        s.minLod = 0; s.maxLod = 0;
        if (vkCreateSampler(device_.device(), &s, nullptr, &sampler_) != VK_SUCCESS)
            throw std::runtime_error("sampler failed");
    }

    VkDescriptorSet Texture2D::allocateDescriptor(VkDescriptorPool pool, VkDescriptorSetLayout layout) {
        VkDescriptorSetAllocateInfo ai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        ai.descriptorPool = pool;
        ai.descriptorSetCount = 1;
        ai.pSetLayouts = &layout;

        VkDescriptorSet set{};
        if (vkAllocateDescriptorSets(device_.device(), &ai, &set) != VK_SUCCESS)
            throw std::runtime_error("alloc texture descriptor failed");

        VkDescriptorImageInfo ii{};
        ii.sampler = sampler_;
        ii.imageView = imageView_;
        ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet w{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        w.dstSet = set;
        w.dstBinding = 0; // binding 0 in the material set
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        w.pImageInfo = &ii;
        vkUpdateDescriptorSets(device_.device(), 1, &w, 0, nullptr);

        descriptorSet_ = set;
        return set;
    }
}