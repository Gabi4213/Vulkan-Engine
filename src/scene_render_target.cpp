#include "scene_render_target.hpp"
#include <stdexcept>
#include <imgui_impl_vulkan.h>

namespace lavander 
{

    static uint32_t findMemoryType(VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags props) 
    {
        VkPhysicalDeviceMemoryProperties m{};
        vkGetPhysicalDeviceMemoryProperties(phys, &m);
        for (uint32_t i = 0; i < m.memoryTypeCount; i++)
        {
            if ((typeBits & (1u << i)) && (m.memoryTypes[i].propertyFlags & props) == props)
            {
                return i;
            }
        }
        throw std::runtime_error("No suitable memory type");
    }

    void SceneRenderTarget::create(VkDevice device, VkPhysicalDevice phys, VkExtent2D ext, VkFormat colorFormat, VkRenderPass* outRP)
    {
        device_ = device; phys_ = phys; extent_ = ext;

        createRenderPass(colorFormat);
        createImage(colorFormat);
        createViewAndSampler(colorFormat);
        createFramebuffer();

        imguiTexId_ = (ImTextureID)ImGui_ImplVulkan_AddTexture( sampler_, imageView_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        if (outRP) 
        { 
            *outRP = renderPass_; 
        }
    }

    void SceneRenderTarget::cleanup()
    {
        if (!device_) return;

        if (framebuffer_) vkDestroyFramebuffer(device_, framebuffer_, nullptr);
        if (renderPass_)  vkDestroyRenderPass(device_, renderPass_, nullptr);
        if (sampler_)     vkDestroySampler(device_, sampler_, nullptr);
        if (imageView_)   vkDestroyImageView(device_, imageView_, nullptr);
        if (image_)       vkDestroyImage(device_, image_, nullptr);
        if (imageMem_)    vkFreeMemory(device_, imageMem_, nullptr);
        framebuffer_ = VK_NULL_HANDLE;
        renderPass_ = VK_NULL_HANDLE;
        sampler_ = VK_NULL_HANDLE;
        imageView_ = VK_NULL_HANDLE;
        image_ = VK_NULL_HANDLE;
        imageMem_ = VK_NULL_HANDLE;
        device_ = VK_NULL_HANDLE;
    }

    void SceneRenderTarget::resize(VkDevice device, VkPhysicalDevice phys, VkExtent2D newExt, VkFormat fmt)
    {
        cleanup();
        create(device, phys, newExt, fmt, nullptr);
    }

    void SceneRenderTarget::createImage(VkFormat format)
    {
        VkImageCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        ci.imageType = VK_IMAGE_TYPE_2D;
        ci.extent = { extent_.width, extent_.height, 1 };
        ci.mipLevels = 1;
        ci.arrayLayers = 1;
        ci.format = format;
        ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device_, &ci, nullptr, &image_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT image");
        }

        VkMemoryRequirements req{};
        vkGetImageMemoryRequirements(device_, image_, &req);
        VkMemoryAllocateInfo ai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(phys_, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (vkAllocateMemory(device_, &ai, nullptr, &imageMem_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT image mem");
        }

        vkBindImageMemory(device_, image_, imageMem_, 0);
    }

    void SceneRenderTarget::createViewAndSampler(VkFormat format)
    {
        VkImageViewCreateInfo vi{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        vi.image = image_;
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = format;
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vi.subresourceRange.levelCount = 1;
        vi.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device_, &vi, nullptr, &imageView_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT view");
        }

        VkSamplerCreateInfo si{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        si.magFilter = VK_FILTER_LINEAR;
        si.minFilter = VK_FILTER_LINEAR;
        si.addressModeU = si.addressModeV = si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        si.minLod = 0; si.maxLod = 0;

        if (vkCreateSampler(device_, &si, nullptr, &sampler_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT sampler");
        }
    }

    void SceneRenderTarget::createRenderPass(VkFormat format)
    {
        VkAttachmentDescription color{};
        color.format = format;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &colorRef;

        VkSubpassDependency dep{};
        dep.srcSubpass = VK_SUBPASS_EXTERNAL;
        dep.dstSubpass = 0;
        dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo rp{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        rp.attachmentCount = 1;
        rp.pAttachments = &color;
        rp.subpassCount = 1;
        rp.pSubpasses = &sub;
        rp.dependencyCount = 1;
        rp.pDependencies = &dep;

        if (vkCreateRenderPass(device_, &rp, nullptr, &renderPass_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT render pass");
        }
    }

    void SceneRenderTarget::createFramebuffer() 
    {
        VkImageView attachments[] = { imageView_ };
        VkFramebufferCreateInfo fi{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fi.renderPass = renderPass_;
        fi.attachmentCount = 1;
        fi.pAttachments = attachments;
        fi.width = extent_.width;
        fi.height = extent_.height;
        fi.layers = 1;

        if (vkCreateFramebuffer(device_, &fi, nullptr, &framebuffer_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT framebuffer");
        }
    }
}