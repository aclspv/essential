#include <aclspv.hh>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <vector>

#define VK_CHECK(call)                                                        \
	do {                                                                      \
		VkResult _r = (call);                                                 \
		if (_r != VK_SUCCESS && _r != VK_SUBOPTIMAL_KHR) {                  \
			fprintf(stderr, "%s failed: %d\n", #call, (int)_r);               \
			exit(EXIT_FAILURE);                                               \
		}                                                                     \
	} while (0)

struct PushData {
	float mx;
	float my;
	float aspect;
	float pad;
};

/* ---- shader source -------------------------------------------------- */
/*
 * what this example exercises, end to end:
 *
 *   lane_of     always_inline, early return through a chain of ifs  -> carved
 *   palette     always_inline, nested if/else returning             -> carved
 *   lane_shape  always_inline, switch returning from every case     -> carved
 *   clamp01     plain `inline` (key word only)                      -> call
 *   smooth      plain `inline`                                      -> call
 *   seg2        plain `inline`                                      -> call
 *   tri         plain `inline`, calls seg2 three times              -> call
 *   scan        plain `inline`, a loop with an early return         -> call
 *   ring        `noinline`                                          -> DontInline
 *
 * every OpFunction this module emits says `Inline` unless the source said
 * `noinline`; only the call sites decide what is carved. build the same
 * binary with ACLSPV_INLINE_BY_DEFAULT=1 (or comp.cc --flag 0x100) and the
 * `inline` helpers are carved too: identical shader, different module
 * (OpFunctionCall 12 -> 3).
 *
 * what you should see: four vertical lanes (red / green / blue / amber).
 * every lane is banded, and the lane the pointer is in gets a small upright
 * triangle with a bright outline near its middle. a soft white spot follows
 * the pointer. q quits.
 */
static const char src[] =
	"typedef float f32;\n"
	"typedef f32 __attribute__((ext_vector_type(4))) f32x4;\n"
	"typedef f32 __attribute__((ext_vector_type(3))) f32x3;\n"
	"typedef f32 __attribute__((ext_vector_type(2))) f32x2;\n"
	"\n"
	"struct PushData { f32 mx; f32 my; f32 aspect; f32 pad; };\n"
	"\n"
	"/* ---- helpers ---------------------------------------------------------- */\n"
	"/* which of the four lanes a horizontal position falls in */\n"
	"__attribute__((always_inline))\n"
	"int lane_of(f32 u) {\n"
	"    if (u < 0.25f) return 0;\n"
	"    if (u < 0.50f) return 1;\n"
	"    if (u < 0.75f) return 2;\n"
	"    return 3;\n"
	"}\n"
	"\n"
	"/* the lane's base colour, and its highlighted variant. every branch\n"
	" * returns: a carved body with returns is what the loop scaffold is for. */\n"
	"__attribute__((always_inline))\n"
	"f32x3 palette(int lane, int hot) {\n"
	"    if (lane == 0) {\n"
	"        if (hot) return (f32x3){ 1.00f, 0.62f, 0.56f };\n"
	"        return (f32x3){ 0.78f, 0.20f, 0.16f };\n"
	"    }\n"
	"    if (lane == 1) {\n"
	"        if (hot) return (f32x3){ 0.62f, 1.00f, 0.72f };\n"
	"        return (f32x3){ 0.16f, 0.68f, 0.28f };\n"
	"    }\n"
	"    if (lane == 2) {\n"
	"        if (hot) return (f32x3){ 0.64f, 0.78f, 1.00f };\n"
	"        return (f32x3){ 0.16f, 0.32f, 0.80f };\n"
	"    }\n"
	"    if (hot) return (f32x3){ 1.00f, 0.90f, 0.50f };\n"
	"    return (f32x3){ 0.70f, 0.54f, 0.14f };\n"
	"}\n"
	"\n"
	"/* a switch that returns from every arm: the merge is unreachable */\n"
	"__attribute__((always_inline))\n"
	"f32 lane_shape(int lane, f32 u) {\n"
	"    switch (lane) {\n"
	"    case 0: return u * 4.0f;\n"
	"    case 1: return (u - 0.25f) * 4.0f;\n"
	"    case 2: return (u - 0.50f) * 4.0f;\n"
	"    default: return (u - 0.75f) * 4.0f;\n"
	"    }\n"
	"}\n"
	"\n"
	"inline f32 clamp01(f32 x) {\n"
	"    if (x < 0.0f) return 0.0f;\n"
	"    if (x > 1.0f) return 1.0f;\n"
	"    return x;\n"
	"}\n"
	"\n"
	"inline f32 smooth(f32 x) {\n"
	"    f32 c = clamp01(x);\n"
	"    return c * c * (3.0f - 2.0f * c);\n"
	"}\n"
	"\n"
	"/* squared distance to a segment, the edge included */\n"
	"inline f32 seg2(f32 px, f32 py, f32 ax, f32 ay, f32 bx, f32 by) {\n"
	"    f32 vx = bx - ax;\n"
	"    f32 vy = by - ay;\n"
	"    f32 wx = px - ax;\n"
	"    f32 wy = py - ay;\n"
	"    f32 t = (wx * vx + wy * vy) / (vx * vx + vy * vy);\n"
	"    if (t < 0.0f) t = 0.0f;\n"
	"    if (t > 1.0f) t = 1.0f;\n"
	"    f32 dx = wx - t * vx;\n"
	"    f32 dy = wy - t * vy;\n"
	"    return dx * dx + dy * dy;\n"
	"}\n"
	"\n"
	"/* apex-up triangle: inside only when inside all three half planes */\n"
	"inline f32 tri(f32 px, f32 py, f32 cx, f32 cy, f32 r) {\n"
	"    f32 ax = cx;\n"
	"    f32 ay = cy - r;\n"
	"    f32 bx = cx - r * 0.866f;\n"
	"    f32 by = cy + r * 0.5f;\n"
	"    f32 qx = cx + r * 0.866f;\n"
	"    f32 qy = cy + r * 0.5f;\n"
	"    f32 e = seg2(px, py, ax, ay, bx, by);\n"
	"    f32 e2 = seg2(px, py, bx, by, qx, qy);\n"
	"    if (e2 < e) e = e2;\n"
	"    e2 = seg2(px, py, qx, qy, ax, ay);\n"
	"    if (e2 < e) e = e2;\n"
	"    int i0 = (bx - ax) * (py - ay) - (by - ay) * (px - ax) < 0.0f;\n"
	"    int i1 = (qx - bx) * (py - by) - (qy - by) * (px - bx) < 0.0f;\n"
	"    int i2 = (ax - qx) * (py - qy) - (ay - qy) * (px - qx) < 0.0f;\n"
	"    if (i0 && i1 && i2) return -e;\n"
	"    return e;\n"
	"}\n"
	"\n"
	"/* never inlined: FunctionControl says DontInline and the call stays */\n"
	"__attribute__((noinline))\n"
	"f32 ring(f32 d, f32 w) {\n"
	"    f32 a = d / w;\n"
	"    if (a < 0.0f) a = -a;\n"
	"    f32 k = 1.0f - a;\n"
	"    if (k < 0.0f) return 0.0f;\n"
	"    return k * k;\n"
	"}\n"
	"\n"
	"/* a loop with an early return: a fragment may do this, a carved body may\n"
	" * not (the inliner refuses a return nested inside a loop). */\n"
	"inline f32 scan(f32 v, int n) {\n"
	"    f32 acc = 0.0f;\n"
	"    for (int i = 0; i < n; i = i + 1) {\n"
	"        f32 s = v * (f32)(i + 1);\n"
	"        s = s - (f32)(int)s;\n"
	"        acc = acc + s;\n"
	"        if (acc > 2.5f) return acc;\n"
	"    }\n"
	"    return acc;\n"
	"}\n"
	"\n"
	"/* ---- vertex ----------------------------------------------------------- */\n"
	"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n"
	"void vs_main(\n"
	"    __attribute__((annotate(\"aclspv_storage_class(9)\")))\n"
	"    struct PushData* pc,\n"
	"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
	"    __attribute__((annotate(\"aclspv_bltin(42)\")))\n"
	"    int* in_vertidx,\n"
	"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
	"    __attribute__((annotate(\"aclspv_bltin(0)\")))\n"
	"    f32x4* out_pos,\n"
	"    __attribute__((annotate(\"aclspv_location(1)\")))\n"
	"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
	"    f32x2* out_uv\n"
	") {\n"
	"    int vid = *in_vertidx;\n"
	"    /* a triangle that covers the screen: two corners far outside */\n"
	"    if (vid == 0) *out_pos = (f32x4){ -1.0f, -1.0f, 0.0f, 1.0f };\n"
	"    else if (vid == 1) *out_pos = (f32x4){ 3.0f, -1.0f, 0.0f, 1.0f };\n"
	"    else *out_pos = (f32x4){ -1.0f, 3.0f, 0.0f, 1.0f };\n"
	"\n"
	"    /* uv has to read 0..1 across the WINDOW. the covering triangle has its\n"
	"     * far corners at +3, so a window point at fraction f along both axes is\n"
	"     * barycentric (1-2f, f/2, f/2) in it -- hence the corner warp below,\n"
	"     * which lands uv on (f, f) exactly. (0,0) is the top-left corner and\n"
	"     * (1,1) the bottom-right one. */\n"
	"    if (vid == 0) *out_uv = (f32x2){ 0.0f, 0.0f };\n"
	"    else if (vid == 1) *out_uv = (f32x2){ 4.0f, 0.0f };\n"
	"    else *out_uv = (f32x2){ 0.0f, 4.0f };\n"
	"\n"
	"    /* keeps the push constant live in the vertex stage */\n"
	"    if (pc->aspect < 0.0f) *out_uv = (f32x2){ -1.0f, -1.0f };\n"
	"}\n"
	"\n"
	"/* ---- fragment --------------------------------------------------------- */\n"
	"__attribute__((annotate(\"aclspv_execmode0(7)\")))\n"
	"__attribute__((annotate(\"aclspv_execmodel(4)\")))\n"
	"void fs_main(\n"
	"    __attribute__((annotate(\"aclspv_storage_class(9)\")))\n"
	"    struct PushData* pc,\n"
	"    __attribute__((annotate(\"aclspv_location(1)\")))\n"
	"    __attribute__((annotate(\"aclspv_storage_class(1)\")))\n"
	"    f32x2* in_uv,\n"
	"    __attribute__((annotate(\"aclspv_location(0)\")))\n"
	"    __attribute__((annotate(\"aclspv_storage_class(3)\")))\n"
	"    f32x4* out_color\n"
	") {\n"
	"    f32x2 uv = *in_uv;\n"
	"    f32 u = uv[0];\n"
	"    f32 v = uv[1];\n"
	"\n"
	"    int lane = lane_of(u);\n"
	"\n"
	"    /* the triangle sits in the lane the pointer is in */\n"
	"    int hl = lane_of(pc->mx * 0.5f + 0.5f);\n"
	"\n"
	"    f32 cx = 1.0f / 8.0f + (f32)hl * 0.25f;\n"
	"    f32 cy = 0.5f;\n"
	"    f32 r = 0.13f;\n"
	"    f32 d = tri(u, v, cx, cy, r);\n"
	"    int inside = d < 0.0f;\n"
	"\n"
	"    /* body: palette colour, brighter toward the middle */\n"
	"    f32 shade = smooth((d + r) / (r * 2.0f));\n"
	"    f32x3 c = palette(lane, inside ? 1 : 0);\n"
	"    f32 x = c[0] * (0.30f + 0.70f * shade);\n"
	"    f32 y = c[1] * (0.30f + 0.70f * shade);\n"
	"    f32 z = c[2] * (0.30f + 0.70f * shade);\n"
	"\n"
	"    /* a hard bright outline on the triangle's boundary */\n"
	"    f32 ad = d < 0.0f ? -d : d;\n"
	"    f32 edge = ring(ad - 0.004f, 0.0030f);\n"
	"    c = (f32x3){ x + edge, y + edge, z + edge };\n"
	"\n"
	"    /* background: banded stripes per lane, darker far from the triangle */\n"
	"    f32 lm = lane_shape(lane, u);\n"
	"    f32 bands = scan(lm * 1.5f + v * 2.0f, 4);\n"
	"    f32 gk = 0.55f + 0.45f * (bands * 0.25f);\n"
	"    if (!inside) {\n"
	"        c = (f32x3){ c[0] * gk, c[1] * gk, c[2] * gk };\n"
	"    }\n"
	"\n"
	"    /* pointer spotlight, in window space (uv reads 0..1 now) */\n"
	"    f32 mxu = pc->mx * 0.5f + 0.5f;\n"
	"    f32 myu = pc->my * 0.5f + 0.5f;\n"
	"    f32 dx = u - mxu;\n"
	"    f32 dy = (v - myu) * pc->aspect;\n"
	"    f32 spot = ring(dx * dx + dy * dy, 0.010f);\n"
	"\n"
	"    c = (f32x3){\n"
	"        clamp01(c[0] + spot * 0.55f),\n"
	"        clamp01(c[1] + spot * 0.55f),\n"
	"        clamp01(c[2] + spot * 0.55f)\n"
	"    };\n"
	"\n"
	"    *out_color = (f32x4){ c[0], c[1], c[2], 1.0f };\n"
	"}\n";


/* ── compile the shader → SPIR-V ───────────────────────────────────────── */
template<size_t len>
static aclspv_wrd_t *compile_spirv(const char src[len], size_t *out_words) {
	if (getenv("ACLSPV_DUMP_SRC")) fputs(src, stderr);

	aclspv::ctx ctx(aclspv::usrcfg_t()
			.flag0(
				ACLSPV_CTX_FLAG0_SYMBOL 
				| ACLSPV_CTX_FLAG0_DUMP_DIAGNOSTICS
				)
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

	aclspv_wrdcount_t words;
	aclspv_wrd_t *spirv;
	ret = ctx.dump_output(words, spirv);
	if (ret != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "dump_output failed: %d\n", (int)ret);
		return NULL;
	}

	if (getenv("PATH_SHADER")) {
		FILE *f = fopen(getenv("PATH_SHADER"), "wb");
		if (f) { fwrite(spirv, sizeof(*spirv), (size_t)words, f); fclose(f); }
	}

	*out_words = (size_t)words;
	return spirv;
}

int main(void) {
	size_t shader_words = 0;
	aclspv_wrd_t *spirv = compile_spirv<sizeof(src)>(src, &shader_words);
	if (!shader_words || !spirv) return EXIT_FAILURE;
	fputs("aclspv: shader compiled\n", stderr);

	/* ── SDL window ─────────────────────────────────────────────────── */
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
	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING,
			"aclspv: inline + switch (h: hover highlights, top-left: flat hint)");
	SDL_Window *window = SDL_CreateWindowWithProperties(props);
	SDL_DestroyProperties(props);
	if (!window) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return EXIT_FAILURE; }
	SDL_ShowWindow(window);

	/* ── instance ───────────────────────────────────────────────────── */
	uint32_t ext_count = 0;
	const char * const *exts = SDL_Vulkan_GetInstanceExtensions(&ext_count);

	VkApplicationInfo app = {};
	app.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app.pApplicationName   = "aclspv inline switch";
	app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app.pEngineName        = "aclspv";
	app.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	app.apiVersion         = VK_API_VERSION_1_1;

	VkInstanceCreateInfo inst_ci = {};
	inst_ci.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_ci.pApplicationInfo        = &app;
	inst_ci.enabledExtensionCount   = ext_count;
	inst_ci.ppEnabledExtensionNames = exts;

	VkInstance instance;
	VK_CHECK(vkCreateInstance(&inst_ci, NULL, &instance));

	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, instance, NULL, &surface)) {
		fprintf(stderr, "SDL_Vulkan_CreateSurface: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	/* ── physical device & queue family ─────────────────────────────── */
	uint32_t phys_count = 0;
	vkEnumeratePhysicalDevices(instance, &phys_count, NULL);
	if (!phys_count) { fprintf(stderr, "no physical device\n"); return EXIT_FAILURE; }
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
		if ((qfsv[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && can_present) { qf_idx = (int)i; break; }
	}
	if (qf_idx < 0) { fprintf(stderr, "no graphics+present queue\n"); return EXIT_FAILURE; }

	float qprio = 1.0f;
	VkDeviceQueueCreateInfo q_ci = {};
	q_ci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	q_ci.queueFamilyIndex = (uint32_t)qf_idx;
	q_ci.queueCount       = 1;
	q_ci.pQueuePriorities = &qprio;

	const char *dev_exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	VkDeviceCreateInfo dev_ci = {};
	dev_ci.sType                  = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_ci.queueCreateInfoCount   = 1;
	dev_ci.pQueueCreateInfos      = &q_ci;
	dev_ci.enabledExtensionCount  = 1;
	dev_ci.ppEnabledExtensionNames = dev_exts;

	VkDevice device;
	VK_CHECK(vkCreateDevice(phys_dev, &dev_ci, NULL, &device));
	VkQueue queue;
	vkGetDeviceQueue(device, (uint32_t)qf_idx, 0, &queue);

	/* ── swapchain ──────────────────────────────────────────────────── */
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_dev, surface, &caps);

	uint32_t fmt_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &fmt_count, NULL);
	std::vector<VkSurfaceFormatKHR> fmtv(fmt_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys_dev, surface, &fmt_count, &fmtv[0]);
	VkSurfaceFormatKHR surf_fmt = ((fmt_count == 1) && (fmtv[0].format == VK_FORMAT_UNDEFINED))
		? VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
		: fmtv[0];

	VkExtent2D extent;
	if (caps.currentExtent.width != (uint32_t)-1) extent = caps.currentExtent;
	else {
		int w = 800, h = 800;
		SDL_GetWindowSizeInPixels(window, &w, &h);
		extent.width  = SDL_clamp((uint32_t)w, caps.minImageExtent.width,  caps.maxImageExtent.width);
		extent.height = SDL_clamp((uint32_t)h, caps.minImageExtent.height, caps.maxImageExtent.height);
	}

	uint32_t image_count = caps.minImageCount + 1;
	if (caps.maxImageCount && image_count > caps.maxImageCount) image_count = caps.maxImageCount;

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
	sc_ci.presentMode      = getenv("ACLSPV_IMMEDIATE")
		? VK_PRESENT_MODE_IMMEDIATE_KHR
		: VK_PRESENT_MODE_FIFO_KHR;
	sc_ci.clipped          = VK_TRUE;

	VkSwapchainKHR swapchain;
	VK_CHECK(vkCreateSwapchainKHR(device, &sc_ci, NULL, &swapchain));

	uint32_t sc_img_count = 0;
	vkGetSwapchainImagesKHR(device, swapchain, &sc_img_count, NULL);
	std::vector<VkImage> sc_images(sc_img_count);
	vkGetSwapchainImagesKHR(device, swapchain, &sc_img_count, &sc_images[0]);
	fprintf(stderr, "swapchain images=%u (min=%u max=%u)\n",
			sc_img_count, caps.minImageCount, caps.maxImageCount);

	std::vector<VkImageView> sc_views(sc_img_count);
	for (uint32_t i = 0; i < sc_img_count; i++) {
		VkImageViewCreateInfo iv_ci = {};
		iv_ci.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iv_ci.image    = sc_images[i];
		iv_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iv_ci.format   = surf_fmt.format;
		iv_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		iv_ci.subresourceRange.levelCount = 1;
		iv_ci.subresourceRange.layerCount = 1;
		VK_CHECK(vkCreateImageView(device, &iv_ci, NULL, &sc_views[i]));
	}

	/* ── render pass ────────────────────────────────────────────────── */
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

	/* ── pipeline layout (push constant: mouse + aspect) ────────────── */
	VkPushConstantRange pc_range = {};
	pc_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pc_range.offset     = 0;
	pc_range.size       = sizeof(PushData);

	VkPipelineLayoutCreateInfo pl_ci = {};
	pl_ci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl_ci.pushConstantRangeCount = 1;
	pl_ci.pPushConstantRanges    = &pc_range;

	VkPipelineLayout pipeline_layout;
	VK_CHECK(vkCreatePipelineLayout(device, &pl_ci, NULL, &pipeline_layout));

	/* ── shader module ──────────────────────────────────────────────── */
	VkShaderModuleCreateInfo sm_ci = {};
	sm_ci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	sm_ci.codeSize = shader_words * sizeof(aclspv_wrd_t);
	sm_ci.pCode    = spirv;

	VkShaderModule module;
	VK_CHECK(vkCreateShaderModule(device, &sm_ci, NULL, &module));
	free(spirv);

	/* ── graphics pipeline (fullscreen triangle) ────────────────────── */
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
	cb_att.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

	const VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dyn = {};
	dyn.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn.dynamicStateCount = 2;
	dyn.pDynamicStates    = dyn_states;

	VkPipelineViewportStateCreateInfo vp = {};
	vp.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vp.viewportCount = 1;
	vp.scissorCount  = 1;

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

	/* ── framebuffers / command pool / sync ─────────────────────────── */
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

	/** frames in flight, kept BELOW the swapchain image count so the next
	 * acquire always has an image free to hand back. tying these 1:1 makes
	 * vkAcquireNextImageKHR block through every image once they are all
	 * submitted but not yet re-presented — the 150ms stall observed. */
	const uint32_t frames_in_flight = 2;

	std::vector<VkCommandBuffer> cmd_bufs(frames_in_flight);
	VkCommandBufferAllocateInfo cb_ai = {};
	cb_ai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cb_ai.commandPool        = cmd_pool;
	cb_ai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cb_ai.commandBufferCount = frames_in_flight;
	VK_CHECK(vkAllocateCommandBuffers(device, &cb_ai, &cmd_bufs[0]));

	std::vector<VkSemaphore> img_avail_sems(frames_in_flight), render_done_sems(frames_in_flight);
	std::vector<VkFence> render_fences(frames_in_flight);
	VkSemaphoreCreateInfo se_ci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkFenceCreateInfo fe_ci = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, NULL, VK_FENCE_CREATE_SIGNALED_BIT};
	for (uint32_t i = 0; i < frames_in_flight; i++) {
		VK_CHECK(vkCreateSemaphore(device, &se_ci, NULL, &img_avail_sems[i]));
		VK_CHECK(vkCreateSemaphore(device, &se_ci, NULL, &render_done_sems[i]));
		VK_CHECK(vkCreateFence(device, &fe_ci, NULL, &render_fences[i]));
	}

	/* ── render loop ────────────────────────────────────────────────── */
	bool running = true;
	uint32_t frame_idx = 0;
	float mouse_x = 0.0f, mouse_y = 0.0f;
	uint64_t perf_freq = SDL_GetPerformanceFrequency();
	uint64_t last_tick = SDL_GetPerformanceCounter();
	uint64_t nframes = 0;

	fputs("aclspv: render loop\n", stderr);

	while (running) {
		SDL_Event evt;
		/** never let the mouse starve: drain every queued event, then do the
		 * (possibly blocking) GPU work. both waits below are bounded, and the
		 * fence wait is dropped entirely when the GPU is already idle. */
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

		if (!running)
			break;

		/** a hidden / minimized window has nobody to present to: compositors
		 * stop handing out swapchain images, so vkAcquireNextImageKHR would
		 * block here (the 150ms stall). instead, poll events and nap, and
		 * start drawing again the instant the window is shown. */
		{
			SDL_WindowFlags wf = SDL_GetWindowFlags(window);
			if (wf & (SDL_WINDOW_MINIMIZED | SDL_WINDOW_HIDDEN | SDL_WINDOW_OCCLUDED)) {
				SDL_Delay(16);
				continue;
			}
		}

		uint32_t img_idx;
		VkResult acq = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
				img_avail_sems[frame_idx], VK_NULL_HANDLE, &img_idx);
		if (acq == VK_ERROR_OUT_OF_DATE_KHR) continue;
		VK_CHECK(acq);

		/** the frame slot is free to reuse once its fence is signalled.
		 * wait only while it is still in flight, then reset and hand it
		 * right back to the submit below. */
		if (vkGetFenceStatus(device, render_fences[frame_idx]) == VK_NOT_READY)
			VK_CHECK(vkWaitForFences(device, 1, &render_fences[frame_idx], VK_TRUE, UINT64_MAX));
		VK_CHECK(vkResetFences(device, 1, &render_fences[frame_idx]));

		VkCommandBuffer cmd = cmd_bufs[frame_idx];
		VkCommandBufferBeginInfo cb_bi = {};
		cb_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK(vkBeginCommandBuffer(cmd, &cb_bi));

		VkClearValue clear_val = {};
		clear_val.color = {{0.10f, 0.11f, 0.14f, 1.0f}};

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

		int win_w = 1, win_h = 1;
		win_w = (int)extent.width;
		win_h = (int)extent.height;

		PushData pc = {};
		pc.mx     = (mouse_x / (float)(win_w ? win_w : 1)) * 2.0f - 1.0f;
		pc.my     = (mouse_y / (float)(win_h ? win_h : 1)) * 2.0f - 1.0f;
		pc.aspect = (float)extent.width / (float)extent.height;

		vkCmdPushConstants(cmd, pipeline_layout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(pc), &pc);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdDraw(cmd, 3, 1, 0, 0);
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

		frame_idx = (frame_idx + 1) % frames_in_flight;

		/** don't throttle: FIFO present mode already paces us at vsync, and
		 * an extra sleep here is what makes mouse motion feel rubbery. */
	}

	/* ── cleanup ────────────────────────────────────────────────────── */
	vkDeviceWaitIdle(device);

	vkFreeCommandBuffers(device, cmd_pool, frames_in_flight, &cmd_bufs[0]);
	vkDestroyCommandPool(device, cmd_pool, NULL);
	for (uint32_t i = 0; i < sc_img_count; i++) {
		vkDestroyFramebuffer(device, framebuffers[i], NULL);
		vkDestroyImageView(device, sc_views[i], NULL);
	}
	for (uint32_t i = 0; i < frames_in_flight; i++) {
		vkDestroySemaphore(device, img_avail_sems[i], NULL);
		vkDestroySemaphore(device, render_done_sems[i], NULL);
		vkDestroyFence(device, render_fences[i], NULL);
	}
	vkDestroyPipeline(device, pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	vkDestroyRenderPass(device, rp, NULL);
	vkDestroyShaderModule(device, module, NULL);
	vkDestroySwapchainKHR(device, swapchain, NULL);
	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);

	SDL_DestroyWindow(window);
	SDL_Vulkan_UnloadLibrary();
	SDL_Quit();

	return EXIT_SUCCESS;
}
