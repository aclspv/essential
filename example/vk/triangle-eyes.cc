#include <aclspv.hh>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

static const char src[] =
"typedef float f32;\n"
"typedef f32 __attribute__((ext_vector_type(4))) f32x4;\n"
"typedef f32 __attribute__((ext_vector_type(3))) f32x3;\n"
"typedef f32 __attribute__((ext_vector_type(2))) f32x2;\n"
"\n"
"struct PushData {\n"
"    f32 mx;\n"
"    f32 my;\n"
"    f32 aspect;\n"
"    f32 pad;\n"
"};\n"
"\n"
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n"
"void vs_main(\n"
"    __attribute__((annotate(\"aclspv_storage_class(9)\")))\n"
"    struct PushData* pc,\n"
"\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
"    __attribute__((annotate(\"aclspv_bltin(42)\")))\n"
"    int* in_vertidx,\n"
"\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
"    __attribute__((annotate(\"aclspv_bltin(43)\")))\n"
"    int* in_instidx,\n"
"\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
"    __attribute__((annotate(\"aclspv_bltin(0)\")))\n"
"    f32x4* out_pos,\n"
"\n"
"    __attribute__((annotate(\"aclspv_location(0)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
"    f32x3* out_color,\n"
"\n"
"    __attribute__((annotate(\"aclspv_location(1)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
"    f32x2* out_uv,\n"
"\n"
"    __attribute__((annotate(\"aclspv_location(2)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
"    __attribute__((annotate(\"aclspv_flat\")))\n"
"    int* out_inst\n"
") {\n"
"    int vid = *in_vertidx;\n"
"    int iid = *in_instidx;\n"
"    *out_inst = iid;\n"
"\n"
"    if (iid == 0) {\n"
"        f32x3 body_clr = { 0.98f, 0.78f, 0.20f };\n"
"        *out_color = body_clr;\n"
"        f32x2 uv0 = { 0.0f, 0.0f };\n"
"        *out_uv = uv0;\n"
"        if (vid == 0) {\n"
"            f32x4 p = { 0.0f, -0.55f, 0.0f, 1.0f };\n"
"            *out_pos = p;\n"
"        } else if (vid == 1) {\n"
"            f32x4 p = { 0.55f, 0.45f, 0.0f, 1.0f };\n"
"            *out_pos = p;\n"
"        } else {\n"
"            f32x4 p = { -0.55f, 0.45f, 0.0f, 1.0f };\n"
"            *out_pos = p;\n"
"        }\n"
"        return;\n"
"    }\n"
"\n"
"    f32 cx = -0.14f;\n"
"    if (iid == 2 || iid == 4) {\n"
"        cx = 0.14f;\n"
"    }\n"
"    f32 cy = 0.05f;\n"
"    f32 radius = 0.11f;\n"
"\n"
"    if (iid >= 3) {\n"
"        radius = 0.05f;\n"
"        f32 dx = pc->mx - cx;\n"
"        f32 dy = pc->my - cy;\n"
"        f32 ax = dx < 0.0f ? -dx : dx;\n"
"        f32 ay = dy < 0.0f ? -dy : dy;\n"
"        f32 m = ax > ay ? ax : ay;\n"
"        if (m > 0.045f) {\n"
"            f32 scale = 0.045f / m;\n"
"            dx = dx * scale;\n"
"            dy = dy * scale;\n"
"        }\n"
"        cx = cx + dx;\n"
"        cy = cy + dy;\n"
"        f32x3 pupil_clr = { 0.12f, 0.12f, 0.15f };\n"
"        *out_color = pupil_clr;\n"
"    } else {\n"
"        f32x3 sclera_clr = { 1.0f, 1.0f, 1.0f };\n"
"        *out_color = sclera_clr;\n"
"    }\n"
"\n"
"    f32 lx = 0.0f;\n"
"    f32 ly = 0.0f;\n"
"    if (vid == 0) {\n"
"        lx = 0.0f; ly = -2.0f;\n"
"    } else if (vid == 1) {\n"
"        lx = 1.732f; ly = 1.0f;\n"
"    } else {\n"
"        lx = -1.732f; ly = 1.0f;\n"
"    }\n"
"\n"
"    f32x2 uv = { lx, ly };\n"
"    *out_uv = uv;\n"
"\n"
"    f32x4 eye_pos = { cx + (lx * radius) / pc->aspect, cy + (ly * radius), 0.0f, 1.0f };\n"
"    *out_pos = eye_pos;\n"
"}\n"
"\n"
"__attribute__((annotate(\"aclspv_execmode0(7)\")))\n"
"__attribute__((annotate(\"aclspv_execmodel(4)\")))\n"
"void fs_main(\n"
"    __attribute__((annotate(\"aclspv_location(0)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
"    f32x3* in_color,\n"
"\n"
"    __attribute__((annotate(\"aclspv_location(1)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
"    f32x2* in_uv,\n"
"\n"
"    __attribute__((annotate(\"aclspv_location(2)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
"    __attribute__((annotate(\"aclspv_flat\")))\n"
"    int* in_inst,\n"
"\n"
"    __attribute__((annotate(\"aclspv_location(0)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
"    f32x4* out_color\n"
") {\n"
"    f32x3 clr = *in_color;\n"
"    f32 alpha = 1.0f;\n"
"\n"
"    if (*in_inst > 0) {\n"
"        f32x2 uv = *in_uv;\n"
"        f32 d2 = uv[0] * uv[0] + uv[1] * uv[1];\n"
"        if (d2 > 1.0f) {\n"
"            alpha = 0.0f;\n"
"        }\n"
"    }\n"
"\n"
"    f32x4 out = { clr[0], clr[1], clr[2], alpha };\n"
"    *out_color = out;\n"
"}\n";

/* ── Vulkan helpers ───────────────────────────────────────────────────── */
#define VK_CHECK(call)                                                        \
	do {                                                                      \
		VkResult _r = (call);                                                 \
		if (_r != VK_SUCCESS && _r != VK_SUBOPTIMAL_KHR) {                  \
			fprintf(stderr, "%s failed: %d\n", #call, (int)_r);               \
			exit(EXIT_FAILURE);                                               \
		}                                                                     \
	} while (0)

template<size_t len>
ae2f_inline static void *compile_spirv(const char src[len], size_t *out_word_count) {
	aclspv::ctx ctx(aclspv::usrcfg_t()
			.flag0(ACLSPV_CTX_FLAG0_SYMBOL | ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS)
			.mdl_addr(SpvAddressingModelLogical)
			.mdl_mem(SpvMemoryModelGLSL450)
			.spirv_ver(1, 3));

	struct CXUnsavedFile files[1];
	const char *argv[] = {"shader.c"};

	files[0].Contents = src;
	files[0].Filename = "shader.c";
	files[0].Length   = len - 1;

	aclspv_compile_t ret = ctx.build3(files, 1, argv, 1, 0);
	if (ret != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "compile failed: %d\n", (int)ret);
		return NULL;
	}

	aclspv_wrdcount_t word_count;
	aclspv_wrd_t *spirv_words;
	ret = ctx.dump_output(word_count, spirv_words);
	if (ret != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "dump_output failed: %d\n", (int)ret);
		return NULL;
	}

	*out_word_count = word_count;
	return (void *)spirv_words;
}

struct PushData {
	float mx;
	float my;
	float aspect;
	float pad;
};

int main(void) {
	size_t shader_words = 0;
	aclspv_wrd_t *spirv = (aclspv_wrd_t *)compile_spirv<sizeof(src)>(src, &shader_words);
	if (!shader_words) return EXIT_FAILURE;

	puts(src);

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	SDL_Vulkan_LoadLibrary(NULL);

	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,  800);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 800);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "aclspv triangle with eyes");
	SDL_Window *window = SDL_CreateWindowWithProperties(props);
	SDL_DestroyProperties(props);
	if (!window) return EXIT_FAILURE;
	SDL_ShowWindow(window);

	uint32_t ext_count;
	const char * const *exts = SDL_Vulkan_GetInstanceExtensions(&ext_count);

	VkApplicationInfo app = {};
	app.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName   = "aclspv triangle eyes";
	app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app.pEngineName        = "aclspv";
	app.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	app.apiVersion         = VK_API_VERSION_1_1;

	const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
	VkInstanceCreateInfo inst_ci = {};
	inst_ci.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_ci.pApplicationInfo        = &app;
	inst_ci.enabledExtensionCount   = ext_count;
	inst_ci.ppEnabledExtensionNames = exts;
	inst_ci.enabledLayerCount       = 1;
	inst_ci.ppEnabledLayerNames     = layers;

	VkInstance instance;
	VK_CHECK(vkCreateInstance(&inst_ci, NULL, &instance));

	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, instance, NULL, &surface)) return EXIT_FAILURE;

	uint32_t phys_count = 0;
	vkEnumeratePhysicalDevices(instance, &phys_count, NULL);
	if (phys_count == 0) return EXIT_FAILURE;
	std::vector<VkPhysicalDevice> phys_devs(phys_count);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &phys_count, &phys_devs[0]));
	VkPhysicalDevice phys_dev = phys_devs[0];

	uint32_t qf_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &qf_count, NULL);
	std::vector<VkQueueFamilyProperties> qfsv(qf_count);
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &qf_count, &qfsv[0]);

	int qf_idx = -1;
	for (uint32_t i = 0; i < qf_count; i++) {
		VkBool32 can_present = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(phys_dev, i, surface, &can_present);
		if ((qfsv[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && can_present) {
			qf_idx = (int)i;
			break;
		}
	}
	if (qf_idx < 0) return EXIT_FAILURE;

	float qprio = 1.0f;
	VkDeviceQueueCreateInfo q_ci = {};
	q_ci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	q_ci.queueFamilyIndex = (uint32_t)qf_idx;
	q_ci.queueCount       = 1;
	q_ci.pQueuePriorities = &qprio;

	const char *dev_exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkDeviceCreateInfo dev_ci = {};
	dev_ci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_ci.queueCreateInfoCount    = 1;
	dev_ci.pQueueCreateInfos       = &q_ci;
	dev_ci.enabledExtensionCount   = 1;
	dev_ci.ppEnabledExtensionNames = dev_exts;

	VkDevice device;
	VK_CHECK(vkCreateDevice(phys_dev, &dev_ci, NULL, &device));

	VkQueue queue;
	vkGetDeviceQueue(device, (uint32_t)qf_idx, 0, &queue);

	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface, &caps);

	uint32_t fmt_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &fmt_count, NULL);
	std::vector<VkSurfaceFormatKHR> fmtv(fmt_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &fmt_count, &fmtv[0]);
	VkSurfaceFormatKHR surf_fmt = (fmt_count == 1 && fmtv[0].format == VK_FORMAT_UNDEFINED)
		? VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
	: fmtv[0];
	VkExtent2D extent = caps.currentExtent;
	if (extent.width == (uint32_t)-1) {
		int w, h;
		SDL_GetWindowSizeInPixels(window, &w, &h);
		extent.width  = SDL_clamp((uint32_t)w, caps.minImageExtent.width,  caps.maxImageExtent.width);
		extent.height = SDL_clamp((uint32_t)h, caps.minImageExtent.height, caps.maxImageExtent.height);
	}
	uint32_t image_count = caps.minImageCount + 1;
	if (caps.maxImageCount != 0 && image_count > caps.maxImageCount)
		image_count = caps.maxImageCount;
	VkSwapchainCreateInfoKHR sc_ci = {};
	sc_ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_ci.surface          = surface;
	sc_ci.minImageCount    = image_count;
	sc_ci.imageFormat      = surf_fmt.format;
	sc_ci.imageColorSpace  = surf_fmt.colorSpace;
	sc_ci.imageExtent      = extent;
	sc_ci.imageArrayLayers = 1;
	sc_ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	sc_ci.preTransform     = caps.currentTransform;
	sc_ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc_ci.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
	sc_ci.clipped          = VK_TRUE;
	VkSwapchainKHR swapchain;
	VK_CHECK(vkCreateSwapchainKHR(device, &sc_ci, NULL, &swapchain));
	uint32_t sc_img_count;
	vkGetSwapchainImagesKHR(device, swapchain, &sc_img_count, NULL);
	std::vector<VkImage> sc_images(sc_img_count);
	vkGetSwapchainImagesKHR(device, swapchain, &sc_img_count, &sc_images[0]);
	std::vector<VkImageView> sc_views(sc_img_count);
	for (uint32_t i = 0; i < sc_img_count; i++) {
		VkImageViewCreateInfo iv_ci = {};
		iv_ci.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv_ci.image    = sc_images[i];
		iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv_ci.format   = surf_fmt.format;
		iv_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		VK_CHECK(vkCreateImageView(device, &iv_ci, nullptr, &sc_views[i]));
	}
	VkAttachmentDescription att = {};
	att.format         = surf_fmt.format;
	att.samples        = VK_SAMPLE_COUNT_1_BIT;
	att.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	att.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	att.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	att.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	att.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference att_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
	VkSubpassDescription sp = {};
	sp.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sp.colorAttachmentCount = 1;
	sp.pColorAttachments    = &att_ref;
	VkSubpassDependency dep = {};
	dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass    = 0;
	dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	VkRenderPassCreateInfo rp_ci = {};
	rp_ci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_ci.attachmentCount = 1;
	rp_ci.pAttachments    = &att;
	rp_ci.subpassCount    = 1;
	rp_ci.pSubpasses      = &sp;
	rp_ci.dependencyCount = 1;
	rp_ci.pDependencies   = &dep;
	VkRenderPass rp;
	VK_CHECK(vkCreateRenderPass(device, &rp_ci, NULL, &rp));
	VkPushConstantRange pc_range = {};
	pc_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pc_range.offset     = 0;
	pc_range.size       = sizeof(PushData);
	VkPipelineLayoutCreateInfo pl_ci = {};
	pl_ci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl_ci.pushConstantRangeCount = 1;
	pl_ci.pPushConstantRanges    = &pc_range;
	VkPipelineLayout pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(device, &pl_ci, NULL, &pipeline_layout));
	VkShaderModuleCreateInfo sm_ci = {};
	sm_ci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	sm_ci.codeSize = shader_words * sizeof(aclspv_wrd_t);
	sm_ci.pCode    = spirv;
	VkShaderModule module;
	VK_CHECK(vkCreateShaderModule(device, &sm_ci, NULL, &module));
	free(spirv);
	VkPipelineVertexInputStateCreateInfo vi = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	VkPipelineInputAssemblyStateCreateInfo ia = {};
	ia.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	VkPipelineRasterizationStateCreateInfo rast = {};
	rast.sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rast.polygonMode = VK_POLYGON_MODE_FILL;
	rast.cullMode    = VK_CULL_MODE_NONE;
	rast.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rast.lineWidth   = 1.0f;
	VkPipelineMultisampleStateCreateInfo ms = {};
	ms.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPipelineColorBlendAttachmentState cb_att = {};
	cb_att.colorWriteMask      = 0xF;
	cb_att.blendEnable         = VK_TRUE;
	cb_att.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	cb_att.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	cb_att.colorBlendOp        = VK_BLEND_OP_ADD;
	cb_att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	cb_att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	cb_att.alphaBlendOp        = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo cb = {};
	cb.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments    = &cb_att;
	VkPipelineViewportStateCreateInfo vp = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	vp.viewportCount = 1;
	vp.scissorCount  = 1;
	const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates    = dyn_states;
	VkPipelineShaderStageCreateInfo stages[2] = {};
	stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
	stages[0].module = module;
	stages[0].pName  = "vs_main";
	stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[1].module = module;
	stages[1].pName  = "fs_main";
	VkGraphicsPipelineCreateInfo gp_ci = {};
	gp_ci.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp_ci.stageCount          = 2;
	gp_ci.pStages             = stages;
	gp_ci.pVertexInputState   = &vi;
	gp_ci.pInputAssemblyState = &ia;
	gp_ci.pRasterizationState = &rast;
	gp_ci.pMultisampleState   = &ms;
	gp_ci.pColorBlendState    = &cb;
	gp_ci.pViewportState      = &vp;
	gp_ci.pDynamicState       = &dyn;
	gp_ci.layout              = pipeline_layout;
	gp_ci.renderPass          = rp;
	VkPipeline pipeline;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gp_ci, NULL, &pipeline));
	std::vector<VkFramebuffer> framebuffers(sc_img_count);
	for (uint32_t i = 0; i < sc_img_count; i++) {
		VkFramebufferCreateInfo fb_ci = {};
		fb_ci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_ci.renderPass      = rp;
		fb_ci.attachmentCount = 1;
		fb_ci.pAttachments    = &sc_views[i];
		fb_ci.width           = extent.width;
		fb_ci.height          = extent.height;
		fb_ci.layers          = 1;
		VK_CHECK(vkCreateFramebuffer(device, &fb_ci, NULL, &framebuffers[i]));
	}
	VkCommandPoolCreateInfo cp_ci = {};
	cp_ci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cp_ci.queueFamilyIndex = (uint32_t)qf_idx;
	cp_ci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkCommandPool cmd_pool;
	VK_CHECK(vkCreateCommandPool(device, &cp_ci, NULL, &cmd_pool));
	std::vector<VkCommandBuffer> cmd_bufs(sc_img_count);
	VkCommandBufferAllocateInfo cb_ai = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, NULL, cmd_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, sc_img_count};
	VK_CHECK(vkAllocateCommandBuffers(device, &cb_ai, &cmd_bufs[0]));
	std::vector<VkSemaphore> img_avail_sems(sc_img_count), render_done_sems(sc_img_count);
	std::vector<VkFence> render_fences(sc_img_count);
	VkSemaphoreCreateInfo se_ci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkFenceCreateInfo fe_ci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, NULL, VK_FENCE_CREATE_SIGNALED_BIT};
	for (uint32_t i = 0; i < sc_img_count; i++) {
		VK_CHECK(vkCreateSemaphore(device, &se_ci, NULL, &img_avail_sems[i]));
		VK_CHECK(vkCreateSemaphore(device, &se_ci, NULL, &render_done_sems[i]));
		VK_CHECK(vkCreateFence(device, &fe_ci, NULL, &render_fences[i]));
	}
	bool running = true;
	uint32_t frame_idx = 0;
	float mouse_x = 0.0f, mouse_y = 0.0f;
	while (running) {
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_EVENT_QUIT || evt.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
				running = false;
			if (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_Q)
				running = false;
			if (evt.type == SDL_EVENT_MOUSE_MOTION) {
				mouse_x = evt.motion.x;
				mouse_y = evt.motion.y;
			}
		}
		vkWaitForFences(device, 1, &render_fences[frame_idx], VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &render_fences[frame_idx]);
		uint32_t img_idx;
		VkResult acq = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, img_avail_sems[frame_idx], VK_NULL_HANDLE, &img_idx);
		if (acq == VK_ERROR_OUT_OF_DATE_KHR) continue;
		VK_CHECK(acq);
		VkCommandBuffer cmd = cmd_bufs[frame_idx];
		VkCommandBufferBeginInfo cb_bi = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		VK_CHECK(vkBeginCommandBuffer(cmd, &cb_bi));
		VkClearValue clear_val = {{{0.15f, 0.17f, 0.22f, 1.0f}}};
		VkRenderPassBeginInfo rp_bi = {};
		rp_bi.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_bi.renderPass      = rp;
		rp_bi.framebuffer     = framebuffers[img_idx];
		rp_bi.renderArea      = {{0, 0}, extent};
		rp_bi.clearValueCount = 1;
		rp_bi.pClearValues    = &clear_val;
		vkCmdBeginRenderPass(cmd, &rp_bi, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport vp_val = {0.f, 0.f, (float)extent.width, (float)extent.height, 0.f, 1.f};
		VkRect2D sc_val = {{0, 0}, extent};
		vkCmdSetViewport(cmd, 0, 1, &vp_val);
		vkCmdSetScissor(cmd, 0, 1, &sc_val);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		int win_w = 1, win_h = 1;
		SDL_GetWindowSize(window, &win_w, &win_h);
		PushData pc = {};
		pc.mx     = (mouse_x / (float)win_w) * 2.0f - 1.0f;
		pc.my     = (mouse_y / (float)win_h) * 2.0f - 1.0f;
		pc.aspect = (float)extent.width / (float)extent.height;
		vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);
		vkCmdDraw(cmd, 3, 5, 0, 0);
		vkCmdEndRenderPass(cmd);
		VK_CHECK(vkEndCommandBuffer(cmd));
		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo sub = {};
		sub.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sub.waitSemaphoreCount   = 1;
		sub.pWaitSemaphores      = &img_avail_sems[frame_idx];
		sub.pWaitDstStageMask    = &wait_stage;
		sub.commandBufferCount   = 1;
		sub.pCommandBuffers      = &cmd;
		sub.signalSemaphoreCount = 1;
		sub.pSignalSemaphores    = &render_done_sems[frame_idx];
		VK_CHECK(vkQueueSubmit(queue, 1, &sub, render_fences[frame_idx]));
		VkPresentInfoKHR pres = {};
		pres.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		pres.waitSemaphoreCount = 1;
		pres.pWaitSemaphores    = &render_done_sems[frame_idx];
		pres.swapchainCount     = 1;
		pres.pSwapchains        = &swapchain;
		pres.pImageIndices      = &img_idx;
		VK_CHECK(vkQueuePresentKHR(queue, &pres));
		frame_idx = (frame_idx + 1) % sc_img_count;
		SDL_Delay(1);
	}
	vkDeviceWaitIdle(device);
	vkFreeCommandBuffers(device, cmd_pool, sc_img_count, &cmd_bufs[0]);
	vkDestroyCommandPool(device, cmd_pool, NULL);
	for (uint32_t i = 0; i < sc_img_count; i++) vkDestroyFramebuffer(device, framebuffers[i], NULL);
	vkDestroyPipeline(device, pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	vkDestroyRenderPass(device, rp, NULL);
	vkDestroyShaderModule(device, module, NULL);
	vkDestroySwapchainKHR(device, swapchain, NULL);
	for (uint32_t i = 0; i < sc_img_count; i++) {
		vkDestroySemaphore(device, img_avail_sems[i], NULL);
		vkDestroySemaphore(device, render_done_sems[i], NULL);
		vkDestroyFence(device, render_fences[i], NULL);
		vkDestroyImageView(device, sc_views[i], NULL);
	}
	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);
	SDL_DestroyWindow(window);
	SDL_Vulkan_UnloadLibrary();
	SDL_Quit();
	return 0;
}
