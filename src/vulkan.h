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
#define max_vulkan_frames_in_flight             4


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
    uint32_t            frames_in_flight_count_ ;

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

    uint32_t            texture_mip_levels_ ;
    VkImage             texture_image_ ;
    VkDeviceMemory      texture_image_memory_ ;
    VkImageView         texture_image_view_ ;
    VkSampler           texture_sampler_ ;

    float               desired_sampler_aniso_ ; //maxSamplerAnisotropy

    VkImage         depth_image_ ;
    VkDeviceMemory  depth_image_memory_ ;
    VkImageView     depth_image_view_ ;

    VkBool32                enable_sampling_ ;
    VkSampleCountFlagBits   sample_count_ ;
    VkBool32                enable_sample_shading_ ;
    float                   min_sample_shading_ ;

    VkImage         color_image_ ;
    VkDeviceMemory  color_image_memory_ ;
    VkImageView     color_image_view_ ;


} vulkan_context ;


int
create_vulkan() ;


void
destroy_vulkan() ;


int
draw_vulkan() ;

void
resize_vulkan() ;


void
add_desriptor_set_layout_binding(
    VkDescriptorSetLayoutBinding *  bindings
,   uint32_t *                      bindings_count
,   uint32_t const                  bindings_count_max
,   uint32_t const                  binding
,   VkDescriptorType const          descriptor_type
,   VkShaderStageFlags const        stage_flags
) ;


bool
create_descriptor_set_layout(
    VkDescriptorSetLayout *                 out_layout
,   VkDevice const                          device
,   VkDescriptorSetLayoutBinding const *    bindings
,   uint32_t const                          bindings_count
) ;


void
fill_pipeline_layout_create_info(
    VkPipelineLayoutCreateInfo *    plci
,   VkDescriptorSetLayout const *   descriptor_set_layouts
,   uint32_t const                  descriptor_set_layouts_count
) ;


void
add_descriptor_pool_size(
    VkDescriptorPoolSize *  pool_sizes
,   uint32_t *              pool_sizes_count
,   uint32_t const          pool_sizes_count_max
,   VkDescriptorType const  descriptor_type
,   uint32_t const          frames_in_flight_count
) ;


bool
create_descriptor_pool(
    VkDescriptorPool *              out_descriptor_pool
,   VkDevice const                  device
,   VkDescriptorPoolSize const *    pool_sizes
,   uint32_t const                  pool_sizes_count
,   uint32_t const                  frames_in_flight_count
) ;


bool
create_descriptor_sets(
    VkDescriptorSet *           out_descriptor_sets
,   VkDevice const              device
,   VkDescriptorSetLayout const descriptor_set_layout
,   VkDescriptorPool const      descriptor_pool
,   uint32_t const              frames_in_flight_count
) ;


bool
create_uniform_buffers(
    VkBuffer *                                  out_uniform_buffers
,   VkDeviceMemory *                            out_uniform_buffers_memory
,   void **                                     out_uniform_buffers_mapped
,   VkDevice const                              device
,   uint32_t const                              uniform_buffer_size
,   VkPhysicalDeviceMemoryProperties const *    pdmp
,   uint32_t const                              frames_in_flight_count
) ;


bool
create_texture_image(
    VkImage *                                   out_image
,   VkDeviceMemory *                            out_image_memory
,   char const * const                          full_name
,   VkDevice const                              device
,   VkCommandPool const                         command_pool
,   VkQueue const                               graphics_queue
,   VkPhysicalDeviceMemoryProperties const *    pdmp
,   uint32_t const                              mip_levels
) ;


bool
create_texture_image_view(
    VkImageView *       out_image_view
,   VkDevice const      device
,   VkImage const       image
,   uint32_t const      mip_levels
) ;


bool
create_texture_sampler(
    VkSampler *     out_sampler
,   VkDevice const  device
,   uint32_t const  mip_levels
,   VkBool32 const  enable_anisotropy
,   float const     max_anisotropy
) ;


void
add_descriptor_buffer_info(
    VkDescriptorBufferInfo *    descriptor_buffer_infos
,   uint32_t *                  descriptor_buffer_infos_count
,   uint32_t const              descriptor_buffer_infos_count_max
,   uint32_t const              frames_in_flight_count
,   VkBuffer const *            buffers
,   VkDeviceSize const          offset
,   VkDeviceSize const          range
) ;


void
add_descriptor_image_info(
    VkDescriptorImageInfo * descriptor_image_infos
,   uint32_t *              descriptor_image_infos_count
,   uint32_t const          descriptor_image_infos_count_max
,   uint32_t const          frames_in_flight_count
,   VkImageView const       image_view
,   VkSampler const         sampler
) ;


void
add_write_descriptor_buffer_set(
    VkWriteDescriptorSet *          write_descriptor_sets
,   uint32_t *                      write_descriptor_sets_count
,   uint32_t const                  write_descriptor_sets_count_max
,   VkDescriptorSet const *         descriptor_sets
,   uint32_t const                  descriptor_sets_count
,   VkDescriptorBufferInfo const *  descriptor_buffer_infos
,   uint32_t const                  descriptor_buffer_infos_count
,   uint32_t const                  descriptor_buffer_infos_count_max
,   uint32_t const                  frames_in_flight_count
,   uint32_t const                  descriptor_buffer_info_index
,   uint32_t const                  binding
,   VkDescriptorType const          descriptor_type
) ;


void
add_write_descriptor_image_set(
    VkWriteDescriptorSet *          write_descriptor_sets
,   uint32_t *                      write_descriptor_sets_count
,   uint32_t const                  write_descriptor_sets_count_max
,   VkDescriptorSet const *         descriptor_sets
,   uint32_t const                  descriptor_sets_count
,   VkDescriptorImageInfo const *   descriptor_image_infos
,   uint32_t const                  descriptor_image_infos_count
,   uint32_t const                  descriptor_image_infos_count_max
,   uint32_t const                  frames_in_flight_count
,   uint32_t const                  descriptor_image_info_index
,   uint32_t const                  binding
,   VkDescriptorType const          descriptor_type
) ;


void
update_descriptor_sets(
    VkWriteDescriptorSet *  write_descriptor_sets
,   uint32_t const          write_descriptor_sets_count
,   uint32_t const          write_descriptor_sets_count_max
,   VkDevice const          device
,   uint32_t const          frames_in_flight_count
) ;


bool
create_vertex_buffer(
    VkBuffer *                                  out_buffer
,   VkDeviceMemory *                            out_buffer_memory
,   VkDevice const                              device
,   VkCommandPool const                         command_pool
,   VkQueue const                               graphics_queue
,   VkPhysicalDeviceMemoryProperties const *    pdmp
,   void const *                                vbo_data
,   VkDeviceSize const                          vbo_data_size
) ;


bool
create_index_buffer(
    VkBuffer *                                  out_buffer
,   VkDeviceMemory *                            out_buffer_memory
,   VkDevice const                              device
,   VkCommandPool const                         command_pool
,   VkQueue const                               graphics_queue
,   VkPhysicalDeviceMemoryProperties const *    pdmp
,   void const *                                ibo_data
,   VkDeviceSize const                          ibo_data_size
) ;


bool
load_shader_file(
    VkShaderModule *    out_shader_module
,   VkDevice const      device
,   char const * const  shader_full_name
) ;


