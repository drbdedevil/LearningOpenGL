// #define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <array>
#include <set>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <limits> // Necessary for std::numeric_limits
#include <cstdint> // Necessary for uint32_t
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // нужно настроить матрицу перспективной проекции на диапазон от 0.0 до 1.0
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

#define USE_DEBUG_INFO true

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string PICTURE_MODEL_PATH = "D:/B_Projects/LearningOpenGL/Textures/texture.jpg";

const std::string VIK_ROOM_MODEL_PATH = "D:/B_Projects/LearningOpenGL/Models/viking_room.obj";
const std::string VIK_ROOM_TEXTURE_PATH = "D:/B_Projects/LearningOpenGL/Textures/viking_room.png";

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

uint32_t currentFrame = 0;
const int MAX_FRAMES_IN_FLIGHT = 2;

// ------------------------------

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
	std::optional<uint32_t> transferFamily;

	bool isComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilites;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{};

		// индекс привязки в массиве привязок (если несколько массивов для данных вершин)
		bindingDescription.binding = 0;
		// Указывает количество байтов от одной записи до другой
		bindingDescription.stride = sizeof(Vertex);
		// Переход к следующей записи данных после каждой вершины
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		// Из какой привязки поступают данные вершин (binding)
		attributeDescriptions[0].binding = 0;
		// Ссылается на директиву ввода в вершинной шейдере
		attributeDescriptions[0].location = 0;
		// Описывет тип данных атрибута. Говорим, что ввод в вершинном шейдере с местоположением 0 - это позиция, которая имеет два 32-битных компонента с плавающей точкой
		// Также неявно определяет размер байта атрибута
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		// Указывает количество байтов с начала данных по вершинам для чтения
		attributeDescriptions[0].offset = offsetof(Vertex, pos);
		// offsetof - возвращает смещение в байтах от начала структуры

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const 
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};
namespace std 
{
	template<> struct hash<Vertex> 
	{
		size_t operator()(Vertex const& vertex) const 
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

// типы необходимо выравнивать определённым образом, см. требования к выравниванию
struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class Utils
{
public:
	static bool checkExtensionSupport(const char** ExtensionsToCheck, uint32_t ExtensionCount, const std::vector<VkExtensionProperties>& AvailableExtenstions, const char* Object)
	{
		// Выводим количество необходимых расширений для объекта
		std::cout << "Required " << Object << " Extension Count - " << ExtensionCount << ":" << std::endl;
		// Выводим необходимые расширения для объекта
		for (uint32_t i = 0; i < ExtensionCount; ++i)
		{
			std::cout << "\t" << ExtensionsToCheck[i] << std::endl;
		}

		// Ищем их
		for (uint32_t i = 0; i < ExtensionCount; ++i)
		{
			bool bHasRequiredExtension = false;
			for (const auto& AvailableExtension : AvailableExtenstions)
			{
				if (std::strcmp(AvailableExtension.extensionName, ExtensionsToCheck[i]))
				{
					bHasRequiredExtension = true;
					break;
				}
			}

			if (!bHasRequiredExtension)
			{
				throw std::runtime_error("Program hasn't required extesions: " + std::string(ExtensionsToCheck[i]));
			}
		}

		std::cout << "All required extensions are available for " << Object << "!" << std::endl;

		return true;
	}
	static bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}
	static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices = findQueueFamilies(device, surface);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}
	static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

				if (presentSupport)
				{
					indices.presentFamily = i;
				}
			}
			if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				indices.transferFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			++i;
		}

		return indices;
	}
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> avaliableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, avaliableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : avaliableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}
	static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilites);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}
	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}
	static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
	static VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module!");
		}

		return shaderModule;
	}
	static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physicalDevice)
	{
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
			{
				return format;
			}
		}

		throw std::runtime_error("Failed to find supported format!");
	}
	static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice)
	{
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, physicalDevice);
	}
	static bool hasStencilComponent(VkFormat format) 
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
	static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice)
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	// Видеокарты предоставляют различные типы памяти для выделения.
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice)
	{
		// Получаем информацию о доступныых типах памяти
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
		// Структура VkPhysicalDeviceMemoryProperties содержит два массива:
		// 1) memoryTypes — список типов памяти.
		// 2) memoryHeaps — отдельные пуллы памяти, такие как выделенная видеопамять(VRAM) и область подкачки в оперативной памяти(RAM), используемая при нехватке VRAM.

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}


	// Читает все байты из указанного файла и возвращает их в массив байтов
	static std::vector<char> readFile(const std::string& filename)
	{
		// ate - начать читать с коцна файла, binary - прочитать как двоичный файл (чтобы избежать преобразование текста)
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file - " + filename + std::string("!"));
		}

		// Преимущества четения с конца - можно использовать позицию чтению для определения размера файла и выделения буфера:
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		// После этого возвращается к началу файла и читаем все байты сразу:
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		// Закрываем файл и возвращаем байты
		file.close();

		// Вывод размера файла:
		// std::cout << "File " << filename << " size: " << fileSize << std::endl;

		return buffer;
	}


	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
};

// ------------------------------

class Application
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void initWindow()
	{
		// инициализирует GLFW библиотеку:
		glfwInit();
		// Так как GLFW изначально была создана для контекста OpenGL, нам необходимо сказать, чтобы этот контекст не создавался следующим вызовом:
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		// создание окна:
		window = glfwCreateWindow(WIDTH, HEIGHT, "MaxVulkan", nullptr, nullptr);

		// Говорим о том, что в glfwSetFramebufferSizeCallback необходимо передать указатель на наше окно
		glfwSetWindowUserPointer(window, this);
		// Устанавливаем функция для обратного вызова при изменении размера окна
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		glfwSetScrollCallback(window, scroll_callback);
	}
	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		loadModel();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}
	void mainLoop()
	{
		// Пока не встретим исключение или не закроем окно, крутим цикл:
		while (!glfwWindowShouldClose(window))
		{
			// Обрабатывает события окна и ввода.
			glfwPollEvents();

			// Отрисовка
			drawFrame();
		}

		// Дожидаемся завершения всех операций логического устройства, прежде чем уничтожать его
		vkDeviceWaitIdle(device);
	}
	void cleanup()
	{
		cleanupSwapChain();

		vkDestroySampler(device, textureSampler, nullptr);
		vkDestroyImageView(device, textureImageView, nullptr);
		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}

		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);

		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyCommandPool(device, transferCommandPool, nullptr);

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers)
		{
			Utils::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void createInstance()
	{
		if (enableValidationLayers && !Utils::checkValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "MaxVulkan";
		appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		// Проверка поддержки расширения:
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		auto extensions = getRequiredExtensions(availableExtensions, extensionCount);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance!");
		}
	}

	void createSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	std::vector<const char*> getRequiredExtensions(const std::vector<VkExtensionProperties>& availableExtensions, uint32_t availableExtensionCount)
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

#ifdef USE_DEBUG_INFO 
#if USE_DEBUG_INFO == true
		std::cout << "Available Extension Count - " << availableExtensionCount << ":" << std::endl;
		for (const auto& extension : availableExtensions)
		{
			std::cout << "\t" << extension.extensionName << std::endl;
		}
		Utils::checkExtensionSupport(glfwExtensions, glfwExtensionCount, availableExtensions, "GLFW");
#endif
#endif

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}
	void setupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (Utils::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (Utils::isDeviceSuitable(device, surface))
			{
				physicalDevice = device;
				msaaSamples = Utils::getMaxUsableSampleCount(physicalDevice);
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}
	void createLogicalDevice()
	{
		QueueFamilyIndices indices = Utils::findQueueFamilies(physicalDevice, surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamiles = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

		// Даже если очередь одна, нужно обязательно указать приоритет (от 0 до 1)
		float queuePriority = 1.f;
		for (uint32_t queueFamily : uniqueQueueFamiles)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		// включает анизотропную фильтрацию - метод улучшения качества текстур при просмотре под углом.  Без неё текстуры выглядят размытыми или искажаются при наклоне поверхности.
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		// улучшает качество текстур
		deviceFeatures.sampleRateShading = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		// Сначала добавляем структуры, описанные выше, и только потом остальную часть:
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		// createInfo.enabledExtensionCount = 0;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// Включение расширений устройства:
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device))
		{
			throw std::runtime_error("Failed to create logical device!");
		}

		// 0 - индекс очереди
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
		vkGetDeviceQueue(device, indices.transferFamily.value(), 0, &transferQueue);
	}
	void createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = Utils::querySwapChainSupport(physicalDevice, surface);

		VkSurfaceFormatKHR surfaceFormat = Utils::chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = Utils::chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = Utils::chooseSwapExtent(swapChainSupport.capabilites, window);

		// Стоит указывать как минимум на одно изображение больше минимума, которые будут в swap chain
		uint32_t imageCount = swapChainSupport.capabilites.minImageCount + 1;
		if (swapChainSupport.capabilites.maxImageCount > 0 && imageCount > swapChainSupport.capabilites.maxImageCount)
		{
			imageCount = swapChainSupport.capabilites.maxImageCount;
		}

		// Структура для создания swap chain
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		// Только после указания поверхности:
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// Указываем, как обрабатывать изображения swap chain, которые будут использовать в нескольких семействах очередей.
		QueueFamilyIndices indices = Utils::findQueueFamilies(physicalDevice, surface);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			// изображения могут использоваться в нескольких семействах очередей без явной передачи права владения.
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			// изображение принадлежит только одному семейству очередей в данный момент, и право владения должно быть явно передано перед использованием изображения в дургом семействе очередей.
			// Этот режим обеспечивает наивышую производительность.
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}

		createInfo.preTransform = swapChainSupport.capabilites.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		// Эм.. Как это параша работает??
		/*if (oldSwapChain != VK_NULL_HANDLE)
		{
			createInfo.oldSwapchain = oldSwapChain;
		}
		else
		{
			createInfo.oldSwapchain = VK_NULL_HANDLE;
		}*/
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}
	void createImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); ++i)
		{
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}
	void createDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		// Указываем, на какой этап дескриптор собирается ссылаться
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		// Связано с выборокой изображения
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}
	}
	void createGraphicsPipeline()
	{
		// создание состояний шейдеров
		auto vertShaderCode = Utils::readFile("D:/B_Projects/LearningOpenGL/Shaders/art/vert.spv");
		auto fragShaderCode = Utils::readFile("D:/B_Projects/LearningOpenGL/Shaders/art/frag.spv");

		VkShaderModule vertShaderModule = Utils::createShaderModule(vertShaderCode, device);
		VkShaderModule fragShaderModule = Utils::createShaderModule(fragShaderCode, device);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// создание структур для vertex input. Этот этап отвечает за настройку вершинных данных (положение, цвет и т.д.)
		VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
		std::array<VkVertexInputAttributeDescription, 3> attributeDescription = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		// создание структуры, описывающей как рисовать примитивы
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Параметры viewport'а:
		VkViewport viewport{};
		viewport.x = 0.0f; // смещение по x (левая граница области вывода)
		viewport.y = 0.0f; // смещение по y (верхняя граница области вывода)
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f; // ближняя плоскость глубины
		viewport.maxDepth = 1.0f; // дальняя плоскость глубины

		// Параметры scissors'а
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		// создание динамических состояний
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// Растеризация
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		// Использование любого режима, кроме заполнения, требует включения функции графического процессора.
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		// любая линия толще 1.0f требует включения wideLines функции GPU
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading = 0.2f; // минимальная фракция для затенения образца; ближе к одному — гладко
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		// указывает, следует ли сравнивать глубину новых фрагментов с буфером глубины
		depthStencil.depthTestEnable = VK_TRUE;
		// следует ли записывать в буфер глубины новую глубину фрагментов, прошедших тест глубины
		depthStencil.depthWriteEnable = VK_TRUE;
		// определяет сравнение, которое выполняется для сохранения или удаления фрагментов
		// VK_COMPARE_OP_LESS - меньшая глубина = ближе, поэтому глубина новых фрагментов должна быть меньше
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		// Позволит записывать фрагменты только если они удовлетворяют данному диапазону
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		// Настривают операции трафарета
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// Создание pipeline'а
		// PipelineLayout определяет, какие дескрипторы будут использовать шейдеры
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		// если надо создать новый graphic pipeline, производный от существующего pipeline'а
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}
	void createRenderPass()
	{
		// создание описания цветового прикрепления (это буфер изображения, который используется в процессе рендеринга для хранения цветовых данных)
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = msaaSamples;

		// настройка к цветовым и глубинным данным
		// говорим, что буфер кадра необходимо очистить до чёрного перед отрисовкой нового кадра
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// сохраняем содержимое после ренедринга в памяти, чтобы его можно было прочитать позже
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		// настройка трафаретного буфера
		// нас не волнует, в каком макете изображение находилось ранее
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// мы не хотим, чтобы мультисэмплированное изображение было представлено напрямую (хотя оно и не может быть представлено)
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = Utils::findDepthFormat(physicalDevice);
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// Добавляем прикрепление для цвета с одним сэмплом - разрешающее вприкрепление:
		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		// является первым в массиве прикреплений
		colorAttachmentRef.attachment = 0;
		// будет использоваться как цветовой буфер
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// RenderPass'у нужно сказать о том, что необходимо resolve (разрешить) многосэмплированное изображение в обычное
		// Создаём ссылку на изображение, которое будет служить целью разрешения
		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // говорим, какие операции мы ждём 
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass!");
		}
	}
	void createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); ++i)
		{
			std::array<VkImageView, 3> attachments = { colorImageView, depthImageView, swapChainImageViews[i]  };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
	}
	void createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = Utils::findQueueFamilies(physicalDevice, surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// Разрешить перезапись command buffer по отдельности
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		// Мы будемзаписывать команды для рисования, поэтому выбрали семейство графических очередей
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}

		// Создание Command Pool для очереди передачи
		VkCommandPoolCreateInfo TransferPoolInfo{};
		TransferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// Разрешить перезапись command buffer по отдельности
		TransferPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		// Мы будемзаписывать команды для рисования, поэтому выбрали семейство графических очередей
		TransferPoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

		if (vkCreateCommandPool(device, &TransferPoolInfo, nullptr, &transferCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}
	}
	void createVertexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		// VK_BUFFER_USAGE_TRANSFER_SRC_BIT - буфер используется как источник трансферной операции
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// копируем данные вершин в буфер. vkMapMemory - позволяет получить доступ к области указанного ресурса памяти, определённой смещением и размером
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data); // делает память доступной CPU
			memcpy(data, vertices.data(), (size_t)bufferSize); // копирует содердимое одной области памяти в другую
		vkUnmapMemory(device, stagingBufferMemory); // освобождает виртуальное отображение памяти

		// VK_BUFFER_USAGE_TRANSFER_DST_BIT - буфер используется как цель достижения трансферной операции
		createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	void createIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		// VK_BUFFER_USAGE_TRANSFER_SRC_BIT - буфер используется как источник трансферной операции
		// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT и VK_MEMORY_PROPERTY_HOST_COHERENT_BIT - память выделена в оперативной амяти RAM и доступна как для CPU, так и для GPU
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		// копируем данные вершин в буфер. vkMapMemory - позволяет получить доступ к области указанного ресурса памяти, определённой смещением и размером
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize); // копирует содердимое одной области памяти в другую
		vkUnmapMemory(device, stagingBufferMemory);

		// VK_BUFFER_USAGE_TRANSFER_DST_BIT - буфер используется как цель достижения трансферной операции, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT - память хрнаится
		// в VRAM и доступна только GPU
		createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	void createUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}
	}
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		QueueFamilyIndices indices = Utils::findQueueFamilies(physicalDevice, surface);

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// Указываем размер буфера в байтах по массиву вершин
		bufferInfo.size = size;
		// Указывает для каких целей будет использовать буфер. Указываем, что будет использовать вершинный буфер и для трансфера
		bufferInfo.usage = usage;
		// буферы так же как и swap chain могут принадлежать определённому семейству очередей или быть общими для нескольких одновременно.
		// Наш буфер будет использовать только из графической очереди, поэтому мы можем придерживать эксклюзивного доступа.
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		// Используется для разрежённой памяти буфера.
		bufferInfo.flags = 0;

		// Говорим, какие очереди могут использовать буферы
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.transferFamily.value() };
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queueFamilyIndices;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create vertex buffer!");
		}

		// Буфер создан, но у него ещё нет выделенной памяти.
		// Первый шаг в выделении памяти для буфера - это запрос его требований к памяти с помощью функции:
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
		// Структура VkMemoryRequirements содержит три поля:
		// 1) size: Размер необходимой памяти в байтах, который может отличаться от bufferInfo.size.
		// 2) alignment: Смещение в байтах, с которого начинается буфер в выделенной области памяти. Оно зависит от bufferInfo.usage и bufferInfo.flags.
		// 3) memoryTypeBit: Битовое поле типов памяти, которые подходят для буфера.

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate vertex buffer memory!");
		}
		// При удачном выделении связываем буфер с памятью под буфер
		// - 4 параметр - смещение внутри области памяти. Поскольку эта память выделена специально для этого буфера вершин, смещение просто равно 0. 
		// Если смещение не равно нулю, то оно должно делиться на memRequirements.alignment.
		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		
		endSingleTimeCommands(commandBuffer);
	}
	void createCommandBuffers()
	{
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		// может быть отправлен в очередь на выполнение, но не может быть вызыван из других буферов команд
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	}
	void createDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool!");
		}
	}
	void createDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) 
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			// кнофигурация дескрипторов обновляется с помощью vkUpdateDescriptorSets функции, которая принимает массив VkWriteDescriptorSetструктур в качестве параметра
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			// говорим, что дескрипторы для записи
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			// привязываем конкретный дескрптор сет
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			// привязываем конкретный дескрптор сет
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = nullptr; // Optional
			descriptorWrites[1].pImageInfo = &imageInfo;
			descriptorWrites[1].pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}


	void recordCommandBuffer(VkCommandBuffer commandBufferIn, uint32_t imageIndex)
	{
		// Начинаем запись буфера комнад с VkBeginCommandBuffer:
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		// после вызова этой функции в буфер команд нельзя будет записывать команды, так как он неявно будет сброшен
		if (vkBeginCommandBuffer(commandBufferIn, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffes!");
		}

		// Рисование начинается с vkCmdBeginRenderPass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		// Таким образом мы создали буфер кадра для каждого изображения swap chain

		// определяем размер области рендеринга
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		// Определяют значения очистки
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 }; // начально значение в каждой точке буфера глубины должно быть максимально возможной глубиной, которая равна 1.0f

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		
		vkCmdBeginRenderPass(commandBufferIn, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Привязывание pipeline:
		vkCmdBindPipeline(commandBufferIn, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		// Привязаем буферы вершин к привязкам
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBufferIn, 0, 1, vertexBuffers, offsets);

		// биндим буфер индексов
		vkCmdBindIndexBuffer(commandBufferIn, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// Так viewport и scissor у нас динамические:
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBufferIn, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBufferIn, 0, 1, &scissor);

		// Связываем правильный набор дескрипторов для каждого кадра с дескрипторами в шейдере
		vkCmdBindDescriptorSets(commandBufferIn, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

		// 2 - указывает количество вершин, которые необходимо отисовать
		// 3 - используется для рендеринга экземпляра (устанавливаем 1, если мы этого не делаем)
		// 4 - смещение в буфере вершин, определяет наименьшее значение gl_VertexIndex
		// 5 - смещение для рендеринга экземпляров, определяет наименьшее значение gl_InstanceIndex.
		// vkCmdDraw(commandBufferIn, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

		vkCmdDrawIndexed(commandBufferIn, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		// завершение прохода рендеринга
		vkCmdEndRenderPass(commandBufferIn);

		// завершение записи буфера команд
		if (vkEndCommandBuffer(commandBufferIn) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
	void createSyncObjects()
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;  // Дополнительные настройки, обычно nullptr
		semaphoreInfo.flags = 0;        // Зарезервировано для будущего использования, всегда 0

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;  // Дополнительные настройки, обычно nullptr
		// Ставим первый кадр в сигнализированное положение, иначе мы не сможем понять, что рендеринг был закончен
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create semaphores or fence!");
			}
		}
	}


	void drawFrame()
	{
		// В начале фрейма нам нужно ждать, пока предыдущий фрейм завершиться
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		updateUniformBuffer(currentFrame);

		// Получаем картинку из swap chain
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		// Если swap chain стал несовместим с поверхностью и больше не может использоваться для рендеринга. Обычно происходит после изменения размера окна
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			oldSwapChain = swapChain;
			recreateSwapChain();
			return;
		} // swap chain по-прежнему можно использовать для представления на поверхности, но свойства поверхности больше не будут точно соответствовать
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		// Нам необходимо вручную сбросить сигнал с fence
		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		// Очищаем все раннее упомянутые команды, буфер переходит в состояние готов к записи
		// Второй параметр - при VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT освобождает связанный с буфером ресурс
		vkResetCommandBuffer(commandBuffers[currentFrame], 0);

		// Начинаем новую запись
		recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

		// Отправка и синхронизация очереди
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
		// Ждём стадию, когда цвет уже записан в буфер, но её не отображён на экране. Мы ждём, пока цветовой буфер станет доступным для записи.
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		// Какие семафоры ждём
		submitInfo.pWaitSemaphores = waitSemaphores;
		// На каком этапе ждём
		submitInfo.pWaitDstStageMask = waitStages;

		// Какие буферы команд фактически следует отправить на выполнение
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

		// Указываем, какие семафоры должны сигнализировать после завершения выполнения буферов команд
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		// Отправляем буфер команд в графическую очередь. Говорим, что CPU должно ждать сигнализирование inFlightFence.
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		// Отправка результата обратно в spaw chain, чтобы он появился на экране
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		// Указываем, какие семаформы следует ждать, прежде чем может произойти представление
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		// Определяем swap chain'ы для представления изображений и индекс изображения для каждой swap chain.
		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		// Позволяет указать массив VkResult значений для проверки каждой отдельной swap chain
		presentInfo.pResults = nullptr; // Optional

		// Отправляет запрос на представление изображения в swap chain
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) 
		{
			oldSwapChain = swapChain;
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to present swap chain image!");
		}

		// Переход к следующему кадру
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
	void updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		// существующее преобразование, угол поворота и ось вращения
		ubo.model = glm::rotate(glm::mat4(1.f), glm::radians(10.f * glm::sin(time)), glm::vec3(0.f, 0.f, 1.f));

		// позиция камеры, центральная точка и ось "вверх"
		ubo.view = glm::lookAt(glm::vec3(zoomValue, zoomValue, zoomValue) * glm::vec3(2.0f, 2.2f, 1.0f), glm::vec3(0.0f, 0.0f, 0.15f), glm::vec3(0.0f, 0.0f, 1.0f));

		// перспективная проекция с углом 45 градусов, соотношение сторон, ближняя и дальняя плоскости отсечения
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);

		// Так как у Vulkan Y инвертирован, надо его перевернуть, иначе картинка будет перевёрнутой
		ubo.proj[1][1] *= -1;

		// Копируем данные из объекта после преобразования в uniform buffer
		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	void createTextureImage()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(VIK_ROOM_TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		// пиксели располагаются по строкам с 4 байтами на пиксель, поэтому умножаем
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		// Вычисляем количество уровней в mip chain. max - выбирает наибольшее измерение, log2 - вычисляет, 
		// сколько раз его можно разделить на 2, 
		// floor - обрабатывает случаи, когда наибольшее измерение не является степенью 2, 
		// 1 - добавляется, чтобы исходное изображение имело уровень MIP.
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		if (!pixels)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);

		stbi_image_free(pixels);

		// VK_IMAGE_USAGE_TRANSFER_DST_BIT - Указываем, что копируем сюда данные и VK_IMAGE_USAGE_SAMPLED_BIT - что мы будем иметь доступ к изображению из шейдера.
		// VK_IMAGE_USAGE_TRANSFER_SRC_BIT - указываем, что будем использовать изображеие как источник
		createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		// Прежде чем скопировать буфер в изображение, нам необходимо поместить изображение в правильный макет. Для этого необходимо выполнить обработку переходов макета
		// Изображние было создано с макетом VK_IMAGE_LAYOUT_UNDEFINED, делаем его VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL - изображение предназначено как приёмник данных при копировании или загрузке
		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		// Переводим изображение в макет VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL - изображение используется только для чтения в шейдерах и не может быть модифицировано
		generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
		
		// transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}
	void createTextureImageView()
	{
		textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	}
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevelsIn, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		// По идее, пискели можно хранить в буфере, но лучше использовать VkImage. Они позволяют облегчить и ускорить получение цветов, позволяя использовать 2D-координаты, наприер.
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevelsIn;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		// Пиксели располагаются в порядке, определяем реализацией, для оптимального доступа
		imageInfo.tiling = tiling;
		// Не может использоваться графическим процессором, самый первый переход отбросит текселы
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples;
		imageInfo.flags = 0;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utils::findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory!");
		}
		vkBindImageMemory(device, image, imageMemory, 0);
	}
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevelsIn)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevelsIn;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to create texture image view!");
		}

		return imageView;
	}

	VkCommandBuffer beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}
	void endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	// Один из наиболее распространённых способов выполнения переходов между макетами изображений - использование барьера памяти для изображения.
	// Такой барьер pipeline обычно используется для синхронизации доступа к ресурсам, например, чтобы гарантировать завершение записи в буфер перед чтением из него.
	// Однако он также может быть использован для перехода между макетами изображений и передачи владения между семействами очередей, когда используется режим VK_SHARING_MODE_EXCLUSIVE. 
	// Существует эквивалентный барьер памяти для буферов, который выполняет аналогичные функции для буферов.
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevelsIn)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		// Можно использовать VK_IMAGE_LAYOUT_UNDEFINED, если нас не волнует существующий контент картинки
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		// Мы не используем барьер для передачи права собственности на семейство очередей, поэтому указываем VK_QUEUE_FAMILY_IGNORED
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevelsIn;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		// Барьеры в основном используются для синхронизации, поэтому необходимо указать, какие типы операций, включающие ресурс, должны выполняться до барьера, 
		// а какие операции, включающие ресурс, должны ожидать барьера.

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) // запись в изображение может ничего не ждать
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // самый ранний этап pipeline'а
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // это псевдо-этап в графичексом и вычеслительном pipeline'е, на котором происходят передачи
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) // чтение изображения должно ждать завершения записи при передачи
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // На этом этапе происходит чтение буфера глубины
			// VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT - на этом этапе происходит запись при отрисовке нового фрагмента
		}
		else 
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage /* TODO Этап pipeline'а, где происходят опреации, которые должны произойти до барьера */, destinationStage /* TODO здесб эти опреации ждут барьер */,
			0,
			0, nullptr, // Типы барьера
			0, nullptr,
			1, &barrier // Мы используем барьер памяти изображения
		);

		endSingleTimeCommands(commandBuffer);
	}
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// Указываем, какая часть буфера будет скопирована в какую часть изображения с помощью структуры VkBufferImageCopy
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // указывает, какой макет на данный момент должно использовать изображение
			1,
			&region
		);

		endSingleTimeCommands(commandBuffer);
	}
	void createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR; // Как интерполировать текселы, которые увеличены - касается проблемы избыточной выборки (когда текселей больше, чем фрагментов)
		samplerInfo.minFilter = VK_FILTER_LINEAR; // Как интерполировать текселы, которые уменьшены - касается проблемы недостаточной выборки (наоборот)
		
		// Указание режима адресации для каждой оси
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // повторить текстуру при выходе за пределы размеров изображения
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		// Включение анизотропной фильтрации
		samplerInfo.anisotropyEnable = VK_TRUE;
		// поле для ограничения количества образцов текселей, которые можно использовать для расчёта окончательного цвета
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		// определяет, какой цвет возвращается при выборке за пределыми изображения с режимом адресации clamp to border
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		// указывает, какую систему координат вы хотите использовать для адресации текселей в изображении. Если это поле равно VK_TRUE, 
		// то вы можете просто использовать координаты в пределах [0, texWidth)и [0, texHeight)диапазона. Если это поле равно VK_FALSE, 
		// то текселы адресуются с использованием [0, 1)диапазона по всем осям.
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = static_cast<float>(mipLevels);

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}
	void createDepthResources()
	{
		// Ищем формат глубины
		VkFormat depthFormat = Utils::findDepthFormat(physicalDevice);

		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	}
	void loadModel()
	{
		// контейнер содержит все позиции, нормали и координаты структуры в своих полях (attrib.vertices, attrib.normals и attrib.texcoords)
		tinyobj::attrib_t attrib;
		// содержит все отдельные объекты и их грани
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		// у LoadObj есть необязательный параметр для автоматической триангуляции граней, которые имеют больше трёх вершин. Включен по умолчанию.
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, VIK_ROOM_MODEL_PATH.c_str())) 
		{
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		// здесь мы объединяем все грани в файле в одну модель
		for (const auto& shape : shapes)
		{
			// функция триангуляции нам уже обеспечила наличие трёх вершин на каждой грани, поэтому теперь мы можем напрямую перебирать вершины и сьрасывать их прямо в наш вектор вершин
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				// по index мы ищем аттрибуты вершин
				vertex.pos = {
					// умножаем на 3, так как attrib.vertices это массив float, а нам нужен glm::vec3
					attrib.vertices[3 * index.vertex_index + 0], // смещения для доступа к X, Y и Z
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0], // смещения для доступа к компонентам U и V
					1.f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}
	// Mipmaps хранятся в структуре VkImage в поле mipLevels. VkImage для Vulkan - это массив текстурных уровней (mipmaps)
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mapLevels)
	{
		// Проверка, что image поддерживает linear blitting - это процесс масштабирования изображения с использованием линейной интерполяции.
		// Linear blitting позволяет плавно уменьшать mipmaps
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) 
		{
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; ++i) 
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 }; // Область, откуда копируем данные
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 }; // Область, куда копируем (её рамезры, как я понял)
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}
	void createColorResources()
	{
		VkFormat colorFormat = swapChainImageFormat;

		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
		colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}


	void cleanupSwapChain()
	{
		vkDestroyImageView(device, colorImageView, nullptr);
		vkDestroyImage(device, colorImage, nullptr);
		vkFreeMemory(device, colorImageMemory, nullptr);

		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		for (auto imageView : swapChainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}
	void recreateSwapChain()
	{
		// Мы остановим приложение, если размер окна с какой-либо стороны будет равен 0
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		// Вызываем эту функцию для того, чтобы не трогать ресурсы, которые ещё используются
		vkDeviceWaitIdle(device);

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createColorResources();
		createDepthResources();
		createFramebuffers();
	}
	

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	// -------------------------------------
	GLFWwindow* window;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // удаляется неявно при удалии VkInstance, поэтому можно не заниматься его удалением.
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkSwapchainKHR swapChain;
	VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	VkCommandPool transferCommandPool;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	std::vector<VkCommandBuffer> commandBuffers;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t mipLevels;
	VkImage textureImage;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkDeviceMemory textureImageMemory;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkImageView depthImageView;
	VkDeviceMemory depthImageMemory;

	VkDebugUtilsMessengerEXT debugMessenger;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	/*const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

		{{-0.5f, -0.5f, 0.2f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.2f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.2f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.2f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};
	const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};*/

public:
	// добавим дополнительный флаг, так как не гарантируется вызов VK_ERROR_OUT_OF_DATE_KHR при имзенении окна
	bool framebufferResized = false;
	float zoomValue = 1.0f;
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) 
{
	// std::cout << "Прокрутка: xoffset = " << xoffset << ", yoffset = " << yoffset << std::endl;
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));

	float zoomUpdate = static_cast<float>(yoffset / -10.0);

	app->zoomValue = glm::clamp(app->zoomValue + zoomUpdate, 0.2f, 20.f);
}

int main()
{
	setlocale(LC_ALL, "rus");

	Application app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}