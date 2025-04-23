#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <optional>
#include <set>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string.h>
#include <algorithm>

using namespace std::chrono_literals;

const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifndef NDEBUG
constexpr bool enable_validation_layers = true;
#else 
constexpr bool enable_validation_layers = false;
#endif

struct QueueFamilyIndices;
struct SwapChainSupportDetails;

VkResult create_debug_utils_messenger(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger);

bool check_validation_layers();

std::vector<const char*> get_required_extensions();

constexpr uint32_t HEIGHT = 600;
constexpr uint32_t WIDTH = 800;

class Application
{
public:
	void run();

private:
	void init_vulkan();
	void init_window();
	void init_debug_messenger();
	void cleanup();

	void engine_loop();
	void draw_frame();
	
	void create_logical_device();
	void create_surface();
	void create_instance();
	void create_swap_chain();
	void create_image_views();
	void create_graphics_pipeline();
	void create_render_pass();
	void create_framebuffers();
	void create_command_pool();
	void create_command_buffer();
	void create_sync_objs();

	void record_command_buffer( VkCommandBuffer buffer, uint32_t image_index );
	
	bool check_device_extension_support(VkPhysicalDevice device);
	void pick_physical_device();
	bool is_device_suitable(VkPhysicalDevice device);
	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_modes);
	VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
	QueueFamilyIndices find_queue_family(VkPhysicalDevice device);
	SwapChainSupportDetails find_chain_support_details(VkPhysicalDevice device);
	VkShaderModule create_shader_module(const std::vector<char>& code);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_types,
		const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
		void* user_data);
	static VkDebugUtilsMessengerCreateInfoEXT make_create_info();

	static std::vector<char> get_file_content(const std::string& path);

	VkSurfaceKHR m_surface = nullptr;
	SDL_Window* m_window = nullptr;
	VkInstance m_instance = nullptr;
	VkDebugUtilsMessengerEXT m_debug_messenger = nullptr;
	VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	VkDevice m_device = nullptr;
	VkQueue m_graphics_queue = nullptr;
	VkPipelineLayout m_pipeline_layout;
	VkQueue m_present_queue = nullptr;
		VkPipeline m_pipeline = nullptr;

		VkRenderPass m_render_pass = nullptr;
	VkSwapchainKHR m_swapchain = nullptr;
	std::vector<VkImage> m_swapchain_images;
	std::vector<VkImageView> m_swapchain_image_views;
	std::vector<VkFramebuffer> m_framebuffers;
	VkFormat m_swapchain_format{};
	VkExtent2D m_swapchain_extent{};

	VkCommandPool m_command_pool = nullptr;
	VkCommandBuffer m_command_buffer = nullptr;

	VkSemaphore m_image_availabale;
	VkSemaphore m_render_finished;
	VkFence m_in_flight;
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	bool complete() { return graphics_family.has_value() && present_family.has_value(); }
};

void Application::draw_frame()
{
	vkWaitForFences(m_device, 1, &m_in_flight, VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(m_device, 1, &m_in_flight);

	uint32_t image_id;
	vkAcquireNextImageKHR(m_device, m_swapchain, std::numeric_limits<uint64_t>::max(), m_image_availabale, VK_NULL_HANDLE, &image_id);

	vkResetCommandBuffer(m_command_buffer, 0);
	record_command_buffer(m_command_buffer, image_id);

	VkSemaphore wait_sems[] = {m_image_availabale};
	VkSemaphore signal_sems[] = {m_render_finished};
	VkSwapchainKHR swapchains[] = {m_swapchain};
	VkPipelineStageFlags stage_masks[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_sems;
	submit_info.pWaitDstStageMask = stage_masks;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &m_command_buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_sems;

	if( vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight) != VK_SUCCESS )
		throw std::runtime_error("cannot submit queue");

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_sems;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &image_id;
	present_info.pResults = nullptr;

	vkQueuePresentKHR(m_graphics_queue, &present_info);
}

void Application::create_sync_objs()
{
	VkSemaphoreCreateInfo sem_info{};
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_inf{};
	fence_inf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_inf.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if(
		vkCreateSemaphore(m_device, &sem_info, nullptr, &m_image_availabale) != VK_SUCCESS ||
		vkCreateSemaphore(m_device, &sem_info, nullptr, &m_render_finished) != VK_SUCCESS ||
		vkCreateFence(m_device, &fence_inf, nullptr, &m_in_flight) != VK_SUCCESS
	)
		throw std::runtime_error("cannot create syncronisation objects");
}

void Application::create_command_buffer()
{
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = m_command_pool;
	alloc_info.commandBufferCount = 1;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	if( vkAllocateCommandBuffers(m_device, &alloc_info, &m_command_buffer) != VK_SUCCESS)
		throw std::runtime_error("cannot create command buffer");
}

void Application::record_command_buffer( VkCommandBuffer buffer, uint32_t image_index )
{
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if( vkBeginCommandBuffer(m_command_buffer, &begin_info) != VK_SUCCESS)
		throw std::runtime_error("failed to begin command buffre");

	VkRenderPassBeginInfo pass_begin_info{};
	pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	pass_begin_info.renderPass = m_render_pass;
	
	pass_begin_info.framebuffer = m_framebuffers[image_index];
	pass_begin_info.renderArea.extent = m_swapchain_extent;
	pass_begin_info.renderArea.offset = {0, 0};

	VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	pass_begin_info.clearValueCount = 1;
	pass_begin_info.pClearValues = &clearColor;


	vkCmdBeginRenderPass(m_command_buffer, &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapchain_extent.width);
		viewport.height = static_cast<float>(m_swapchain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = m_swapchain_extent;
		vkCmdSetScissor(m_command_buffer, 0, 1, &scissor);

		vkCmdDraw(m_command_buffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(m_command_buffer);

	if( vkEndCommandBuffer(m_command_buffer) != VK_SUCCESS)
		throw std::runtime_error("cannot end command buffer");
}

void Application::create_command_pool()
{
	auto queue_family = find_queue_family(m_physical_device);

	VkCommandPoolCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.queueFamilyIndex = queue_family.graphics_family.value();
	create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if( vkCreateCommandPool(m_device, &create_info, nullptr, &m_command_pool) != VK_SUCCESS )
		throw std::runtime_error("cannot create command pool");
}

void Application::create_framebuffers()
{
	m_framebuffers.resize( m_swapchain_image_views.size() );

	for(size_t i = 0; i < m_framebuffers.size(); ++i )
	{
		VkImageView attachments[] = {
			m_swapchain_image_views[i]
		};

		VkFramebufferCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		create_info.renderPass = m_render_pass;
		create_info.attachmentCount = 1;
		create_info.pAttachments = attachments;
		create_info.width = m_swapchain_extent.width;
		create_info.height = m_swapchain_extent.height;
		create_info.layers = 1;

		if( vkCreateFramebuffer(m_device, &create_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("cannot create frambuffers");
	}
}

VkShaderModule Application::create_shader_module(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;

	if (vkCreateShaderModule(m_device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
		throw std::runtime_error("Cannot create shader module");

	return shader_module;
}

std::vector<char> Application::get_file_content(const std::string& path)
{
	std::ifstream file{ path, std::ios::ate | std::ios::binary };

	if (!file.is_open())
		throw std::runtime_error("cannot open file " + path);

	size_t file_size = file.tellg();
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), file_size);

	file.close();
	return buffer;
}

QueueFamilyIndices Application::find_queue_family(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queue_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, nullptr);

	std::vector<VkQueueFamilyProperties> queues_properties(queue_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_count, queues_properties.data());

	/*
		TODO: remarque: graphics_family et present_family peuvent �tre diff�rents mais il peut �tre 
		int�r�ssant de pr�f�rer une famille qui propose les deux � la fois pour de 
		meilleurs performances
	*/
	int i = 0;
	for (const auto& queue : queues_properties)
	{
		// can do graphics
		if(queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphics_family = i;

		// can present to a surface
		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &present_support);

		if (present_support)
			indices.present_family = i;

		if (indices.complete()) break;

		++i;
	}

	return indices;
}

SwapChainSupportDetails Application::find_chain_support_details(VkPhysicalDevice device)
{
	SwapChainSupportDetails details{};

	// details.capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

	// details.formats
	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);
	if (format_count != 0)
	{
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, details.formats.data());
	}

	// details.present_modes
	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr);
	if (present_mode_count != 0)
	{
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, details.present_modes.data());
	}

	return details;
}

VkDebugUtilsMessengerCreateInfoEXT Application::make_create_info()
{
	VkDebugUtilsMessengerCreateInfoEXT create_info{};

	create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	create_info.pNext = nullptr;
	create_info.pUserData = nullptr;
	create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	create_info.pfnUserCallback = debug_callback;

	return create_info;
}

std::vector<const char*> get_required_extensions()
{
	uint32_t ext_count{ 0 };
	const char *const *ext_names = SDL_Vulkan_GetInstanceExtensions(&ext_count);

	std::vector<const char*> required_exts(ext_names, ext_names + ext_count);

	if (enable_validation_layers)
		required_exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return required_exts;
}

VkResult create_debug_utils_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void destroy_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks* allocator)
{
	auto fonc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (fonc != nullptr)
		fonc(instance, messenger, allocator);
}

bool Application::is_device_suitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	std::cout << "GPU: " << properties.deviceName << std::endl;


	auto family = find_queue_family(device);

	bool device_supports_extensions = check_device_extension_support(device);
	bool swap_chain_adequate = false;
	if (device_supports_extensions)
	{
		auto swapchain_support_details = find_chain_support_details(device);
		swap_chain_adequate = !swapchain_support_details.formats.empty() && !swapchain_support_details.present_modes.empty();
	}

	return family.complete() && device_supports_extensions && swap_chain_adequate;
}

bool check_validation_layers()
{
	uint32_t layer_count{ 0 };
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	for (const char* layer_name : validation_layers)
	{
		bool found{ false };
		for (VkLayerProperties& prop : available_layers)
		{
			if (strcmp(layer_name, prop.layerName) == 0)
				found = true;
		}

		if (!found) return false;
	}

	return true;
}

bool Application::check_device_extension_support(VkPhysicalDevice device)
{
	uint32_t properties_count = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &properties_count, nullptr);

	std::vector<VkExtensionProperties> properties(properties_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &properties_count, properties.data());

	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

	for (const auto& extension : properties)
	{
		//std::cout << extension.extensionName << " ";
		required_extensions.erase(extension.extensionName);
	}

	//std::cout << std::endl;

	return required_extensions.empty();
}

VkExtent2D Application::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else
	{
		int width, height;
		SDL_GetWindowSize(m_window, &width, &height);

		VkExtent2D extent = {
			std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};

		return extent;
	}
}

VkPresentModeKHR Application::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_modes)
{
	for (const auto& mode : available_modes)
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Application::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats)
{
	for (const auto& format : available_formats)
	{
		if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
			return format;
	}

	return available_formats[0];
}

void Application::create_image_views()
{
	m_swapchain_image_views.resize(m_swapchain_images.size());
	for (size_t i = 0; i < m_swapchain_image_views.size(); ++i)
	{
		VkImageViewCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = m_swapchain_images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = m_swapchain_format;

		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device, &create_info, nullptr, &m_swapchain_image_views[i]) != VK_SUCCESS)
			throw std::runtime_error("cannot create image views");
	}
}

void Application::create_logical_device()
{
	QueueFamilyIndices indices = find_queue_family(m_physical_device);
	//TODO: ne faut-il pas vérifier que m_physical device possède bien ces deux valeurs? si oui on n'appelle pas la fonction find_queue_family deux fois?
	std::set<uint32_t> unique_queue_families = { indices.graphics_family.value(), indices.present_family.value() };
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

	float queue_priority = 1.0f;
	for (auto& family_index : unique_queue_families)
	{
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueCount = 1;
		queue_create_info.queueFamilyIndex = family_index;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures features{};

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = static_cast<uint32_t>( queue_create_infos.size() );
	device_create_info.pEnabledFeatures = &features;
	device_create_info.ppEnabledExtensionNames = device_extensions.data();
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());

	if (enable_validation_layers)
	{
		device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		device_create_info.ppEnabledLayerNames = validation_layers.data();
	}
	else
	{
		device_create_info.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device) != VK_SUCCESS)
		throw std::runtime_error("cannot create device");

	vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
	vkGetDeviceQueue(m_device, indices.present_family.value(), 0, &m_present_queue);
}

void Application::run()
{
	init_window();
	init_vulkan();
	engine_loop();
	cleanup();
}

void Application::init_vulkan()
{
	create_instance();
	init_debug_messenger();
	create_surface();
	pick_physical_device();
	create_logical_device();
	create_swap_chain();
	create_image_views();
	create_render_pass();
	create_graphics_pipeline();
	create_framebuffers();
	create_command_pool();
	create_command_buffer();
	create_sync_objs();
}

void Application::create_render_pass()
{
	VkAttachmentDescription att_desc{};
	att_desc.format = m_swapchain_format;
	att_desc.samples = VK_SAMPLE_COUNT_1_BIT;
	att_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	att_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	att_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	att_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	att_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	att_desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment{};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_desc{};
		subpass_desc.colorAttachmentCount = 1;
		subpass_desc.pColorAttachments = &color_attachment;

	VkSubpassDependency dependency;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo create_info {};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.subpassCount = 1;
		create_info.pSubpasses = &subpass_desc;
		create_info.attachmentCount = 1;
		create_info.pAttachments = &att_desc;
	create_info.dependencyCount = 1;
	create_info.pDependencies = &dependency;

		if(vkCreateRenderPass(m_device, &create_info, nullptr, &m_render_pass) != VK_SUCCESS)
				throw std::runtime_error("canot create render pass");
}

void Application::create_surface()
{
	if (SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &m_surface) != true)
		throw std::runtime_error("Cannot create window surface");
}

void Application::cleanup()
{
	vkDestroyFence(m_device, m_in_flight, nullptr);
	vkDestroySemaphore(m_device, m_render_finished, nullptr);
	vkDestroySemaphore(m_device, m_image_availabale, nullptr);
	vkDestroyCommandPool(m_device, m_command_pool, nullptr);

	for(auto &framebuffer: m_framebuffers)
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);

	for (auto& view : m_swapchain_image_views)
		vkDestroyImageView(m_device, view, nullptr);

		vkDestroyPipeline(m_device, m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
		vkDestroyRenderPass(m_device, m_render_pass, nullptr);
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	vkDestroyDevice(m_device, nullptr);

	if( enable_validation_layers )
		destroy_debug_messenger(m_instance, m_debug_messenger, nullptr);
	
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void Application::pick_physical_device()
{
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

	if (device_count == 0)
		throw std::runtime_error("no vulkan support");

	std::vector<VkPhysicalDevice> available_devices (device_count);
	vkEnumeratePhysicalDevices(m_instance, &device_count, available_devices.data());

	for (const auto& device : available_devices)
	{
		if (is_device_suitable(device))
		{
			m_physical_device = device;
			break;
		}
	}

	if (m_physical_device == VK_NULL_HANDLE) 
		throw std::runtime_error("No suitable device available");
}

void Application::create_swap_chain()
{
	SwapChainSupportDetails swap_chain_support = find_chain_support_details(m_physical_device);

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
	VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);
	VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);

	uint32_t image_count = 1 + swap_chain_support.capabilities.minImageCount;
	// (swap_chain_support.capabilities.maxImageCount == 0) <=> pas de maximum
	if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount)
		image_count = swap_chain_support.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = m_surface;
	create_info.pNext = nullptr;

	create_info.minImageCount = image_count;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageFormat = surface_format.format;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	// ou alors VK_IMAGE_USAGE_TRANSFER_DST_BIT si on veut l'utiliser pour faire du post processing
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	auto indices = find_queue_family(m_physical_device);
	uint32_t queue_family_indices[2] = {indices.graphics_family.value(), indices.present_family.value()};

	if (queue_family_indices[0] != queue_family_indices[1])
	{
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	}
	else
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
	}

	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE; // sauf si on a quand même besoin de récup un pixel s'il est obstruit par une autre fenêtre

	create_info.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain) != VK_SUCCESS)
		throw std::runtime_error("Cannot create swapchain");

	// retrive swapchain images
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
	m_swapchain_images.resize(image_count);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_swapchain_images.data());

	m_swapchain_format = surface_format.format;
	m_swapchain_extent = extent;
}

void Application::create_graphics_pipeline()
{
	// SHADERS 
	auto vert_shader_code = get_file_content("../shaders/shader.vert.spv");
	auto frag_shader_code = get_file_content("../shaders/shader.frag.spv");

	auto vert_shader_mod = create_shader_module(vert_shader_code);
	auto frag_shader_mod = create_shader_module(frag_shader_code);

	// TODO: s'intéresser à shader_stage_info.pSpecializationInfo"
	VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_mod;
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_mod;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

	// FIXED FUNCTIONS
	std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dyn_state_create_info{};
	dyn_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dyn_state_create_info.dynamicStateCount = 2;
	dyn_state_create_info.pDynamicStates = dynamic_states.data();

	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = nullptr;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
	input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchain_extent.width;
	viewport.height = (float)m_swapchain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchain_extent;

	VkPipelineViewportStateCreateInfo viewport_state {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.scissorCount = 1;
	viewport_state.viewportCount = 1;
	viewport_state.pScissors = &scissor;
	viewport_state.pViewports = &viewport;

	VkPipelineRasterizationStateCreateInfo rasterizer_info {};
	rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer_info.depthClampEnable = VK_FALSE;
	rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
	rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer_info.lineWidth = 1.0f;
	rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer_info.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState color_blend_att {};
	color_blend_att.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_att.blendEnable = VK_FALSE;
	color_blend_att.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_att.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_att.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_att.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_att;
	for(int i = 0; i < 4; ++i)
		color_blending.blendConstants[i] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_info.setLayoutCount = 0;
	pipeline_info.pSetLayouts = nullptr;
	pipeline_info.pushConstantRangeCount = 0;
	pipeline_info.pPushConstantRanges = nullptr;

	if( vkCreatePipelineLayout(m_device, &pipeline_info, nullptr, &m_pipeline_layout) != VK_SUCCESS )
		throw std::runtime_error("cannot create pipeline");


		VkGraphicsPipelineCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		create_info.stageCount = 2;
		create_info.pStages = shader_stages;

		create_info.pColorBlendState = &color_blending;
		create_info.pViewportState = &viewport_state;
		create_info.pDynamicState = &dyn_state_create_info;
		create_info.pDepthStencilState = VK_NULL_HANDLE;
		create_info.pRasterizationState = &rasterizer_info;
		create_info.pVertexInputState = &vertex_input_info;
		create_info.pMultisampleState = &multisampling;
		create_info.pInputAssemblyState = &input_assembly_create_info;

		create_info.layout = m_pipeline_layout;
		create_info.renderPass = m_render_pass;
		create_info.subpass = 0;

		create_info.basePipelineHandle = VK_NULL_HANDLE;
		create_info.basePipelineIndex = -1;

		if(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &m_pipeline) != VK_SUCCESS)
				throw std::runtime_error("cannot create graphics pipeline");

	vkDestroyShaderModule(m_device, vert_shader_mod, nullptr);
	vkDestroyShaderModule(m_device, frag_shader_mod, nullptr);
}

void Application::engine_loop()
{
	bool should_close = false;
	while (!should_close)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_EVENT_QUIT)
			{
				should_close = true;
			}
		}
		
		draw_frame();
		std::this_thread::sleep_for(10ms);
	}

	// TODO: mauvaise pratique, peux amener à de très mauvaises perf, cf vulkan-tutorial:
	/*
		One thing i want to point out is that using queue wait idle on each frame is not the best thing to do. In fact the performance could be worst than GL.
		Best thing to do is use semaphores for each swapchain image rather than using single semaphore
	*/
	vkQueueWaitIdle(m_graphics_queue);
}

void Application::create_instance()
{
	if (enable_validation_layers && !check_validation_layers())
	{
		throw std::runtime_error("Validation layers not available");
	}

	static_assert(VK_MAKE_API_VERSION(0, 1, 0, 0) == VK_MAKE_VERSION(1, 0, 0));

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.apiVersion = VK_API_VERSION_1_0;
	app_info.pApplicationName = "Engine";
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	app_info.pNext = nullptr;

	VkInstanceCreateInfo create_info{};
	create_info.pApplicationInfo = &app_info;
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;


	auto required_ext = get_required_extensions();
	create_info.ppEnabledExtensionNames = required_ext.data();
	create_info.enabledExtensionCount = (uint32_t)required_ext.size();

	if (enable_validation_layers)
	{
		create_info.enabledLayerCount = (uint32_t)validation_layers.size();
		create_info.ppEnabledLayerNames = validation_layers.data();

		auto create_info = make_create_info();
		create_info.pNext = &create_info;
	}
	else
	{
		create_info.enabledLayerCount = 0;
	}

	VkResult res = vkCreateInstance(&create_info, nullptr, &m_instance);

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vulkan instance");
	}

}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debug_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_types,
	const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	void* user_data)
{
	if( message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
		std::cout << "[vk DEBUG]: " << callback_data->pMessage << "\nMessage types: " << message_types << " Severity: " << message_severity << "\n";

	return VK_FALSE;
}

void Application::init_debug_messenger()
{
	if (!enable_validation_layers) return;

	auto create_info = make_create_info();

	if (create_debug_utils_messenger(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
		throw std::runtime_error("cannot create degug messenger");
}

void Application::init_window()
{
	bool result = SDL_Init( SDL_INIT_VIDEO );
	result &= SDL_Vulkan_LoadLibrary(nullptr);

	if(!result)
	{
		throw std::runtime_error("Cannot initialize SDL");
	}

	m_window = SDL_CreateWindow(
		"Pierre",
		WIDTH, HEIGHT,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
	);

	if(m_window == nullptr)
	{
		throw std::runtime_error("Cannot create SDL Window");
	}
}

int main()
{
	try
	{
		Application app;
		app.run();
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}

	return 0;
}