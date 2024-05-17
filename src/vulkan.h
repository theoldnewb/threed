#pragma once


#include <vulkan/vulkan.h>


#define max_vulkan_desired_extensions           8
#define max_vulkan_desired_layers               8
#define max_vulkan_physical_devices             16
#define max_vulkan_queue_family_properties      16
#define max_vulkan_device_extensions            512
#define max_vulkan_unique_queue_family_indices  4
#define max_vulkan_desired_device_extensions    8
#define max_vulkan_surface_formats              8
#define max_vulkan_present_modes                 8



typedef struct vulkan_swapchain_support_details
{
    VkSurfaceCapabilitiesKHR    capabilities_ ;
    VkSurfaceFormatKHR          formats_[max_vulkan_surface_formats] ;
    uint32_t                    formats_count_ ;
    VkPresentModeKHR            modes_[max_vulkan_present_modes] ;
    uint32_t                    modes_count_ ;

} vulkan_swapchain_support_details ;


typedef struct vulkan_queue_family_indices
{
    uint32_t    graphics_family_ ;
    uint32_t    graphics_family_valid_ ;

    uint32_t    present_family_ ;
    uint32_t    present_family_valid_ ;

} vulkan_queue_family_indices ;


typedef struct vulkan_physical_device_info
{
    VkPhysicalDevice                    device_ ;
    VkPhysicalDeviceProperties          properties_ ;
    VkPhysicalDeviceFeatures            features_ ;
    VkPhysicalDeviceMemoryProperties    memory_properties_ ;

    VkQueueFamilyProperties     queue_family_properties_[max_vulkan_queue_family_properties] ;
    uint32_t                    queue_family_properties_count_ ;

    VkExtensionProperties       device_extensions_[max_vulkan_device_extensions] ;
    uint32_t                    device_extensions_count_ ;

    vulkan_queue_family_indices queue_families_indices_ ;
    VkBool32                    queue_families_indices_complete_ ;

    uint32_t                    unique_queue_families_indices_[max_vulkan_unique_queue_family_indices] ;
    uint32_t                    unique_queue_families_indices_count_ ;

    char const *                desired_device_extensions_[max_vulkan_desired_device_extensions] ;
    uint32_t                    desired_device_extensions_count_ ;
    VkBool32                    desired_device_extensions_okay_ ;

    VkSampleCountFlagBits       max_usable_sample_count_ ;
    VkSampleCountFlagBits       sample_count_ ;

    vulkan_swapchain_support_details    swapchain_support_details_ ;

} vulkan_physical_device_info ;


typedef struct vulkan_context
{
    uint32_t                platform_instance_extensions_count_ ;
    char const * const *    platform_instance_extensions_ ;

    VkInstance  instance_ ;

    uint32_t                instance_extensions_properties_count_ ;
    VkExtensionProperties * instance_extensions_properties_ ;

    char const *            desired_extensions_[max_vulkan_desired_extensions] ;
    uint32_t                desired_extensions_count_ ;

    VkBool32                enable_validation_ ;

    VkLayerProperties *     layer_properties_ ;
    uint32_t                layer_properties_count_ ;

    char const *            desired_layers_[max_vulkan_desired_layers] ;
    uint32_t                desired_layers_count_ ;

    VkDebugUtilsMessengerEXT            debug_messenger_ ;
    PFN_vkCreateDebugUtilsMessengerEXT  create_debug_messenger_func_ ;
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger_func_ ;

    uint32_t                    physical_devices_count_ ;
    VkPhysicalDevice            physical_devices_[max_vulkan_physical_devices] ;
    vulkan_physical_device_info physical_devices_info_[max_vulkan_physical_devices] ;


    VkSurfaceKHR    surface_ ;


} vulkan_context ;


int
create_vulkan() ;


void
destroy_vulkan() ;

