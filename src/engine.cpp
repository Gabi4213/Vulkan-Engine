#include "engine.hpp"
#include "components.hpp"

#include <stdexcept>
#include <array>

namespace lavander 
{

    Engine::Engine()
    {
        createDescriptorSetLayout();
        createMaterialSetLayout();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets(descriptorSetLayout);
        createMaterialDescriptorPool();
        createMaterialSetLayout();


        createPipelineLayout();
        createPipeline();

        renderer2D = std::make_unique<Renderer2D>(
            device, swapChain.getRenderPass(), swapChain.getSwapChainExtent(),
            pipelineLayout, materialSetLayout, materialPool
        );


        sceneGraph.SetTextureLoader(
            [this](const std::string& path) -> std::shared_ptr<Texture2D> {
                auto tex = std::make_shared<Texture2D>(device, path);
                tex->allocateDescriptor(materialPool, materialSetLayout);
                return tex;
            },
            "../../src/assets"
        );


        lavander::RegisterBuiltInComponents();

        createImGuiDescriptorPool();

        initImGui();

        auto fmt = swapChain.getSwapChainImageFormat();
        sceneRT.create(device.device(), device.getPhysicalDevice(),
            swapChain.getSwapChainExtent(), fmt, nullptr);
        sceneView.SetSceneTexture(sceneRT.imguiTexId());


        initBuffers();
        allocateCommandBuffers();
    }

    Engine::~Engine()
    {
        shutdownImGui();
        if (materialPool) vkDestroyDescriptorPool(device.device(), materialPool, nullptr);
        if (materialSetLayout) vkDestroyDescriptorSetLayout(device.device(), materialSetLayout, nullptr);
        if (descriptorSetLayout) vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
        if (pipelineLayout) vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void Engine::run()
    {
        while (!window.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }
    }

    void Engine::createPipelineLayout()
    {
        VkPushConstantRange push{};
        push.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push.offset = 0;
        push.size = sizeof(PushConst);

        VkDescriptorSetLayout setLayouts[2] = { descriptorSetLayout, materialSetLayout };

        VkPipelineLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        ci.setLayoutCount = 2;
        ci.pSetLayouts = setLayouts;
        ci.pushConstantRangeCount = 1;
        ci.pPushConstantRanges = &push;

        if (vkCreatePipelineLayout(device.device(), &ci, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void Engine::createPipeline()
    {
        auto pipelineConfig = c_pipeline::defaultPipelineConfigInfo(swapChain.width(), swapChain.height());
        pipelineConfig.colorBlendInfo.pAttachments = &pipelineConfig.colorBlendAttachment;
        pipelineConfig.renderPass = swapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<c_pipeline>(
            device, 
            "../../src/shaders/default_shader.vert.spv", 
            "../../src/shaders/default_shader.frag.spv", 
            pipelineConfig);
    }

    void Engine::createCommandBuffers()
    {
        commandBuffers.resize(swapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if(vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data())!= VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (int i = 0; i < commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffers!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = swapChain.getRenderPass();
            renderPassInfo.framebuffer = swapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.2f, 0.5f, 0.9f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            pipeline->bind(commandBuffers[i]);

            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

            renderer2D->draw(commandBuffers[i], registry);

            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer");
            }

        }
    }

    void Engine::initBuffers()
    {
        std::vector<Vertex> vertices = 
        {
            {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
        };

        std::vector<uint32_t> indices = 
        { 
            0, 1, 2 
        };

        buffers = std::make_unique<c_buffers>(device, vertices, indices);
    }

    void Engine::drawFrame()
    {
        uint32_t imageIndex;
        auto result = swapChain.acquireNextImage(&imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("imageIndex failed to acquire swap chain image!");
        }

        //start ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();


        //ui editor
        BeginMainDockspace();
        sceneView.OnImGuiRender();
        sceneGraph.OnImGuiRender();

        ImVec2 vp = sceneGraph.getSceneViewportSize();
        if (vp.x > 0 && vp.y > 0) setSceneViewport(vp.x, vp.y);

        sceneView.SetSceneTexture(sceneRT.imguiTexId());
        ImGui::Render(); // finalize ImGui draw data for this frame

        updateUniformBuffer(imageIndex);
        recordCommandBuffer(imageIndex);

        result = swapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("result failed to acquire swap chain image!");
        }
    }

    void Engine::createDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void Engine::createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        size_t imageCount = swapChain.imageCount();
        uniformBuffers.resize(imageCount);
        uniformBuffersMemory.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++)
        {
            device.createBuffer(
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                uniformBuffers[i],
                uniformBuffersMemory[i]);
        }
    }
    void Engine::createDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(swapChain.imageCount());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(swapChain.imageCount());

        if (vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    void Engine::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout)
    {
        std::vector<VkDescriptorSetLayout> layouts(swapChain.imageCount(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain.imageCount());
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(swapChain.imageCount());
        if (vkAllocateDescriptorSets(device.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < swapChain.imageCount(); i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device.device(), 1, &descriptorWrite, 0, nullptr);
        }

    }

    void Engine::updateUniformBuffer(uint32_t currentImage)
    {
        UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f);
        ubo.view = glm::mat4(1.0f);

        float w = sceneViewW * 2.7;
        float h = sceneViewH;
        float aspect = w / h;

        ubo.proj = glm::ortho(-aspect, aspect, -1.0f, 1.0f);
        ubo.proj[1][1] *= -1.0f;

        void* data = nullptr;
        vkMapMemory(device.device(), uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        std::memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device.device(), uniformBuffersMemory[currentImage]);
    }

    void Engine::allocateCommandBuffers() 
    {
        commandBuffers.resize(swapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Engine::recordCommandBuffer(int idx) {
        auto cmd = commandBuffers[idx];
        vkResetCommandBuffer(cmd, 0);
        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(cmd, &beginInfo);

        //offscreen pass
        VkClearValue sceneClear{};
        sceneClear.color = { 0.1f, 0.1f, 0.12f, 1.0f };
        VkRenderPassBeginInfo rpScene{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        rpScene.renderPass = sceneRT.renderPass();
        rpScene.framebuffer = sceneRT.framebuffer();
        rpScene.renderArea.offset = { 0,0 };
        rpScene.renderArea.extent = sceneRT.extent();
        rpScene.clearValueCount = 1;
        rpScene.pClearValues = &sceneClear;
        vkCmdBeginRenderPass(cmd, &rpScene, VK_SUBPASS_CONTENTS_INLINE);

        pipeline->bind(cmd);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSets.data() + idx, 0, nullptr);

        renderer2D->draw(cmd, registry);

        vkCmdEndRenderPass(cmd);

        //swapchain pass for imgui only
        VkRenderPassBeginInfo rpMain{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        rpMain.renderPass = swapChain.getRenderPass();
        rpMain.framebuffer = swapChain.getFrameBuffer(idx);
        rpMain.renderArea = { {0,0}, swapChain.getSwapChainExtent() };

        std::array<VkClearValue, 2> clears{};
        clears[0].color = { 0.2f,0.5f,0.9f,1.0f };
        clears[1].depthStencil = { 1.0f,0 };
        rpMain.clearValueCount = (uint32_t)clears.size();
        rpMain.pClearValues = clears.data();

        vkCmdBeginRenderPass(cmd, &rpMain, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);
    }

    void Engine::createMaterialSetLayout()
    {
        VkDescriptorSetLayoutBinding sampler{};
        sampler.binding = 0;
        sampler.descriptorCount = 1;
        sampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        info.bindingCount = 1;
        info.pBindings = &sampler;
        if (vkCreateDescriptorSetLayout(device.device(), &info, nullptr, &materialSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create material set layout");
        }
    }

    void Engine::createMaterialDescriptorPool(uint32_t maxSets)
    {
        VkDescriptorPoolSize size{};
        size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        size.descriptorCount = maxSets;

        VkDescriptorPoolCreateInfo ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        ci.poolSizeCount = 1;
        ci.pPoolSizes = &size;
        ci.maxSets = maxSets;

        if (vkCreateDescriptorPool(device.device(), &ci, nullptr, &materialPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create material descriptor pool");
        }
    }

    void Engine::createImGuiDescriptorPool()
    {
        //general purpose pool for imgui
        std::array<VkDescriptorPoolSize, 11> pool_sizes =
        {
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 },
        };

        VkDescriptorPoolCreateInfo pool_info{ };
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * (uint32_t)pool_sizes.size();
        pool_info.poolSizeCount = (uint32_t)pool_sizes.size();
        pool_info.pPoolSizes = pool_sizes.data();

        if (vkCreateDescriptorPool(device.device(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create ImGui descriptor pool");
        }
    }

    static void CheckVk(VkResult err) 
    {
        if (err == VK_SUCCESS) return;
        {
            throw std::runtime_error("ImGui/Vulkan error: " + std::to_string(err));
        }
    }

    void Engine::initImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        //enable docking
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //optional - multi viewports for OS level floating windows
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        //GLFW backend
        ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);

        //vulkan backend
        ImGui_ImplVulkan_InitInfo init_info{};
        init_info.Instance = device.getInstance();              // expose from c_device if needed
        init_info.PhysicalDevice = device.getPhysicalDevice();  // expose from c_device if needed
        init_info.Device = device.device();
        init_info.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily;
        init_info.Queue = device.graphicsQueue();
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = std::max<uint32_t>(2, swapChain.imageCount());
        init_info.ImageCount = swapChain.imageCount();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.CheckVkResultFn = CheckVk;

        init_info.RenderPass = swapChain.getRenderPass();


        //imgui expects to manually load vulkan function pointers before the init, so here we do that if no prototypes is set
#if defined(IMGUI_IMPL_VULKAN_NO_PROTOTYPES) || defined(VK_NO_PROTOTYPES)
        ImGui_ImplVulkan_LoadFunctions(
            VK_API_VERSION_1_0,
            [](const char* name, void* user_data) -> PFN_vkVoidFunction {
                VkInstance inst = (VkInstance)user_data;
                return vkGetInstanceProcAddr(inst, name);
            },
            device.getInstance() 
        );
#endif

        ImGui_ImplVulkan_Init(&init_info);

        uploadImGuiFonts();
    }

    void Engine::BeginMainDockspace()
    {
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_MenuBar |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin("MainDockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        //top bar
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Scene")) { /* TODO */ }
                if (ImGui::MenuItem("Open...", "Ctrl+O")) { /* TODO */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* TODO */ }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window.getGLFWwindow(), 1); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::MenuItem("Undo", "Ctrl+Z", false, false);
                ImGui::MenuItem("Redo", "Ctrl+Y", false, false);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::MenuItem("Reset Layout")) { ImGui::DockBuilderRemoveNode(ImGui::GetID("MyMainDockSpace")); } // optional
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                ImGui::MenuItem("About", nullptr, false, true);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        //dockspace
        ImGuiID dockspace_id = ImGui::GetID("MyMainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);

        //default layout
        static bool first_time = true;
        if (first_time)
        {
            first_time = false;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::DockBuilderSetNodeSize(dockspace_id, vp->WorkSize);

            ImGuiID dock_main_id = dockspace_id;
            ImGuiID dock_left, dock_right;
            //left - 20%
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, &dock_left, &dock_main_id);

            //right - 25%
            ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, &dock_right, &dock_main_id);

            //ensure to make the name the same as the window otherwise things fails
            ImGui::DockBuilderDockWindow("Scene", dock_left);
            ImGui::DockBuilderDockWindow("Properties", dock_right);

            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::End();
    }

    void Engine::shutdownImGui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        if (imguiPool) 
        {
            vkDestroyDescriptorPool(device.device(), imguiPool, nullptr);
            imguiPool = VK_NULL_HANDLE;
        }
    }

    void Engine::uploadImGuiFonts()
    {
        VkCommandBuffer cmd = device.beginSingleTimeCommands();
        device.endSingleTimeCommands(cmd);
    }
}