#include "scene_render_target.hpp"
#include <stdexcept>
#include <array>
#include <imgui_impl_vulkan.h>

namespace lavander {

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

        depthFormat_ = pickDepthFormat();
        createDepthImageAndView();

        createFramebuffer();

        imguiTexId_ = (ImTextureID)ImGui_ImplVulkan_AddTexture(sampler_, imageView_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        if (outRP) *outRP = renderPass_;
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
        if (depthView_)   vkDestroyImageView(device_, depthView_, nullptr);
        if (depthImage_)  vkDestroyImage(device_, depthImage_, nullptr);
        if (depthMem_)    vkFreeMemory(device_, depthMem_, nullptr);

        framebuffer_ = VK_NULL_HANDLE;
        renderPass_ = VK_NULL_HANDLE;
        sampler_ = VK_NULL_HANDLE;
        imageView_ = VK_NULL_HANDLE;
        image_ = VK_NULL_HANDLE;
        imageMem_ = VK_NULL_HANDLE;
        depthView_ = VK_NULL_HANDLE;
        depthImage_ = VK_NULL_HANDLE;
        depthMem_ = VK_NULL_HANDLE;
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
            throw std::runtime_error("SceneRT color image");
        }

        VkMemoryRequirements req{};
        vkGetImageMemoryRequirements(device_, image_, &req);
        VkMemoryAllocateInfo ai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(phys_, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device_, &ai, nullptr, &imageMem_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT color image mem");
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
            throw std::runtime_error("SceneRT color view");
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


    VkFormat SceneRenderTarget::pickDepthFormat() const
    {
        const VkFormat candidates[] = 
        {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM
        };
        for (VkFormat f : candidates)
        {
            VkFormatProperties props{};
            vkGetPhysicalDeviceFormatProperties(phys_, f, &props);

            if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                return f;
            }
        }
        throw std::runtime_error("No supported depth format");
    }

    void SceneRenderTarget::createDepthImageAndView()
    {
        VkImageCreateInfo ci{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        ci.imageType = VK_IMAGE_TYPE_2D;
        ci.extent = { extent_.width, extent_.height, 1 };
        ci.mipLevels = 1;
        ci.arrayLayers = 1;
        ci.format = depthFormat_;
        ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        ci.samples = VK_SAMPLE_COUNT_1_BIT;
        ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device_, &ci, nullptr, &depthImage_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT depth image");
        }

        VkMemoryRequirements req{};
        vkGetImageMemoryRequirements(device_, depthImage_, &req);
        VkMemoryAllocateInfo ai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(phys_, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device_, &ai, nullptr, &depthMem_) != VK_SUCCESS) 
        {
            throw std::runtime_error("SceneRT depth image mem");
        }

        vkBindImageMemory(device_, depthImage_, depthMem_, 0);

        VkImageViewCreateInfo vi{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        vi.image = depthImage_;
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = depthFormat_;
        vi.subresourceRange.levelCount = 1;
        vi.subresourceRange.layerCount = 1;
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (depthFormat_ == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            vi.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        if (vkCreateImageView(device_, &vi, nullptr, &depthView_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT depth view");
        }
    }

    void SceneRenderTarget::createRenderPass(VkFormat colorFormat)
    {
        VkAttachmentDescription color{};
        color.format = colorFormat;
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

        VkAttachmentDescription depth{};
        depth.format = pickDepthFormat();
        depth.samples = VK_SAMPLE_COUNT_1_BIT;
        depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthRef{};
        depthRef.attachment = 1;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &colorRef;
        sub.pDepthStencilAttachment = &depthRef;

        VkSubpassDependency depIn{};
        depIn.srcSubpass = VK_SUBPASS_EXTERNAL;
        depIn.dstSubpass = 0;
        depIn.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        depIn.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        depIn.srcAccessMask = 0;
        depIn.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depIn.dependencyFlags = 0;

        VkSubpassDependency depOut{};
        depOut.srcSubpass = 0;
        depOut.dstSubpass = VK_SUBPASS_EXTERNAL;
        depOut.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        depOut.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        depOut.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        depOut.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        depOut.dependencyFlags = 0;

        std::array<VkAttachmentDescription, 2> atts{ color, depth };
        std::array<VkSubpassDependency, 2> deps{ depIn, depOut };

        VkRenderPassCreateInfo rp{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        rp.attachmentCount = static_cast<uint32_t>(atts.size());
        rp.pAttachments = atts.data();
        rp.subpassCount = 1;
        rp.pSubpasses = &sub;
        rp.dependencyCount = static_cast<uint32_t>(deps.size());
        rp.pDependencies = deps.data();

        if (vkCreateRenderPass(device_, &rp, nullptr, &renderPass_) != VK_SUCCESS)
        {
            throw std::runtime_error("SceneRT render pass");
        }
    }

    void SceneRenderTarget::createFramebuffer()
    {
        VkImageView attachments[] = 
        {
            imageView_,
            depthView_
        };
        VkFramebufferCreateInfo fi{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        fi.renderPass = renderPass_;
        fi.attachmentCount = 2;
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