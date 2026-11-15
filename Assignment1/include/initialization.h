#ifndef INITIALIZATION
#define INITIALIZATION
#include <vulkan/vulkan.hpp>
#include <cstring>

// compability purposes


struct AppResources
{
    vk::Instance instance;
    vk::DebugUtilsMessengerEXT dbgUtilsMgr;
    vk::PhysicalDevice pDevice;

    vk::Device device;
    uint32_t cQ;
    vk::Queue computeQueue;
    vk::CommandPool computeCommandPool;
    vk::QueryPool queryPool;
    // We don't create transfer queue for sakes of integrated GPUs compatibility 

    void destroy();
};

VKAPI_ATTR VkBool32 VKAPI_CALL
debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                            VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
                            void * /*pUserData*/);
vk::DebugUtilsMessengerCreateInfoEXT makeDebugUtilsMessengerCreateInfoEXT();

void selectPhysicalDevice(vk::Instance &instance, vk::PhysicalDevice &pDevice);
void createInstance(vk::Instance &instance, vk::DebugUtilsMessengerEXT &debugUtilsMessenger,
                    std::string appName, std::string engineName);
void createLogicalDevice(vk::Instance &instance, vk::PhysicalDevice &pDevice, vk::Device &device);
std::tuple<uint32_t, uint32_t> getComputeAndcomputeQueues(vk::PhysicalDevice &pDevice);
void createCommandPool(vk::Device &device, vk::CommandPool &commandPool, uint32_t queueIndex);
void destroyInstance(vk::Instance &instance, vk::DebugUtilsMessengerEXT &debugUtilsMessenger);
void destroyLogicalDevice(vk::Device &device);
void destroyCommandPool(vk::Device &device, vk::CommandPool &commandPool);

void createTimestampQueryPool(vk::Device &device, vk::QueryPool &queryPool, uint32_t queryCount);
void destroyQueryPool(vk::Device &device, vk::QueryPool &queryPool);


void printDeviceCapabilities(vk::PhysicalDevice &pDevice);

void initApp(AppResources &app);
#endif
