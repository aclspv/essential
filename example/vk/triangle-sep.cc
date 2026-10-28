#include <aclspv.hh>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#define vert_idx_if(c, p0, p1, c0, c1, c2) "if( " c " )"	\
	"{"	\
	"f32x4 pos4 = { " #p0 ", " #p1 ", 0.f, 1.f }; "	\
	"f32x3 clr = { " #c0 ", " #c1 ", " #c2 " };"	\
	"*out_pos = pos4; *out_color = clr;"	\
	"}"

/* ── vertex shader source ─────────────────────────────────────────────── */
static const char vertex_src[] =
"typedef float f32;\n"
"typedef f32 __attribute__((ext_vector_type(4))) f32x4;\n"
"typedef f32 __attribute__((ext_vector_type(3))) f32x3;\n"
"typedef f32 __attribute__((ext_vector_type(2))) f32x2;\n"
"\n"
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n"
"void vs_main(\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
"    __attribute__((annotate(\"aclspv_bltin(42)\")))\n"
"    int* in_vertidx,\n"
"\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
"    __attribute__((annotate(\"aclspv_bltin(0)\")))\n"
"    f32x4* out_pos,\n"
"\n"
"    __attribute__((annotate(\"aclspv_location(0)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
"    f32x3* out_color\n"
") {\n"
vert_idx_if("*in_vertidx == 0", 0.f, -0.5f, 1.f, 0.f, 0.f)
"else " vert_idx_if("*in_vertidx == 1", 0.5f, 0.5f, 0.f, 1.f, 0.f)
"else " vert_idx_if("*in_vertidx == 2", 0.f - 0.5, 0.5f, 0.f, 0.f, 1.f)
"}\n";

/* ── fragment shader source ───────────────────────────────────────────── */
static const char fragment_src[] =
"typedef float f32;\n"
"typedef f32 __attribute__((ext_vector_type(3))) f32x3;\n"
"typedef f32 __attribute__((ext_vector_type(4))) f32x4;\n"
"\n"
"__attribute__((annotate(\"aclspv_execmode0(7)\")))\n"
"__attribute__((annotate(\"aclspv_execmodel(4)\")))\n"
"void fs_main(\n"
"    __attribute__((annotate(\"aclspv_location(0)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
"    f32x3* in_color,\n"
"\n"
"    __attribute__((annotate(\"aclspv_location(0)\")))\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
"    f32x4* out_color\n"
") {\n"
""  "f32x3 clr = *in_color;"
"    f32x4 out = { clr[0], clr[1], clr[2], 1.0f };\n"
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

/* ── compile shader source → SPIR-V blob ───────────────────────────────── */
template<size_t len>
ae2f_inline static void *compile_spirv(const char src[len], size_t *out_word_count, const char* envname_to_dump) {
	aclspv::ctx ctx(aclspv::usrcfg_t()
			.flag0(ACLSPV_CTX_FLAG0_SYMBOL | ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS)
			.mdl_addr(SpvAddressingModelLogical)
			.mdl_mem(SpvMemoryModelGLSL450)
			.spirv_ver(1, 3));

	struct CXUnsavedFile files[1];
	const char *argv[] = {"shader.c"};

	files[0].Contents  = src;
	files[0].Filename  = "shader.c";
	files[0].Length    = len - 1;

	aclspv_compile_t ret = ctx.compile3(files, 1, argv, 1, 0);
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

	/* dump SPIR-V for debugging if OUTPUT_SPIRV_PATH is set */
	const char *dump_path = getenv(envname_to_dump);
	if (dump_path) {
		FILE *f = fopen(dump_path, "wb");
		if (f) {
			fwrite(spirv_words, sizeof(aclspv_wrd_t), word_count, f);
			fclose(f);
		}
	}

	*out_word_count = word_count;
	return (void *)spirv_words;
}

/* ── main ──────────────────────────────────────────────────────────────── */
int main(void) {
	/* ── compile shaders ──────────────────────────────────────────────── */
	size_t vs_words = 0, fs_words = 0;
	aclspv_wrd_t *vs_spirv = (aclspv_wrd_t *)compile_spirv<sizeof(vertex_src)>(
			vertex_src
			, &vs_words
			, "PATH_VS"
			);
	aclspv_wrd_t *fs_spirv = (aclspv_wrd_t *)compile_spirv<sizeof(fragment_src)>(
			fragment_src
			, &fs_words
			, "PATH_FS"
			);
	if (!vs_spirv || !fs_spirv) return EXIT_FAILURE;

	fputs("compile is done\n", stderr);

	/* ── SDL init ─────────────────────────────────────────────────────── */
	fputs("sdl init\n", stderr);
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
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "aclspv triangle");
	SDL_Window *window = SDL_CreateWindowWithProperties(props);
	SDL_DestroyProperties(props);
	if (!window) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return EXIT_FAILURE; }
	SDL_ShowWindow(window);

	/* ── Vulkan instance ──────────────────────────────────────────────── */
	fputs("vulkan instance\n", stderr);
	uint32_t ext_count;
	const char * const *exts = SDL_Vulkan_GetInstanceExtensions(&ext_count);

	VkApplicationInfo app = {};
	app.sType           = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName= "aclspv triangle";
	app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app.pEngineName     = "aclspv";
	app.engineVersion   = VK_MAKE_VERSION(1, 0, 0);
	app.apiVersion      = VK_API_VERSION_1_1;

	const char *layers[] = {"VK_LAYER_KHRONOS_validation"};

	VkInstanceCreateInfo inst_ci = {};
	inst_ci.sType             = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_ci.pApplicationInfo  = &app;
	inst_ci.enabledExtensionCount = ext_count;
	inst_ci.ppEnabledExtensionNames   = exts;
	inst_ci.enabledLayerCount     = 1;
	inst_ci.ppEnabledLayerNames       = layers;

	VkInstance instance;
	VK_CHECK(vkCreateInstance(&inst_ci, NULL, &instance));

	if(!instance) {
		fputs("\t[X] instance is zero\n", stderr);
		return EXIT_FAILURE;
	}

	/* ── surface ──────────────────────────────────────────────────────── */
	fputs("surface\n", stderr);
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, instance, NULL, &surface)) {
		fprintf(stderr, "SDL_Vulkan_CreateSurface: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	/* ── physical device ──────────────────────────────────────────────── */
	fputs("physical device\n", stderr);
	uint32_t phys_count = 0;
	vkEnumeratePhysicalDevices(instance, &phys_count, NULL);
	if (phys_count == 0) { fprintf(stderr, "no physical device\n"); return EXIT_FAILURE; }

	fprintf(stderr, "physical device count: %lu\n", (unsigned long)phys_count);

	std::vector<VkPhysicalDevice> phys_devs(phys_count);
	VK_CHECK(vkEnumeratePhysicalDevices(instance, &phys_count, &phys_devs[0]));

	VkPhysicalDevice& phys_dev = phys_devs[0];
	{
		typedef uint32_t u32;
		const char* name = getenv("IDX_PHYDEV");
		u32 c = name ? atoi(name) : 0;
		c %= phys_count;
		phys_dev = phys_devs[c];
	}

	/* ── queue family ─────────────────────────────────────────────────── */
	fputs("queue family\n", stderr);
	uint32_t qf_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &qf_count, NULL);
	std::vector<VkQueueFamilyProperties> qfsv(qf_count);
	VkQueueFamilyProperties *qfs = &qfsv[0];
	vkGetPhysicalDeviceQueueFamilyProperties(phys_dev, &qf_count, qfs);

	int qf_idx = -1;
	for (uint32_t i = 0; i < qf_count; i++) {
		VkBool32 can_present = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(phys_dev, i, surface, &can_present);
		if (qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && can_present) {
			qf_idx = (int)i;
			break;
		}
	}
	if (qf_idx < 0) { fprintf(stderr, "no suitable queue family\n"); return EXIT_FAILURE; }

	float qprio = 1.0f;
	VkDeviceQueueCreateInfo q_ci = {};
	q_ci.sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	q_ci.queueFamilyIndex   = (uint32_t)qf_idx;
	q_ci.queueCount         = 1;
	q_ci.pQueuePriorities   = &qprio;

	const char *dev_exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkDeviceCreateInfo dev_ci = {};
	dev_ci.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_ci.queueCreateInfoCount     = 1;
	dev_ci.pQueueCreateInfos        = &q_ci;
	dev_ci.enabledExtensionCount    = 1;
	dev_ci.ppEnabledExtensionNames  = dev_exts;

	VkDevice device;
	VK_CHECK(vkCreateDevice(phys_dev, &dev_ci, NULL, &device));

	VkQueue queue;
	vkGetDeviceQueue(device, (uint32_t)qf_idx, 0, &queue);

	/* ── swapchain ────────────────────────────────────────────────────── */
	fputs("swapchain\n", stderr);
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface, &caps);

	uint32_t fmt_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &fmt_count, NULL);
	std::vector<VkSurfaceFormatKHR> fmtv(fmt_count);
	VkSurfaceFormatKHR *fmts = &fmtv[0];
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &fmt_count, fmts);

	VkSurfaceFormatKHR surf_fmt;
	if (fmt_count == 1 && fmts[0].format == VK_FORMAT_UNDEFINED) {
		surf_fmt.format      = VK_FORMAT_B8G8R8A8_UNORM;
		surf_fmt.colorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	} else {
		surf_fmt = fmts[0];
	}

	uint32_t mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &mode_count, NULL);
	std::vector<VkPresentModeKHR> modev(mode_count);
	VkPresentModeKHR *modes = &modev[0];
	vkGetPhysicalDeviceSurfacePresentModesKHR(phys_dev, surface, &mode_count, modes);

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (uint32_t i = 0; i < mode_count; i++) {
		if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) { present_mode = modes[i]; break; }
	}

	VkExtent2D extent;
	if (caps.currentExtent.width != (uint32_t)-1) {
		extent = caps.currentExtent;
	} else {
		int width, height;
		SDL_GetWindowSize(window, &width, &height);

		extent.width = width;
		extent.height = height;

		extent.width  = SDL_clamp(extent.width,  caps.minImageExtent.width,  caps.maxImageExtent.width);
		extent.height = SDL_clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
	}

	VkSwapchainKHR swapchain;
	VkSwapchainCreateInfoKHR sc_ci = {};
	sc_ci.sType               = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sc_ci.surface             = surface;
	sc_ci.minImageCount       = caps.minImageCount + 1;
	sc_ci.imageFormat         = surf_fmt.format;
	sc_ci.imageColorSpace     = surf_fmt.colorSpace;
	sc_ci.imageExtent         = extent;
	sc_ci.imageArrayLayers    = 1;
	sc_ci.imageUsage          = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sc_ci.imageSharingMode    = VK_SHARING_MODE_EXCLUSIVE;
	sc_ci.preTransform        = caps.currentTransform;
	sc_ci.compositeAlpha      = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc_ci.presentMode         = present_mode;
	sc_ci.oldSwapchain        = VK_NULL_HANDLE;
	sc_ci.clipped             = VK_TRUE;

	VK_CHECK(vkCreateSwapchainKHR(device, &sc_ci, NULL, &swapchain));

	uint32_t sc_img_count;
	vkGetSwapchainImagesKHR(device, swapchain, &sc_img_count, NULL);
	std::vector<VkImage> sc_imgv(sc_img_count);
	VkImage *sc_images = &sc_imgv[0];
	vkGetSwapchainImagesKHR(device, swapchain, &sc_img_count, sc_images);

	std::vector<VkImageView> sc_viewv(sc_img_count);
	VkImageView *sc_views = &sc_viewv[0];
	for (uint32_t i = 0; i < sc_img_count; i++) {
		VkImageViewCreateInfo iv_ci = {};
		iv_ci.sType           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv_ci.image           = sc_images[i];
		iv_ci.viewType        = VK_IMAGE_VIEW_TYPE_2D;
		iv_ci.format          = surf_fmt.format;
		iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		iv_ci.subresourceRange.levelCount = 1;
		iv_ci.subresourceRange.layerCount = 1;
		VK_CHECK(vkCreateImageView(device, &iv_ci, nullptr, &sc_views[i]));
	}

	/* ── render pass ──────────────────────────────────────────────────── */
	fputs("render pass\n", stderr);
	VkAttachmentDescription att = {};
	att.format        = surf_fmt.format;
	att.samples       = VK_SAMPLE_COUNT_1_BIT;
	att.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
	att.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
	att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	att.stencilStoreOp= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	att.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference att_ref = {};
	att_ref.attachment = 0;
	att_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription sp = {};
	sp.pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sp.colorAttachmentCount   = 1;
	sp.pColorAttachments      = &att_ref;

	VkSubpassDependency dep = {};
	dep.srcSubpass      = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass      = 0;
	dep.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.srcAccessMask   = 0;
	dep.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPass rp;
	VkRenderPassCreateInfo rp_ci = {};
	rp_ci.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_ci.attachmentCount  = 1;
	rp_ci.pAttachments     = &att;
	rp_ci.subpassCount     = 1;
	rp_ci.pSubpasses       = &sp;
	rp_ci.dependencyCount  = 1;
	rp_ci.pDependencies    = &dep;
	VK_CHECK(vkCreateRenderPass(device, &rp_ci, NULL, &rp));

	/* ── pipeline layout ──────────────────────────────────────────────── */
	fputs("pipeline layout\n", stderr);
	VkPipelineLayoutCreateInfo pl_ci = {};
	pl_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	VkPipelineLayout pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(device, &pl_ci, NULL, &pipeline_layout));

	/* ── shader modules ───────────────────────────────────────────────── */
	fputs("shader modules\n", stderr);
	VkShaderModule vs_module, fs_module;

	{
		VkShaderModuleCreateInfo sm_ci = {};
		sm_ci.sType            = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		sm_ci.codeSize         = vs_words * sizeof(aclspv_wrd_t);
		sm_ci.pCode            = vs_spirv;
		VK_CHECK(vkCreateShaderModule(device, &sm_ci, NULL, &vs_module));
	}
	{
		VkShaderModuleCreateInfo sm_ci = {};
		sm_ci.sType            = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		sm_ci.codeSize         = fs_words * sizeof(aclspv_wrd_t);
		sm_ci.pCode            = fs_spirv;
		VK_CHECK(vkCreateShaderModule(device, &sm_ci, NULL, &fs_module));
	}

	/* ── graphics pipeline ────────────────────────────────────────────── */
	/* vertex input — no vertex buffers, hardcoded triangle */
	fputs("graphics pipeline\n", stderr);
	VkPipelineVertexInputStateCreateInfo vi = {};
	vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo ia = {};
	ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rast = {};
	rast.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rast.depthClampEnable = VK_FALSE;
	rast.rasterizerDiscardEnable = VK_FALSE;
	rast.polygonMode = VK_POLYGON_MODE_FILL;
	rast.cullMode = VK_CULL_MODE_NONE;
	rast.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rast.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo ms = {};
	ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState cb_att = {};
	cb_att.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	cb_att.blendEnable = VK_TRUE;
	cb_att.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	cb_att.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	cb_att.colorBlendOp = VK_BLEND_OP_ADD;
	cb_att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	cb_att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	cb_att.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo cb = {};
	cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cb.attachmentCount = 1;
	cb.pAttachments    = &cb_att;

	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount  = 1;

	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	const VkDynamicState dyn_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	dyn.pDynamicStates = dyn_states;

	VkPipelineShaderStageCreateInfo stages[2] = {};

	{
		stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[0].module = vs_module;
		stages[0].pName = "vs_main";
	}
	{
		stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[1].module = fs_module;
		stages[1].pName = "fs_main";
	}

	VkGraphicsPipelineCreateInfo gp_ci = {};
	gp_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gp_ci.stageCount = 2;
	gp_ci.pStages = stages;
	gp_ci.pVertexInputState      = &vi;
	gp_ci.pInputAssemblyState    = &ia;
	gp_ci.pRasterizationState    = &rast;
	gp_ci.pMultisampleState      = &ms;
	gp_ci.pColorBlendState       = &cb;
	gp_ci.pViewportState         = &vp;
	gp_ci.pDynamicState          = &dyn;
	gp_ci.layout                 = pipeline_layout;
	gp_ci.renderPass             = rp;
	gp_ci.subpass                = 0;

	VkPipeline pipeline;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gp_ci, NULL, &pipeline));

	/* ── framebuffers ─────────────────────────────────────────────────── */
	fputs("frame buffers\n", stderr);
	std::vector<VkFramebuffer> fbv(sc_img_count);
	VkFramebuffer *framebuffers = &fbv[0];
	for (uint32_t i = 0; i < sc_img_count; i++) {
		VkFramebufferCreateInfo fb_ci = {};
		fb_ci.sType            = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_ci.renderPass       = rp;
		fb_ci.attachmentCount  = 1;
		fb_ci.pAttachments     = &sc_views[i];
		fb_ci.width            = extent.width;
		fb_ci.height           = extent.height;
		fb_ci.layers           = 1;
		VK_CHECK(vkCreateFramebuffer(device, &fb_ci, NULL, &framebuffers[i]));
	}

	/* ── command pool & buffers ───────────────────────────────────────── */
	fputs("command pool & buffers\n", stderr);
	VkCommandPoolCreateInfo cp_ci = {};
	cp_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cp_ci.queueFamilyIndex = (uint32_t)qf_idx;
	cp_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool cmd_pool;
	VK_CHECK(vkCreateCommandPool(device, &cp_ci, NULL, &cmd_pool));

	std::vector<VkCommandBuffer> cmd_bufv(sc_img_count);
	VkCommandBuffer *cmd_bufs =  &cmd_bufv[0];
	VkCommandBufferAllocateInfo cb_ai = {};
	cb_ai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cb_ai.commandPool        = cmd_pool;
	cb_ai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cb_ai.commandBufferCount = sc_img_count;
	VK_CHECK(vkAllocateCommandBuffers(device, &cb_ai, cmd_bufs));

	/* ── semaphores & fence ───────────────────────────────────────────── */
	fputs("semaphores & fence\n", stderr);

	std::vector<VkSemaphore>	img_avail_semsv(sc_img_count), render_done_semsv(sc_img_count);
	std::vector<VkFence>		render_fencev(sc_img_count);

	VkSemaphore *image_avail_sems = &img_avail_semsv[0];
	VkSemaphore *render_done_sems = &render_done_semsv[0];
	VkFence     *render_fences    = &render_fencev[0];

	{
		VkSemaphoreCreateInfo se_ci = {};
		se_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for (uint32_t i = 0; i < sc_img_count; i++) {
			VK_CHECK(vkCreateSemaphore(device, &se_ci, NULL, &image_avail_sems[i]));
			VK_CHECK(vkCreateSemaphore(device, &se_ci, NULL, &render_done_sems[i]));
		}
	}
	{
		VkFenceCreateInfo fe_ci = {};
		fe_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fe_ci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (uint32_t i = 0; i < sc_img_count; i++) {
			VK_CHECK(vkCreateFence(device, &fe_ci, NULL, &render_fences[i]));
		}
	}

	/* ── render loop ──────────────────────────────────────────────────── */
	bool running = true;
	fputs("from here is loop\n", stderr);

	uint32_t frame_idx = 0;

	while (running) {
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_EVENT_QUIT)
				running = false;
			if (evt.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
				running = false;
			if (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_Q)
				running = false;
		}

		/* wait for previous frame on this image */
		vkWaitForFences(device, 1, &render_fences[frame_idx], VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &render_fences[frame_idx]);

		/* acquire next swapchain image */
		uint32_t img_idx;
		VkResult acq = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
				image_avail_sems[frame_idx], VK_NULL_HANDLE, &img_idx);
		if (acq == VK_ERROR_OUT_OF_DATE_KHR) {
			continue;
		}
		VK_CHECK(acq);

		VkCommandBuffer& cmd_buf = cmd_bufs[frame_idx];

		/* record command buffer */
		VkCommandBufferBeginInfo cb_bi = {};
		cb_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK(vkBeginCommandBuffer(cmd_buf, &cb_bi));

		/* render pass */
		VkClearValue clear_val = {};
		clear_val.color = {{0.2f, 0.2f, 0.3f, 1.0f}};

		VkRenderPassBeginInfo rp_bi = {};
		rp_bi.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_bi.renderPass       = rp;
		rp_bi.framebuffer      = framebuffers[img_idx];
		rp_bi.renderArea.offset = {0, 0};
		rp_bi.renderArea.extent = extent;
		rp_bi.clearValueCount  = 1;
		rp_bi.pClearValues     = &clear_val;
		vkCmdBeginRenderPass(cmd_buf, &rp_bi, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport vp_val = {};
		vp_val.x = 0.f; vp_val.y = 0.f;
		vp_val.width  = (float)extent.width;
		vp_val.height = (float)extent.height;
		vp_val.minDepth = 0.f; vp_val.maxDepth = 1.f;
		vkCmdSetViewport(cmd_buf, 0, 1, &vp_val);

		VkRect2D sc_val = {};
		sc_val.offset = {0, 0};
		sc_val.extent = extent;
		vkCmdSetScissor(cmd_buf, 0, 1, &sc_val);

		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdDraw(cmd_buf, 3, 1, 0, 0);  /* 3 vertices, 1 instance */

		vkCmdEndRenderPass(cmd_buf);
		VK_CHECK(vkEndCommandBuffer(cmd_buf));

		/* submit — wait on image_avail, signal render_done for this image */
		VkSemaphore wait_sems[] = {image_avail_sems[frame_idx]};
		VkSemaphore signal_sems[] = {render_done_sems[frame_idx]};
		VkPipelineStageFlags wait_stages[] = {
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo sub = {};
		sub.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sub.waitSemaphoreCount  = 1;
		sub.pWaitSemaphores     = wait_sems;
		sub.pWaitDstStageMask   = wait_stages;
		sub.commandBufferCount  = 1;
		sub.pCommandBuffers     = &cmd_buf;
		sub.signalSemaphoreCount = 1;
		sub.pSignalSemaphores    = signal_sems;
		VK_CHECK(vkQueueSubmit(queue, 1, &sub, render_fences[frame_idx]));

		/* present — wait on render_done for this image */
		VkPresentInfoKHR pres = {};
		pres.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		pres.waitSemaphoreCount = 1;
		pres.pWaitSemaphores    = signal_sems;
		pres.swapchainCount     = 1;
		pres.pSwapchains        = &swapchain;
		pres.pImageIndices      = &img_idx;
		VK_CHECK(vkQueuePresentKHR(queue, &pres));

		++frame_idx;
		frame_idx %= sc_img_count;

		SDL_Delay(1);
	}

	/* ── cleanup ──────────────────────────────────────────────────────── */
	vkDeviceWaitIdle(device);

	vkFreeCommandBuffers(device, cmd_pool, sc_img_count, cmd_bufs);
	vkDestroyCommandPool(device, cmd_pool, NULL);

	for (uint32_t i = 0; i < sc_img_count; i++)
		vkDestroyFramebuffer(device, framebuffers[i], NULL);

	vkDestroyPipeline(device, pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	vkDestroyRenderPass(device, rp, NULL);
	vkDestroyShaderModule(device, fs_module, NULL);
	vkDestroyShaderModule(device, vs_module, NULL);
	vkDestroySwapchainKHR(device, swapchain, NULL);

	for (uint32_t i = 0; i < sc_img_count; i++) {
		vkDestroySemaphore(device, image_avail_sems[i], NULL);
		vkDestroySemaphore(device, render_done_sems[i], NULL);
		vkDestroyFence(device, render_fences[i], NULL);
	}

	for(uint32_t i = sc_img_count; i-->0; )vkDestroyImageView(device, sc_views[i], 0);

	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);

	SDL_DestroyWindow(window);
	SDL_Vulkan_UnloadLibrary();
	SDL_Quit();

	return 0;
}
