#pragma once


#include <vulkan/vulkan.h>


#define max_vulkan_desired_extensions   8
#define max_vulkan_desired_layers       8


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

} vulkan_context ;


int
create_vulkan() ;


void
destroy_vulkan() ;

