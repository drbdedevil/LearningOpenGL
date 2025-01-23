#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>

#define USE_DEBUG_INFO true

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// ------------------------------

static class Utils
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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // создание окна:
        window = glfwCreateWindow(WIDTH, HEIGHT, "MaxVulkan", nullptr, nullptr);
    }
    void initVulkan()
    {
        createInstance();
    }
    void mainLoop()
    {
        // Пока не встретим исключение или не закроем окно, крутим цикл:
        while (!glfwWindowShouldClose(window))
        {
            // Обрабатывает события окна и ввода.
            glfwPollEvents();
        }
    }
    void cleanup()
    {
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void createInstance()
    {
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

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // Проверка поддержки расширения:
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

#ifdef USE_DEBUG_INFO 
    #if USE_DEBUG_INFO == true
        std::cout << "Available Extension Count - " << extensionCount << ":" << std::endl;
        for (const auto& extension : extensions)
        {
            std::cout << "\t" << extension.extensionName << std::endl;
        }
        Utils::checkExtensionSupport(glfwExtensions, glfwExtensionCount, extensions, "GLFW");
    #endif
#endif

        createInfo.enabledLayerCount = 0;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create instance!");
        }
    }

    // -------------------------------------
    GLFWwindow* window;
    VkInstance instance;
};

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