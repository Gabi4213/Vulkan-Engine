#include "pipeline.hpp"
#include "buffers.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace lavander
{

    c_pipeline::c_pipeline(
        c_device& device,
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& configInfo) :
        device{ device }
    {
        createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
    }

    c_pipeline::~c_pipeline()
    {
        vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(device.device(), graphicsPipeline, nullptr);
    }

    std::vector<char> c_pipeline::readFile(const std::string& filepath)
    {
        std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file: " + filepath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void c_pipeline::createGraphicsPipeline(
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const PipelineConfigInfo& cfg)
    {
        assert(cfg.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout");
        assert(cfg.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass");

        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2]{};
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";

        //vertex input
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = cfg.vertexInputInfo;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(cfg.bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = cfg.bindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(cfg.attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = cfg.attributeDescriptions.data();

        //viewport and scissor
        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = &cfg.viewport;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = &cfg.scissor;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &cfg.inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &cfg.rasterizationInfo;
        pipelineInfo.pMultisampleState = &cfg.multisampleInfo;
        pipelineInfo.pColorBlendState = &cfg.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &cfg.depthStencilInfo;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = cfg.pipelineLayout;
        pipelineInfo.renderPass = cfg.renderPass;
        pipelineInfo.subpass = cfg.subpass;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

    void c_pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void c_pipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    PipelineConfigInfo c_pipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
    {
        PipelineConfigInfo cfg{};

        //input assembly
        cfg.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        cfg.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        cfg.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        //viewport and scissor
        cfg.viewport.x = 0.0f;
        cfg.viewport.y = 0.0f;
        cfg.viewport.width = static_cast<float>(width);
        cfg.viewport.height = static_cast<float>(height);
        cfg.viewport.minDepth = 0.0f;
        cfg.viewport.maxDepth = 1.0f;

        cfg.scissor.offset = { 0, 0 };
        cfg.scissor.extent = { width, height };

        //rasterizer
        cfg.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        cfg.rasterizationInfo.depthClampEnable = VK_FALSE;
        cfg.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        cfg.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        cfg.rasterizationInfo.lineWidth = 1.0f;
        cfg.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        cfg.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;

        //multisampling
        cfg.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        cfg.multisampleInfo.sampleShadingEnable = VK_FALSE;
        cfg.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        //color blend
        cfg.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        cfg.colorBlendAttachment.blendEnable = VK_FALSE;

        cfg.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        cfg.colorBlendInfo.logicOpEnable = VK_FALSE;
        cfg.colorBlendInfo.attachmentCount = 1;
        cfg.colorBlendInfo.pAttachments = &cfg.colorBlendAttachment; // set it here for convenience

        //depth stencil
        cfg.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        cfg.depthStencilInfo.depthTestEnable = VK_TRUE;
        cfg.depthStencilInfo.depthWriteEnable = VK_TRUE;
        cfg.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        cfg.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        cfg.depthStencilInfo.stencilTestEnable = VK_FALSE;

        //cull and new stufdf for 3d renderinfg
        cfg.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        cfg.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        return cfg;
    }
}