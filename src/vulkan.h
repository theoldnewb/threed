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
#define max_vulkan_present_modes                8
#define max_vulkan_desired_format_properties    8
#define max_vulkan_swapchain_images             4
#define max_vulkan_frames_in_flight             2


typedef struct vulkan_desired_format_properties
{
    VkFormat            format_ ;
    VkFormatProperties  properties_ ;

} vulkan_desired_format_properties ;


typedef struct vulkan_swapchain_support_details
{
    VkPhysicalDevice            physical_device_ ;
    VkSurfaceKHR                surface_ ;
    VkSurfaceCapabilitiesKHR    capabilities_ ;
    VkFormatProperties          formats_properties_[max_vulkan_surface_formats] ;
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
    VkBool32                            swapchain_support_details_okay_ ;
    vulkan_desired_format_properties    desired_format_properties_[max_vulkan_desired_format_properties] ;
    uint32_t                            desired_format_properties_count_ ;

    VkFormat    depth_format_ ;

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

    vulkan_physical_device_info *   picked_physical_device_ ;

    VkDevice device_ ;
    VkQueue graphics_queue_ ;
    VkQueue present_queue_ ;

    VkSwapchainKHR      swapchain_ ;
    VkSurfaceFormatKHR  swapchain_surface_format_ ;
    VkPresentModeKHR    swapchain_present_mode_ ;
    VkExtent2D          swapchain_extent_ ;
    uint32_t            swapchain_images_count_ ;
    VkImage             swapchain_images_[max_vulkan_swapchain_images] ;
    VkImageView         swapchain_views_[max_vulkan_swapchain_images] ;
    uint32_t            desired_swapchain_image_count_ ;


    void *              vert_shader_memory_ ;
    uint64_t            vert_shader_memory_size_ ;
    VkShaderModule      vert_shader_ ;
    void *              frag_shader_memory_ ;
    uint64_t            frag_shader_memory_size_ ;
    VkShaderModule      frag_shader_ ;

    VkRenderPass            render_pass_ ;
    VkDescriptorSetLayout   descriptor_set_layout_ ;
    VkPipelineLayout        pipeline_layout_ ;
    VkPipeline              graphics_pipeline_ ;

    VkFramebuffer   framebuffers_[max_vulkan_swapchain_images] ;
    VkCommandPool   command_pool_ ;
    VkCommandBuffer command_buffer_[max_vulkan_frames_in_flight] ;

    VkSemaphore image_available_semaphore_[max_vulkan_frames_in_flight] ;
    VkSemaphore render_finished_semaphore_[max_vulkan_frames_in_flight] ;
    VkFence     in_flight_fence_[max_vulkan_frames_in_flight] ;
    uint32_t    current_frame_ ;
    VkBool32    resizing_ ;

    VkBuffer        vertex_buffer_ ;
    VkDeviceMemory  vertex_buffer_memory_ ;
    VkBuffer        index_buffer_ ;
    VkDeviceMemory  index_buffer_memory_ ;

    VkBuffer        uniform_buffers_[max_vulkan_frames_in_flight] ;
    VkDeviceMemory  uniform_buffers_memory_[max_vulkan_frames_in_flight] ;
    void *          uniform_buffers_mapped_[max_vulkan_frames_in_flight] ;

    VkDescriptorPool    descriptor_pool_ ;
    VkDescriptorSet     descriptor_sets_[max_vulkan_frames_in_flight] ;

    VkImage             texture_image_ ;
    VkDeviceMemory      texture_image_memory_ ;
    VkImageView         texture_image_view_ ;
    VkSampler           texture_sampler_ ;

    float               desired_sampler_aniso_ ; //maxSamplerAnisotropy

    VkImage         depth_image_ ;
    VkDeviceMemory  depth_image_memory_ ;
    VkImageView     depth_image_view_ ;


} vulkan_context ;


int
create_vulkan() ;


void
destroy_vulkan() ;


int
draw_vulkan() ;

void
resize_vulkan() ;
