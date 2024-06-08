#include "app.h"
#include "vulkan.h"
#include "types.h"
#include "check.h"
#include "log.h"
#include "debug.h"
#include "math.h"
#include "vulkan_rob.h"

#include <SDL3/SDL_vulkan.h>
#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/affine.h>
#include <cglm/cam.h>

#include <stb/stb_image.h>


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static vulkan_context   the_vulkan_context_ = { 0 } ;
static vulkan_context * vc_ = &the_vulkan_context_ ;


static char const vk_layer_khronos_validation_name[]            = "VK_LAYER_KHRONOS_validation" ;
static char const vk_ext_debug_utils_extension_name[]           = VK_EXT_DEBUG_UTILS_EXTENSION_NAME ;
static char const vk_create_debug_utils_messenger_ext_name[]    = "vkCreateDebugUtilsMessengerEXT" ;
static char const vk_destroy_debug_utils_messenger_ext_name[]   = "vkDestroyDebugUtilsMessengerEXT" ;
static char const vk_khr_swapchain_extension_name[]             = VK_KHR_SWAPCHAIN_EXTENSION_NAME ;





typedef struct vertex {
    vec3 pos ;
    vec3 color ;
    vec2 uv ;
} vertex ;

static uint32_t const vertex_size = sizeof(vertex) ;

    // {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    // {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    // {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    // {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}

static vertex const vertices[] =
{
    { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }
,   { { 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }
,   { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} }
,   { {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }
,   { {-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }
,   { { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }
,   { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} }
,   { {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }

} ;
static uint32_t const vertices_size = sizeof(vertices) ;
static uint32_t const vertices_count = array_count(vertices) ;


static uint16_t const indices[] =
{
    0, 1, 2
,   2, 3, 0
,   4, 5, 6
,   6, 7, 4
} ;
static uint32_t const   indices_size = sizeof(indices) ;
static uint32_t const   indices_count = array_count(indices) ;


typedef struct uniform_buffer_object
{
    mat4 model ;
    mat4 view ;
    mat4 proj ;
} uniform_buffer_object ;

static uint32_t const uniform_buffer_object_size = sizeof(uniform_buffer_object) ;



////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static bool
create_image(
    VkImage *                                   out_image
,   VkDeviceMemory *                            out_image_memory
,   VkDevice const                              device
,   VkPhysicalDeviceMemoryProperties const *    pdmp
,   uint32_t const                              width
,   uint32_t const                              height
,   uint32_t const                              mip_levels
,   VkSampleCountFlagBits const                 msaa_sample_count
,   VkFormat const                              format
,   VkImageTiling const                         tiling
,   VkImageUsageFlags const                     usage
,   VkMemoryPropertyFlags const                 mem_prop_flags
) ;


static VkVertexInputBindingDescription
get_binding_description()
{
    // typedef struct VkVertexInputBindingDescription {
    //     uint32_t             binding;
    //     uint32_t             stride;
    //     VkVertexInputRate    inputRate;
    // } VkVertexInputBindingDescription;
    static VkVertexInputBindingDescription binding_description = { 0 } ;
    binding_description.binding     = 0 ;
    binding_description.stride      = vertex_size ;
    binding_description.inputRate   = VK_VERTEX_INPUT_RATE_VERTEX ;

    return binding_description ;
}


static void
get_attribute_descriptions(
    VkVertexInputAttributeDescription * viad
,   uint32_t const                      viad_count
)
{
    require(viad) ;
    require(viad_count == 3) ;
    // typedef struct VkVertexInputAttributeDescription {
    //     uint32_t    location;
    //     uint32_t    binding;
    //     VkFormat    format;
    //     uint32_t    offset;
    // } VkVertexInputAttributeDescription;
    viad[0].location    = 0 ;
    viad[0].binding     = 0 ;
    viad[0].format      = VK_FORMAT_R32G32B32_SFLOAT ;
    viad[0].offset      = offsetof(vertex, pos) ;

    viad[1].location    = 1 ;
    viad[1].binding     = 0 ;
    viad[1].format      = VK_FORMAT_R32G32B32_SFLOAT ;
    viad[1].offset      = offsetof(vertex, color) ;

    viad[2].location    = 2 ;
    viad[2].binding     = 0 ;
    viad[2].format      = VK_FORMAT_R32G32_SFLOAT ;
    viad[2].offset      = offsetof(vertex, uv) ;
}



static VkFormat const desired_formats[] =
{
    VK_FORMAT_B8G8R8A8_UNORM
,   VK_FORMAT_B8G8R8A8_SRGB
,   VK_FORMAT_D32_SFLOAT
,   VK_FORMAT_D32_SFLOAT_S8_UINT
,   VK_FORMAT_D24_UNORM_S8_UINT
} ;
static uint32_t const desired_formats_count = array_count(desired_formats) ;
static_require(array_count(desired_formats) < max_vulkan_desired_format_properties, "fix me!") ;



#define max_dump_buffer 4096



static void
update_uniform_buffer(
    vulkan_context *    vc
,   uint32_t const      current_frame
)
{
    require(vc) ;
    require(current_frame < max_vulkan_frames_in_flight) ;
    require(current_frame < vc->frames_in_flight_count_) ;

    static bool once = true ;
    static uint64_t previous_time = 0 ;

    uint64_t const current_time = get_app_time() ;

    if(once)
    {
        once = false ;
        previous_time = current_time ;
    }

    uint64_t const delta_time = (current_time - previous_time) / 10 ;
    double const fractional_seconds = (double) delta_time * get_performance_frequency_inverse() ;
    //log_debug("%f", fractional_seconds) ;

    uniform_buffer_object ubo = { 0 } ;

    // glm_mat4_identity(ubo.model) ;
    // glm_mat4_identity(ubo.view) ;
    // glm_mat4_identity(ubo.proj) ;

    float angle = fractional_seconds ;
    vec3 axis = {1.0f, 0.5f, 1.0f} ;
    glm_rotate_make(ubo.model, angle, axis) ;

    vec3 eye    = { 0.0f, 1.0f, 1.0f } ;
    vec3 center = { 0.0f, 0.0f, 0.0f } ;
    vec3 up     = { 0.0f, 0.0f, 1.0f } ;
    glm_lookat(eye, center, up, ubo.view) ;

    float fovy = glm_rad(45.0f) ;
    float aspect_ratio = (float)vc->swapchain_extent_.width / (float)vc->swapchain_extent_.height ;
    float near = 0.1f ;
    float far = 10.f ;
    glm_perspective(fovy, aspect_ratio, near, far, ubo.proj) ;

    SDL_memcpy(vc->uniform_buffers_mapped_[current_frame], &ubo, sizeof(ubo)) ;
}




////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT          messageSeverity
,   VkDebugUtilsMessageTypeFlagsEXT                 messageType
,   VkDebugUtilsMessengerCallbackDataEXT const *    pCallbackData
,   void *                                          pUserData
)
{
    UNUSED(messageSeverity) ;
    UNUSED(messageType) ;
    UNUSED(pUserData) ;

    //std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    if(pCallbackData)
    {
        if(pCallbackData->pMessage)
        {
            log_debug_str(pCallbackData->pMessage) ;
        }
    }
    return VK_FALSE;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static void
dump_char_star_array(
    char const * const * const  s
,   uint32_t const              n
)
{
    for(uint32_t i = 0 ; i < n ; ++i)
    {
        log_debug_u32(i) ;
        require(s) ;
        log_debug_str(s[i]) ;
    }
}


static void
dump_extension_property(
    VkExtensionProperties const *   extprop
)
{
    require(extprop) ;

    // typedef struct VkExtensionProperties {
    //     char        extensionName[VK_MAX_EXTENSION_NAME_SIZE];
    //     uint32_t    specVersion;
    // } VkExtensionProperties;
    log_debug_str(extprop->extensionName) ;
    log_debug_u32(extprop->specVersion) ;
}


static void
dump_extension_properties(
    VkExtensionProperties const *   extprop
,   uint32_t const                  extprop_count
)
{
    log_debug_u32(extprop_count) ;
    for(
        uint32_t i = 0
    ;   i < extprop_count
    ;   ++i
    )
    {

        log_debug_u32(i) ;
        require(extprop) ;
        dump_extension_property(&extprop[i]) ;
    }
}


static char const *
physical_device_type_to_string(
    VkPhysicalDeviceType const physical_device_type
)
{
    // typedef enum VkPhysicalDeviceType {
    //     VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
    //     VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
    //     VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
    //     VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
    //     VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
    // } VkPhysicalDeviceType;
    switch(physical_device_type)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:             return "VK_PHYSICAL_DEVICE_TYPE_OTHER" ;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:    return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU" ;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:      return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU" ;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:       return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU" ;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:               return "VK_PHYSICAL_DEVICE_TYPE_CPU" ;
    default:                                        return "Unknown Device Type" ;
    }
}

// static char const *
// sample_count_flag_bits_to_string(
//     VkSampleCountFlagBits const bits
// )
// {
//     // typedef enum VkSampleCountFlagBits {
//     //     VK_SAMPLE_COUNT_1_BIT = 0x00000001,
//     //     VK_SAMPLE_COUNT_2_BIT = 0x00000002,
//     //     VK_SAMPLE_COUNT_4_BIT = 0x00000004,
//     //     VK_SAMPLE_COUNT_8_BIT = 0x00000008,
//     //     VK_SAMPLE_COUNT_16_BIT = 0x00000010,
//     //     VK_SAMPLE_COUNT_32_BIT = 0x00000020,
//     //     VK_SAMPLE_COUNT_64_BIT = 0x00000040,
//     // } VkSampleCountFlagBits;
//     switch(bits)
//     {
//     case VK_SAMPLE_COUNT_1_BIT:     return "VK_SAMPLE_COUNT_1_BIT" ;
//     case VK_SAMPLE_COUNT_2_BIT:     return "VK_SAMPLE_COUNT_2_BIT" ;
//     case VK_SAMPLE_COUNT_4_BIT:     return "VK_SAMPLE_COUNT_4_BIT" ;
//     case VK_SAMPLE_COUNT_8_BIT:     return "VK_SAMPLE_COUNT_8_BIT" ;
//     case VK_SAMPLE_COUNT_16_BIT:    return "VK_SAMPLE_COUNT_16_BIT" ;
//     case VK_SAMPLE_COUNT_32_BIT:    return "VK_SAMPLE_COUNT_32_BIT" ;
//     case VK_SAMPLE_COUNT_64_BIT:    return "VK_SAMPLE_COUNT_64_BIT" ;
//     default:                        return "Unknown Sample Count" ;
//     }
// }


static void
dump_physical_device_limits(
    VkPhysicalDeviceLimits const *  physical_device_limits
)
{
    require(physical_device_limits) ;
    // typedef struct VkPhysicalDeviceLimits {
    //     uint32_t              maxImageDimension1D;
    //     uint32_t              maxImageDimension2D;
    //     uint32_t              maxImageDimension3D;
    //     uint32_t              maxImageDimensionCube;
    //     uint32_t              maxImageArrayLayers;
    //     uint32_t              maxTexelBufferElements;
    //     uint32_t              maxUniformBufferRange;
    //     uint32_t              maxStorageBufferRange;
    //     uint32_t              maxPushConstantsSize;
    //     uint32_t              maxMemoryAllocationCount;
    //     uint32_t              maxSamplerAllocationCount;
    //     VkDeviceSize          bufferImageGranularity;
    //     VkDeviceSize          sparseAddressSpaceSize;
    //     uint32_t              maxBoundDescriptorSets;
    //     uint32_t              maxPerStageDescriptorSamplers;
    //     uint32_t              maxPerStageDescriptorUniformBuffers;
    //     uint32_t              maxPerStageDescriptorStorageBuffers;
    //     uint32_t              maxPerStageDescriptorSampledImages;
    //     uint32_t              maxPerStageDescriptorStorageImages;
    //     uint32_t              maxPerStageDescriptorInputAttachments;
    //     uint32_t              maxPerStageResources;
    //     uint32_t              maxDescriptorSetSamplers;
    //     uint32_t              maxDescriptorSetUniformBuffers;
    //     uint32_t              maxDescriptorSetUniformBuffersDynamic;
    //     uint32_t              maxDescriptorSetStorageBuffers;
    //     uint32_t              maxDescriptorSetStorageBuffersDynamic;
    //     uint32_t              maxDescriptorSetSampledImages;
    //     uint32_t              maxDescriptorSetStorageImages;
    //     uint32_t              maxDescriptorSetInputAttachments;
    //     uint32_t              maxVertexInputAttributes;
    //     uint32_t              maxVertexInputBindings;
    //     uint32_t              maxVertexInputAttributeOffset;
    //     uint32_t              maxVertexInputBindingStride;
    //     uint32_t              maxVertexOutputComponents;
    //     uint32_t              maxTessellationGenerationLevel;
    //     uint32_t              maxTessellationPatchSize;
    //     uint32_t              maxTessellationControlPerVertexInputComponents;
    //     uint32_t              maxTessellationControlPerVertexOutputComponents;
    //     uint32_t              maxTessellationControlPerPatchOutputComponents;
    //     uint32_t              maxTessellationControlTotalOutputComponents;
    //     uint32_t              maxTessellationEvaluationInputComponents;
    //     uint32_t              maxTessellationEvaluationOutputComponents;
    //     uint32_t              maxGeometryShaderInvocations;
    //     uint32_t              maxGeometryInputComponents;
    //     uint32_t              maxGeometryOutputComponents;
    //     uint32_t              maxGeometryOutputVertices;
    //     uint32_t              maxGeometryTotalOutputComponents;
    //     uint32_t              maxFragmentInputComponents;
    //     uint32_t              maxFragmentOutputAttachments;
    //     uint32_t              maxFragmentDualSrcAttachments;
    //     uint32_t              maxFragmentCombinedOutputResources;
    //     uint32_t              maxComputeSharedMemorySize;
    //     uint32_t              maxComputeWorkGroupCount[3];
    //     uint32_t              maxComputeWorkGroupInvocations;
    //     uint32_t              maxComputeWorkGroupSize[3];
    //     uint32_t              subPixelPrecisionBits;
    //     uint32_t              subTexelPrecisionBits;
    //     uint32_t              mipmapPrecisionBits;
    //     uint32_t              maxDrawIndexedIndexValue;
    //     uint32_t              maxDrawIndirectCount;
    //     float                 maxSamplerLodBias;
    //     float                 maxSamplerAnisotropy;
    //     uint32_t              maxViewports;
    //     uint32_t              maxViewportDimensions[2];
    //     float                 viewportBoundsRange[2];
    //     uint32_t              viewportSubPixelBits;
    //     size_t                minMemoryMapAlignment;
    //     VkDeviceSize          minTexelBufferOffsetAlignment;
    //     VkDeviceSize          minUniformBufferOffsetAlignment;
    //     VkDeviceSize          minStorageBufferOffsetAlignment;
    //     int32_t               minTexelOffset;
    //     uint32_t              maxTexelOffset;
    //     int32_t               minTexelGatherOffset;
    //     uint32_t              maxTexelGatherOffset;
    //     float                 minInterpolationOffset;
    //     float                 maxInterpolationOffset;
    //     uint32_t              subPixelInterpolationOffsetBits;
    //     uint32_t              maxFramebufferWidth;
    //     uint32_t              maxFramebufferHeight;
    //     uint32_t              maxFramebufferLayers;
    //     VkSampleCountFlags    framebufferColorSampleCounts;
    //     VkSampleCountFlags    framebufferDepthSampleCounts;
    //     VkSampleCountFlags    framebufferStencilSampleCounts;
    //     VkSampleCountFlags    framebufferNoAttachmentsSampleCounts;
    //     uint32_t              maxColorAttachments;
    //     VkSampleCountFlags    sampledImageColorSampleCounts;
    //     VkSampleCountFlags    sampledImageIntegerSampleCounts;
    //     VkSampleCountFlags    sampledImageDepthSampleCounts;
    //     VkSampleCountFlags    sampledImageStencilSampleCounts;
    //     VkSampleCountFlags    storageImageSampleCounts;
    //     uint32_t              maxSampleMaskWords;
    //     VkBool32              timestampComputeAndGraphics;
    //     float                 timestampPeriod;
    //     uint32_t              maxClipDistances;
    //     uint32_t              maxCullDistances;
    //     uint32_t              maxCombinedClipAndCullDistances;
    //     uint32_t              discreteQueuePriorities;
    //     float                 pointSizeRange[2];
    //     float                 lineWidthRange[2];
    //     float                 pointSizeGranularity;
    //     float                 lineWidthGranularity;
    //     VkBool32              strictLines;
    //     VkBool32              standardSampleLocations;
    //     VkDeviceSize          optimalBufferCopyOffsetAlignment;
    //     VkDeviceSize          optimalBufferCopyRowPitchAlignment;
    //     VkDeviceSize          nonCoherentAtomSize;
    // } VkPhysicalDeviceLimits;

    log_debug_u32(physical_device_limits->maxImageDimension1D) ;
    log_debug_u32(physical_device_limits->maxImageDimension1D) ;
    log_debug_u32(physical_device_limits->maxImageDimension2D) ;
    log_debug_u32(physical_device_limits->maxImageDimension2D) ;
    log_debug_u32(physical_device_limits->maxImageDimension3D) ;
    log_debug_u32(physical_device_limits->maxImageDimension3D) ;
    log_debug_u32(physical_device_limits->maxImageDimensionCube) ;
    log_debug_u32(physical_device_limits->maxImageDimensionCube) ;
    log_debug_u32(physical_device_limits->maxImageArrayLayers) ;
    log_debug_u32(physical_device_limits->maxImageArrayLayers) ;
    log_debug_u32(physical_device_limits->maxTexelBufferElements) ;
    log_debug_u32(physical_device_limits->maxTexelBufferElements) ;
    log_debug_u32(physical_device_limits->maxUniformBufferRange) ;
    log_debug_u32(physical_device_limits->maxUniformBufferRange) ;
    log_debug_u32(physical_device_limits->maxStorageBufferRange) ;
    log_debug_u32(physical_device_limits->maxStorageBufferRange) ;
    log_debug_u32(physical_device_limits->maxPushConstantsSize) ;
    log_debug_u32(physical_device_limits->maxPushConstantsSize) ;
    log_debug_u32(physical_device_limits->maxMemoryAllocationCount) ;
    log_debug_u32(physical_device_limits->maxMemoryAllocationCount) ;
    log_debug_u32(physical_device_limits->maxSamplerAllocationCount) ;
    log_debug_u32(physical_device_limits->maxSamplerAllocationCount) ;

    log_debug_u64(physical_device_limits->bufferImageGranularity) ;
    log_debug_u64(physical_device_limits->bufferImageGranularity) ;
    log_debug_u64(physical_device_limits->sparseAddressSpaceSize) ;
    log_debug_u64(physical_device_limits->sparseAddressSpaceSize) ;

    log_debug_u32(physical_device_limits->maxBoundDescriptorSets) ;
    log_debug_u32(physical_device_limits->maxBoundDescriptorSets) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorSamplers) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorSamplers) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorUniformBuffers) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorUniformBuffers) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorStorageBuffers) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorStorageBuffers) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorSampledImages) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorSampledImages) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorStorageImages) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorStorageImages) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorInputAttachments) ;
    log_debug_u32(physical_device_limits->maxPerStageDescriptorInputAttachments) ;
    log_debug_u32(physical_device_limits->maxPerStageResources) ;
    log_debug_u32(physical_device_limits->maxPerStageResources) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetSamplers) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetSamplers) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetUniformBuffers) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetUniformBuffers) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetUniformBuffersDynamic) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetUniformBuffersDynamic) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetStorageBuffers) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetStorageBuffers) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetStorageBuffersDynamic) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetStorageBuffersDynamic) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetSampledImages) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetSampledImages) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetStorageImages) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetStorageImages) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetInputAttachments) ;
    log_debug_u32(physical_device_limits->maxDescriptorSetInputAttachments) ;
    log_debug_u32(physical_device_limits->maxVertexInputAttributes) ;
    log_debug_u32(physical_device_limits->maxVertexInputAttributes) ;
    log_debug_u32(physical_device_limits->maxVertexInputBindings) ;
    log_debug_u32(physical_device_limits->maxVertexInputAttributeOffset) ;
    log_debug_u32(physical_device_limits->maxVertexInputBindingStride) ;
    log_debug_u32(physical_device_limits->maxVertexOutputComponents) ;
    log_debug_u32(physical_device_limits->maxTessellationGenerationLevel) ;
    log_debug_u32(physical_device_limits->maxTessellationPatchSize) ;
    log_debug_u32(physical_device_limits->maxTessellationControlPerVertexInputComponents) ;
    log_debug_u32(physical_device_limits->maxTessellationControlPerVertexOutputComponents) ;
    log_debug_u32(physical_device_limits->maxTessellationControlPerPatchOutputComponents) ;
    log_debug_u32(physical_device_limits->maxTessellationControlTotalOutputComponents) ;
    log_debug_u32(physical_device_limits->maxTessellationEvaluationInputComponents) ;
    log_debug_u32(physical_device_limits->maxTessellationEvaluationOutputComponents) ;
    log_debug_u32(physical_device_limits->maxGeometryShaderInvocations) ;
    log_debug_u32(physical_device_limits->maxGeometryInputComponents) ;
    log_debug_u32(physical_device_limits->maxGeometryOutputComponents) ;
    log_debug_u32(physical_device_limits->maxGeometryOutputVertices) ;
    log_debug_u32(physical_device_limits->maxGeometryTotalOutputComponents) ;
    log_debug_u32(physical_device_limits->maxFragmentInputComponents) ;
    log_debug_u32(physical_device_limits->maxFragmentOutputAttachments) ;
    log_debug_u32(physical_device_limits->maxFragmentDualSrcAttachments) ;
    log_debug_u32(physical_device_limits->maxFragmentCombinedOutputResources) ;
    log_debug_u32(physical_device_limits->maxComputeSharedMemorySize) ;
    log_debug_u32(physical_device_limits->maxComputeWorkGroupCount[0]) ;
    log_debug_u32(physical_device_limits->maxComputeWorkGroupCount[1]) ;
    log_debug_u32(physical_device_limits->maxComputeWorkGroupCount[2]) ;
    log_debug_u32(physical_device_limits->maxComputeWorkGroupInvocations) ;
    log_debug_u32(physical_device_limits->maxComputeWorkGroupSize[0]) ;
    log_debug_u32(physical_device_limits->maxComputeWorkGroupSize[1]) ;
    log_debug_u32(physical_device_limits->maxComputeWorkGroupSize[2]) ;
    log_debug_u32(physical_device_limits->subPixelPrecisionBits) ;
    log_debug_u32(physical_device_limits->subTexelPrecisionBits) ;
    log_debug_u32(physical_device_limits->mipmapPrecisionBits) ;
    log_debug_u32(physical_device_limits->maxDrawIndexedIndexValue) ;
    log_debug_u32(physical_device_limits->maxDrawIndirectCount) ;

    log_debug_f32(physical_device_limits->maxSamplerLodBias) ;
    log_debug_f32(physical_device_limits->maxSamplerAnisotropy) ;

    log_debug_u32(physical_device_limits->maxViewports) ;
    log_debug_u32(physical_device_limits->maxViewportDimensions[0]) ;
    log_debug_u32(physical_device_limits->maxViewportDimensions[1]) ;

    log_debug_f32(physical_device_limits->viewportBoundsRange[0]) ;
    log_debug_f32(physical_device_limits->viewportBoundsRange[1]) ;


    log_debug_u32(physical_device_limits->viewportSubPixelBits) ;
    log_debug_u64(physical_device_limits->minMemoryMapAlignment) ;
    log_debug_u64(physical_device_limits->minTexelBufferOffsetAlignment) ;
    log_debug_u64(physical_device_limits->minUniformBufferOffsetAlignment) ;
    log_debug_u64(physical_device_limits->minStorageBufferOffsetAlignment) ;

    log_debug_s32(physical_device_limits->minTexelOffset) ;

    log_debug_u32(physical_device_limits->maxTexelOffset) ;

    log_debug_s32(physical_device_limits->minTexelGatherOffset) ;

    log_debug_u32(physical_device_limits->maxTexelGatherOffset) ;
    log_debug_f32(physical_device_limits->minInterpolationOffset) ;
    log_debug_f32(physical_device_limits->maxInterpolationOffset) ;
    log_debug_u32(physical_device_limits->subPixelInterpolationOffsetBits) ;
    log_debug_u32(physical_device_limits->maxFramebufferWidth) ;
    log_debug_u32(physical_device_limits->maxFramebufferHeight) ;
    log_debug_u32(physical_device_limits->maxFramebufferLayers) ;

    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->framebufferColorSampleCounts)) ;
    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->framebufferDepthSampleCounts)) ;
    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->framebufferStencilSampleCounts)) ;
    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->framebufferNoAttachmentsSampleCounts)) ;

    log_debug_u32(physical_device_limits->framebufferColorSampleCounts) ;
    log_debug_u32(physical_device_limits->framebufferDepthSampleCounts) ;
    log_debug_u32(physical_device_limits->framebufferStencilSampleCounts) ;
    log_debug_u32(physical_device_limits->framebufferNoAttachmentsSampleCounts) ;

    log_debug_u32(physical_device_limits->maxColorAttachments) ;

    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->sampledImageColorSampleCounts)) ;
    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->sampledImageIntegerSampleCounts)) ;
    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->sampledImageDepthSampleCounts)) ;
    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->sampledImageStencilSampleCounts)) ;
    // log_debug_str(sample_count_flag_bits_to_string(physical_device_limits->storageImageSampleCounts)) ;

    log_debug_u32(physical_device_limits->sampledImageColorSampleCounts) ;
    log_debug_u32(physical_device_limits->sampledImageIntegerSampleCounts) ;
    log_debug_u32(physical_device_limits->sampledImageDepthSampleCounts) ;
    log_debug_u32(physical_device_limits->sampledImageStencilSampleCounts) ;
    log_debug_u32(physical_device_limits->storageImageSampleCounts) ;

    log_debug_u32(physical_device_limits->maxSampleMaskWords) ;
    log_debug_u32(physical_device_limits->timestampComputeAndGraphics) ;

    log_debug_f32(physical_device_limits->timestampPeriod) ;

    log_debug_u32(physical_device_limits->maxClipDistances) ;
    log_debug_u32(physical_device_limits->maxCullDistances) ;
    log_debug_u32(physical_device_limits->maxCombinedClipAndCullDistances) ;
    log_debug_u32(physical_device_limits->discreteQueuePriorities) ;

    log_debug_f32(physical_device_limits->pointSizeRange[0]) ;
    log_debug_f32(physical_device_limits->pointSizeRange[1]) ;
    log_debug_f32(physical_device_limits->lineWidthRange[0]) ;
    log_debug_f32(physical_device_limits->lineWidthRange[1]) ;
    log_debug_f32(physical_device_limits->pointSizeGranularity) ;
    log_debug_f32(physical_device_limits->lineWidthGranularity) ;

    log_debug_u32(physical_device_limits->strictLines) ;
    log_debug_u32(physical_device_limits->standardSampleLocations) ;

    log_debug_u64(physical_device_limits->optimalBufferCopyOffsetAlignment) ;
    log_debug_u64(physical_device_limits->optimalBufferCopyRowPitchAlignment) ;
    log_debug_u64(physical_device_limits->nonCoherentAtomSize) ;

}


static void
dump_physical_device_sparse_properties(
    VkPhysicalDeviceSparseProperties const * physical_device_sparse_properties
)
{
    require(physical_device_sparse_properties) ;
    // typedef struct VkPhysicalDeviceSparseProperties {
    //     VkBool32    residencyStandard2DBlockShape;
    //     VkBool32    residencyStandard2DMultisampleBlockShape;
    //     VkBool32    residencyStandard3DBlockShape;
    //     VkBool32    residencyAlignedMipSize;
    //     VkBool32    residencyNonResidentStrict;
    // } VkPhysicalDeviceSparseProperties;
    log_debug_u32(physical_device_sparse_properties->residencyStandard2DBlockShape) ;
    log_debug_u32(physical_device_sparse_properties->residencyStandard2DMultisampleBlockShape) ;
    log_debug_u32(physical_device_sparse_properties->residencyStandard3DBlockShape) ;
    log_debug_u32(physical_device_sparse_properties->residencyAlignedMipSize) ;
    log_debug_u32(physical_device_sparse_properties->residencyNonResidentStrict) ;
}


static void
dump_physical_device_properties(
    VkPhysicalDeviceProperties const *  physical_device_properties
)
{
    require(physical_device_properties) ;
    log_debug_ptr(physical_device_properties) ;

    // typedef struct VkPhysicalDeviceProperties {
    //     uint32_t                            apiVersion;
    //     uint32_t                            driverVersion;
    //     uint32_t                            vendorID;
    //     uint32_t                            deviceID;
    //     VkPhysicalDeviceType                deviceType;
    //     char                                deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
    //     uint8_t                             pipelineCacheUUID[VK_UUID_SIZE];
    //     VkPhysicalDeviceLimits              limits;
    //     VkPhysicalDeviceSparseProperties    sparseProperties;
    // } VkPhysicalDeviceProperties;
    log_debug_u32(physical_device_properties->apiVersion) ;
    log_debug_u32(physical_device_properties->driverVersion) ;
    log_debug_u32(physical_device_properties->vendorID) ;
    log_debug_u32(physical_device_properties->deviceID) ;
    log_debug_str(physical_device_type_to_string(physical_device_properties->deviceType)) ;
    log_debug_str(physical_device_properties->deviceName) ;
    dump_physical_device_limits(&physical_device_properties->limits) ;
    dump_physical_device_sparse_properties(&physical_device_properties->sparseProperties) ;
}


static void
dump_physical_device_features(
    VkPhysicalDeviceFeatures const * physical_device_features
)
{
    require(physical_device_features) ;

    // typedef struct VkPhysicalDeviceFeatures {
    //     VkBool32    robustBufferAccess;
    //     VkBool32    fullDrawIndexUint32;
    //     VkBool32    imageCubeArray;
    //     VkBool32    independentBlend;
    //     VkBool32    geometryShader;
    //     VkBool32    tessellationShader;
    //     VkBool32    sampleRateShading;
    //     VkBool32    dualSrcBlend;
    //     VkBool32    logicOp;
    //     VkBool32    multiDrawIndirect;
    //     VkBool32    drawIndirectFirstInstance;
    //     VkBool32    depthClamp;
    //     VkBool32    depthBiasClamp;
    //     VkBool32    fillModeNonSolid;
    //     VkBool32    depthBounds;
    //     VkBool32    wideLines;
    //     VkBool32    largePoints;
    //     VkBool32    alphaToOne;
    //     VkBool32    multiViewport;
    //     VkBool32    samplerAnisotropy;
    //     VkBool32    textureCompressionETC2;
    //     VkBool32    textureCompressionASTC_LDR;
    //     VkBool32    textureCompressionBC;
    //     VkBool32    occlusionQueryPrecise;
    //     VkBool32    pipelineStatisticsQuery;
    //     VkBool32    vertexPipelineStoresAndAtomics;
    //     VkBool32    fragmentStoresAndAtomics;
    //     VkBool32    shaderTessellationAndGeometryPointSize;
    //     VkBool32    shaderImageGatherExtended;
    //     VkBool32    shaderStorageImageExtendedFormats;
    //     VkBool32    shaderStorageImageMultisample;
    //     VkBool32    shaderStorageImageReadWithoutFormat;
    //     VkBool32    shaderStorageImageWriteWithoutFormat;
    //     VkBool32    shaderUniformBufferArrayDynamicIndexing;
    //     VkBool32    shaderSampledImageArrayDynamicIndexing;
    //     VkBool32    shaderStorageBufferArrayDynamicIndexing;
    //     VkBool32    shaderStorageImageArrayDynamicIndexing;
    //     VkBool32    shaderClipDistance;
    //     VkBool32    shaderCullDistance;
    //     VkBool32    shaderFloat64;
    //     VkBool32    shaderInt64;
    //     VkBool32    shaderInt16;
    //     VkBool32    shaderResourceResidency;
    //     VkBool32    shaderResourceMinLod;
    //     VkBool32    sparseBinding;
    //     VkBool32    sparseResidencyBuffer;
    //     VkBool32    sparseResidencyImage2D;
    //     VkBool32    sparseResidencyImage3D;
    //     VkBool32    sparseResidency2Samples;
    //     VkBool32    sparseResidency4Samples;
    //     VkBool32    sparseResidency8Samples;
    //     VkBool32    sparseResidency16Samples;
    //     VkBool32    sparseResidencyAliased;
    //     VkBool32    variableMultisampleRate;
    //     VkBool32    inheritedQueries;
    // } VkPhysicalDeviceFeatures;
    log_debug_u32(physical_device_features->robustBufferAccess) ;
    log_debug_u32(physical_device_features->fullDrawIndexUint32) ;
    log_debug_u32(physical_device_features->imageCubeArray) ;
    log_debug_u32(physical_device_features->independentBlend) ;
    log_debug_u32(physical_device_features->geometryShader) ;
    log_debug_u32(physical_device_features->tessellationShader) ;
    log_debug_u32(physical_device_features->sampleRateShading) ;
    log_debug_u32(physical_device_features->dualSrcBlend) ;
    log_debug_u32(physical_device_features->logicOp) ;
    log_debug_u32(physical_device_features->multiDrawIndirect) ;
    log_debug_u32(physical_device_features->drawIndirectFirstInstance) ;
    log_debug_u32(physical_device_features->depthClamp) ;
    log_debug_u32(physical_device_features->depthBiasClamp) ;
    log_debug_u32(physical_device_features->fillModeNonSolid) ;
    log_debug_u32(physical_device_features->depthBounds) ;
    log_debug_u32(physical_device_features->wideLines) ;
    log_debug_u32(physical_device_features->largePoints) ;
    log_debug_u32(physical_device_features->alphaToOne) ;
    log_debug_u32(physical_device_features->multiViewport) ;
    log_debug_u32(physical_device_features->samplerAnisotropy) ;
    log_debug_u32(physical_device_features->textureCompressionETC2) ;
    log_debug_u32(physical_device_features->textureCompressionASTC_LDR) ;
    log_debug_u32(physical_device_features->textureCompressionBC) ;
    log_debug_u32(physical_device_features->occlusionQueryPrecise) ;
    log_debug_u32(physical_device_features->pipelineStatisticsQuery) ;
    log_debug_u32(physical_device_features->vertexPipelineStoresAndAtomics) ;
    log_debug_u32(physical_device_features->fragmentStoresAndAtomics) ;
    log_debug_u32(physical_device_features->shaderTessellationAndGeometryPointSize) ;
    log_debug_u32(physical_device_features->shaderImageGatherExtended) ;
    log_debug_u32(physical_device_features->shaderStorageImageExtendedFormats) ;
    log_debug_u32(physical_device_features->shaderStorageImageMultisample) ;
    log_debug_u32(physical_device_features->shaderStorageImageReadWithoutFormat) ;
    log_debug_u32(physical_device_features->shaderStorageImageWriteWithoutFormat) ;
    log_debug_u32(physical_device_features->shaderUniformBufferArrayDynamicIndexing) ;
    log_debug_u32(physical_device_features->shaderSampledImageArrayDynamicIndexing) ;
    log_debug_u32(physical_device_features->shaderStorageBufferArrayDynamicIndexing) ;
    log_debug_u32(physical_device_features->shaderStorageImageArrayDynamicIndexing) ;
    log_debug_u32(physical_device_features->shaderClipDistance) ;
    log_debug_u32(physical_device_features->shaderCullDistance) ;
    log_debug_u32(physical_device_features->shaderFloat64) ;
    log_debug_u32(physical_device_features->shaderInt64) ;
    log_debug_u32(physical_device_features->shaderInt16) ;
    log_debug_u32(physical_device_features->shaderResourceResidency) ;
    log_debug_u32(physical_device_features->shaderResourceMinLod) ;
    log_debug_u32(physical_device_features->sparseBinding) ;
    log_debug_u32(physical_device_features->sparseResidencyBuffer) ;
    log_debug_u32(physical_device_features->sparseResidencyImage2D) ;
    log_debug_u32(physical_device_features->sparseResidencyImage3D) ;
    log_debug_u32(physical_device_features->sparseResidency2Samples) ;
    log_debug_u32(physical_device_features->sparseResidency4Samples) ;
    log_debug_u32(physical_device_features->sparseResidency8Samples) ;
    log_debug_u32(physical_device_features->sparseResidency16Samples) ;
    log_debug_u32(physical_device_features->sparseResidencyAliased) ;
    log_debug_u32(physical_device_features->variableMultisampleRate) ;
    log_debug_u32(physical_device_features->inheritedQueries) ;
}


static char const *
dump_queue_flag_bit(
    VkQueueFlagBits const queue_flag_bit
)
{
    // typedef enum VkQueueFlagBits {
    //     VK_QUEUE_GRAPHICS_BIT = 0x00000001,
    //     VK_QUEUE_COMPUTE_BIT = 0x00000002,
    //     VK_QUEUE_TRANSFER_BIT = 0x00000004,
    //     VK_QUEUE_SPARSE_BINDING_BIT = 0x00000008,
    //   // Provided by VK_VERSION_1_1
    //     VK_QUEUE_PROTECTED_BIT = 0x00000010,
    //   // Provided by VK_KHR_video_decode_queue
    //     VK_QUEUE_VIDEO_DECODE_BIT_KHR = 0x00000020,
    //   // Provided by VK_KHR_video_encode_queue
    //     VK_QUEUE_VIDEO_ENCODE_BIT_KHR = 0x00000040,
    //   // Provided by VK_NV_optical_flow
    //     VK_QUEUE_OPTICAL_FLOW_BIT_NV = 0x00000100,
    // } VkQueueFlagBits;
    switch(queue_flag_bit)
    {
    case VK_QUEUE_GRAPHICS_BIT:         return "VK_QUEUE_GRAPHICS_BIT" ;
    case VK_QUEUE_COMPUTE_BIT:          return "VK_QUEUE_COMPUTE_BIT" ;
    case VK_QUEUE_TRANSFER_BIT:         return "VK_QUEUE_TRANSFER_BIT" ;
    case VK_QUEUE_SPARSE_BINDING_BIT:   return "VK_QUEUE_SPARSE_BINDING_BIT" ;
    case VK_QUEUE_PROTECTED_BIT:        return "VK_QUEUE_PROTECTED_BIT" ;
    case VK_QUEUE_VIDEO_DECODE_BIT_KHR: return "VK_QUEUE_VIDEO_DECODE_BIT_KHR" ;
    case VK_QUEUE_VIDEO_ENCODE_BIT_KHR: return "VK_QUEUE_VIDEO_ENCODE_BIT_KHR" ;
    case VK_QUEUE_OPTICAL_FLOW_BIT_NV:  return "VK_QUEUE_OPTICAL_FLOW_BIT_NV" ;
    default:                            return "Unknown Queue Flag Bit" ;
    }
}


static char const *
dump_queue_flag_bits(
    VkQueueFlagBits const queue_flag_bits
)
{
    VkQueueFlagBits const all_queue_flag_bits[] =
    {
        VK_QUEUE_GRAPHICS_BIT
    ,   VK_QUEUE_COMPUTE_BIT
    ,   VK_QUEUE_TRANSFER_BIT
    ,   VK_QUEUE_SPARSE_BINDING_BIT
    ,   VK_QUEUE_PROTECTED_BIT
    ,   VK_QUEUE_VIDEO_DECODE_BIT_KHR
    ,   VK_QUEUE_VIDEO_ENCODE_BIT_KHR
    ,   VK_QUEUE_OPTICAL_FLOW_BIT_NV
    } ;
    static uint32_t const all_queue_flag_bits_count = array_count(all_queue_flag_bits) ;

    static char dump_buffer[max_dump_buffer] = { 0 } ;
    SDL_memset(dump_buffer, 0, max_dump_buffer) ;
    size_t n = 0 ;
    for(
        uint32_t i = 0
    ;   i < all_queue_flag_bits_count
    ;   ++i
    )
    {
        if(queue_flag_bits & all_queue_flag_bits[i])
        {
            n = SDL_strlcat(dump_buffer, dump_queue_flag_bit(all_queue_flag_bits[i]), max_dump_buffer) ;
            require(n < max_dump_buffer) ;
            n = SDL_strlcat(dump_buffer, " | ", max_dump_buffer) ;
            require(n < max_dump_buffer) ;
        }
    }
    return dump_buffer ;
}



static void
dump_queue_family_property(
    VkQueueFamilyProperties const * queue_family_properties
)
{
    require(queue_family_properties) ;
    // typedef struct VkQueueFamilyProperties {
    //     VkQueueFlags    queueFlags;
    //     uint32_t        queueCount;
    //     uint32_t        timestampValidBits;
    //     VkExtent3D      minImageTransferGranularity;
    // } VkQueueFamilyProperties;
    // typedef struct VkExtent3D {
    //     uint32_t    width;
    //     uint32_t    height;
    //     uint32_t    depth;
    // } VkExtent3D;

    log_debug_str(dump_queue_flag_bits(queue_family_properties->queueFlags)) ;
    log_debug_u32(queue_family_properties->queueCount) ;
    log_debug_u32(queue_family_properties->timestampValidBits) ;
    log_debug_u32(queue_family_properties->minImageTransferGranularity.width) ;
    log_debug_u32(queue_family_properties->minImageTransferGranularity.height) ;
    log_debug_u32(queue_family_properties->minImageTransferGranularity.depth) ;
}


static void
dump_queue_family_properties(
    VkQueueFamilyProperties const * queue_family_properties
,   uint32_t const                  queue_family_properties_count
)
{
    for(
        uint32_t i = 0
    ;   i < queue_family_properties_count
    ;   ++i
    )
    {
        require(queue_family_properties) ;
        dump_queue_family_property(&queue_family_properties[i]) ;
    }
}


static char const *
dump_memory_property_flags_bit(
    VkMemoryPropertyFlagBits const flag
)
{
    switch(flag)
    {
    case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:           return "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" ;
    case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:           return "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" ;
    case VK_MEMORY_PROPERTY_HOST_COHERENT_BIT:          return "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" ;
    case VK_MEMORY_PROPERTY_HOST_CACHED_BIT:            return "VK_MEMORY_PROPERTY_HOST_CACHED_BIT" ;
    case VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT:       return "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" ;
    case VK_MEMORY_PROPERTY_PROTECTED_BIT:              return "VK_MEMORY_PROPERTY_PROTECTED_BIT" ;
    case VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD:    return "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD" ;
    case VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD:    return "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD" ;
    case VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV:        return "VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV" ;
    default:                                            return "Unknown Memory Property Flag Bit" ;
    }
}


static char const *
dump_memory_property_flags_bits(
    VkMemoryPropertyFlagBits const memory_property_flag_bits
)
{
    VkMemoryPropertyFlagBits const all_memory_property_flag_bits[] =
    {
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    ,   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    ,   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    ,   VK_MEMORY_PROPERTY_HOST_CACHED_BIT
    ,   VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT
    ,   VK_MEMORY_PROPERTY_PROTECTED_BIT
    ,   VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD
    ,   VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD
    ,   VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV
    } ;
    uint32_t const all_memory_property_flag_bits_count = array_count(all_memory_property_flag_bits) ;

    static char dump_buffer[max_dump_buffer] = { 0 } ;
    SDL_memset(dump_buffer, 0, max_dump_buffer) ;
    size_t n = 0 ;
    for(
        uint32_t i = 0
    ;   i < all_memory_property_flag_bits_count
    ;   ++i
    )
    {
        if(memory_property_flag_bits & all_memory_property_flag_bits[i])
        {
            n = SDL_strlcat(dump_buffer, dump_memory_property_flags_bit(all_memory_property_flag_bits[i]), max_dump_buffer) ;
            require(n < max_dump_buffer) ;
            n = SDL_strlcat(dump_buffer, " | ", max_dump_buffer) ;
            require(n < max_dump_buffer) ;
        }
    }
    return dump_buffer ;
}


static void
dump_memory_type(
    VkMemoryType const * memory_type
)
{
    require(memory_type) ;
    log_debug_u32(memory_type->propertyFlags) ;
    log_debug_str(dump_memory_property_flags_bits(memory_type->propertyFlags)) ;
    log_debug_u32(memory_type->heapIndex) ;
}


static char const *
dump_memory_heap_flag_bit(
    VkMemoryHeapFlagBits const memory_heap_flag_bit
)
{
    switch(memory_heap_flag_bit)
    {
    case VK_MEMORY_HEAP_DEVICE_LOCAL_BIT:           return "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT" ;
    case VK_MEMORY_HEAP_MULTI_INSTANCE_BIT:         return "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT" ;
    //case VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR:   return "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR" ;
    default:                                        return "Unknown Memory Heap Flag Bit" ;
    }
}


static char const *
dump_memory_heap_flags_bits(
    VkMemoryHeapFlagBits const memory_heap_flag_bits
)
{
    VkMemoryHeapFlagBits const all_memory_heap_flag_bits[] =
    {
        VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
    ,   VK_MEMORY_HEAP_MULTI_INSTANCE_BIT
    } ;
    uint32_t const all_memory_heap_flag_bits_count = array_count(all_memory_heap_flag_bits) ;

    static char dump_buffer[max_dump_buffer] = { 0 } ;
    SDL_memset(dump_buffer, 0, max_dump_buffer) ;
    size_t n = 0 ;
    for(
        uint32_t i = 0
    ;   i < all_memory_heap_flag_bits_count
    ;   ++i
    )
    {
        if(memory_heap_flag_bits & all_memory_heap_flag_bits[i])
        {
            n = SDL_strlcat(dump_buffer, dump_memory_heap_flag_bit(all_memory_heap_flag_bits[i]), max_dump_buffer) ;
            require(n < max_dump_buffer) ;
            n = SDL_strlcat(dump_buffer, " | ", max_dump_buffer) ;
            require(n < max_dump_buffer) ;
        }
    }
    return dump_buffer ;
}


static void
dump_memory_heap(
    VkMemoryHeap const * memory_heap
)
{
    require(memory_heap) ;
    log_debug_u64(memory_heap->size) ;
    log_debug_u32(memory_heap->flags) ;
    log_debug_str(dump_memory_heap_flags_bits(memory_heap->flags)) ;
}


static void
dump_physical_device_memory_properties(
    VkPhysicalDeviceMemoryProperties const * physical_device_memory_properties
)
{
    require(physical_device_memory_properties) ;

    // typedef struct VkPhysicalDeviceMemoryProperties {
    //     uint32_t        memoryTypeCount;
    //     VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
    //     uint32_t        memoryHeapCount;
    //     VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
    // } VkPhysicalDeviceMemoryProperties;

    // typedef struct VkMemoryType {
    //     VkMemoryPropertyFlags    propertyFlags;
    //     uint32_t                 heapIndex;
    // } VkMemoryType;

    // typedef struct VkMemoryHeap {
    //     VkDeviceSize         size;
    //     VkMemoryHeapFlags    flags;
    // } VkMemoryHeap;

    // typedef enum VkMemoryPropertyFlagBits {
    //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 0x00000001,
    //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002,
    //     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004,
    //     VK_MEMORY_PROPERTY_HOST_CACHED_BIT = 0x00000008,
    //     VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT = 0x00000010,
    //   // Provided by VK_VERSION_1_1
    //     VK_MEMORY_PROPERTY_PROTECTED_BIT = 0x00000020,
    //   // Provided by VK_AMD_device_coherent_memory
    //     VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD = 0x00000040,
    //   // Provided by VK_AMD_device_coherent_memory
    //     VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD = 0x00000080,
    //   // Provided by VK_NV_external_memory_rdma
    //     VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV = 0x00000100,
    // } VkMemoryPropertyFlagBits;

    // typedef enum VkMemoryHeapFlagBits {
    //     VK_MEMORY_HEAP_DEVICE_LOCAL_BIT = 0x00000001,
    //   // Provided by VK_VERSION_1_1
    //     VK_MEMORY_HEAP_MULTI_INSTANCE_BIT = 0x00000002,
    //   // Provided by VK_KHR_device_group_creation
    //     VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHR = VK_MEMORY_HEAP_MULTI_INSTANCE_BIT,
    // } VkMemoryHeapFlagBits;

    log_debug_u32(physical_device_memory_properties->memoryTypeCount) ;
    for(
        uint32_t i = 0
    ;   i < physical_device_memory_properties->memoryTypeCount
    ;   ++i
    )
    {
        log_debug_u32(i) ;
        dump_memory_type(&physical_device_memory_properties->memoryTypes[i]) ;
    }

    log_debug_u32(physical_device_memory_properties->memoryHeapCount) ;
    for(
        uint32_t i = 0
    ;   i < physical_device_memory_properties->memoryHeapCount
    ;   ++i
    )
    {
        log_debug_u32(i) ;
        dump_memory_heap(&physical_device_memory_properties->memoryHeaps[i]) ;
    }
}


static char const *
dump_surface_transform_flag_bit_khr(
    VkSurfaceTransformFlagBitsKHR const bit
)
{

    // typedef enum VkSurfaceTransformFlagBitsKHR {
    //     VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR = 0x00000001,
    //     VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR = 0x00000002,
    //     VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR = 0x00000004,
    //     VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR = 0x00000008,
    //     VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR = 0x00000010,
    //     VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR = 0x00000020,
    //     VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR = 0x00000040,
    //     VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR = 0x00000080,
    //     VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR = 0x00000100,
    // } VkSurfaceTransformFlagBitsKHR;

    switch(bit)
    {
    case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:                     return "VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR" ;
    case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:                    return "VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR" ;
    case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:                   return "VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR" ;
    case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:                   return "VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR" ;
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR:            return "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR" ;
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR:  return "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR" ;
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR: return "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KH" ;
    case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR: return "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KH" ;
    case VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR:                      return "VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR" ;
    default:                                                        return "Unknoen Surface Transform Bit" ;
    }
}


static char const *
dump_surface_transform_flag_bits_khr(
    VkSurfaceTransformFlagBitsKHR const bits
)
{

    VkSurfaceTransformFlagBitsKHR const all_bits[] =
    {
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
    ,   VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR
    ,   VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR
    ,   VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR
    ,   VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR
    ,   VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR
    ,   VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR
    ,   VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR
    ,   VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR
    } ;
    uint32_t const all_bits_count = array_count(all_bits) ;

    static char dump_buffer[max_dump_buffer] = { 0 } ;
    SDL_memset(dump_buffer, 0, max_dump_buffer) ;
    size_t n = 0 ;

    for(
        uint32_t i = 0
    ;   i < all_bits_count
    ;   ++i
    )
    {
        if(bits & all_bits[i])
        {
            n = SDL_strlcat(dump_buffer, dump_surface_transform_flag_bit_khr(all_bits[i]), max_dump_buffer) ;
            require(n < max_dump_buffer) ;
            n = SDL_strlcat(dump_buffer, " | ", max_dump_buffer) ;
            require(n < max_dump_buffer) ;
        }
    }
    return dump_buffer ;
}


static char const *
dump_composite_alpha_flag_bit_khr(
    VkCompositeAlphaFlagBitsKHR const bit
)
{

    // typedef enum VkCompositeAlphaFlagBitsKHR {
    //     VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR = 0x00000001,
    //     VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR = 0x00000002,
    //     VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR = 0x00000004,
    //     VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR = 0x00000008,
    // } VkCompositeAlphaFlagBitsKHR;

    switch(bit)
    {
    case VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR:             return "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR" ;
    case VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR:     return "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR" ;
    case VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR:    return "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR" ;
    case VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR:            return "VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR" ;
    default:                                            return "Unknown Composite Alpha Flag Bit" ;
    }
}


static char const *
dump_composite_alpha_flag_bits_khr(
    VkSurfaceTransformFlagBitsKHR const bits
)
{
    VkCompositeAlphaFlagBitsKHR const all_bits[] =
    {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
    ,   VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
    ,   VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
    ,   VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
    } ;
    uint32_t const all_bits_count = array_count(all_bits) ;

    static char dump_buffer[max_dump_buffer] = { 0 } ;
    SDL_memset(dump_buffer, 0, max_dump_buffer) ;
    size_t n = 0 ;

    for(
        uint32_t i = 0
    ;   i < all_bits_count
    ;   ++i
    )
    {
        if(bits & all_bits[i])
        {
            n = SDL_strlcat(dump_buffer, dump_composite_alpha_flag_bit_khr(all_bits[i]), max_dump_buffer) ;
            require(n < max_dump_buffer) ;
            n = SDL_strlcat(dump_buffer, " | ", max_dump_buffer) ;
            require(n < max_dump_buffer) ;
        }
    }
    return dump_buffer ;
}


static char const *
dump_image_usage_flag_bit(
    VkImageUsageFlagBits const bit
)
{

    // typedef enum VkImageUsageFlagBits {
    //     VK_IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001,
    //     VK_IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
    //     VK_IMAGE_USAGE_SAMPLED_BIT = 0x00000004,
    //     VK_IMAGE_USAGE_STORAGE_BIT = 0x00000008,
    //     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010,
    //     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020,
    //     VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT = 0x00000040,
    //     VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT = 0x00000080,
    // // Provided by VK_KHR_video_decode_queue
    //     VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR = 0x00000400,
    // // Provided by VK_KHR_video_decode_queue
    //     VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR = 0x00000800,
    // // Provided by VK_KHR_video_decode_queue
    //     VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR = 0x00001000,
    // // Provided by VK_EXT_fragment_density_map
    //     VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT = 0x00000200,
    // // Provided by VK_KHR_fragment_shading_rate
    //     VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR = 0x00000100,
    // // Provided by VK_EXT_host_image_copy
    //     VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT = 0x00400000,
    // // Provided by VK_KHR_video_encode_queue
    //     VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR = 0x00002000,
    // // Provided by VK_KHR_video_encode_queue
    //     VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR = 0x00004000,
    // // Provided by VK_KHR_video_encode_queue
    //     VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR = 0x00008000,
    // // Provided by VK_EXT_attachment_feedback_loop_layout
    //     VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT = 0x00080000,
    // // Provided by VK_HUAWEI_invocation_mask
    //     VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI = 0x00040000,
    // // Provided by VK_QCOM_image_processing
    //     VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM = 0x00100000,
    // // Provided by VK_QCOM_image_processing
    //     VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM = 0x00200000,
    // // Provided by VK_NV_shading_rate_image
    //     VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
    // } VkImageUsageFlagBits;

    switch(bit)
    {
    case VK_IMAGE_USAGE_TRANSFER_SRC_BIT:                           return "VK_IMAGE_USAGE_TRANSFER_SRC_BIT" ;
    case VK_IMAGE_USAGE_TRANSFER_DST_BIT:                           return "VK_IMAGE_USAGE_TRANSFER_DST_BIT" ;
    case VK_IMAGE_USAGE_SAMPLED_BIT:                                return "VK_IMAGE_USAGE_SAMPLED_BIT" ;
    case VK_IMAGE_USAGE_STORAGE_BIT:                                return "VK_IMAGE_USAGE_STORAGE_BIT" ;
    case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:                       return "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT" ;
    case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT:               return "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT" ;
    case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT:                   return "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT" ;
    case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT:                       return "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT" ;
    case VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR:                   return "VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR" ;
    case VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR:                   return "VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR" ;
    case VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR:                   return "VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR" ;
    case VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT:               return "VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT" ;
    case VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR:   return "VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR" ;
    case VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT:                      return "VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT" ;
    case VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR:                   return "VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR" ;
    case VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR:                   return "VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR" ;
    case VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR:                   return "VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR" ;
    case VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT:           return "VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT" ;
    case VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI:                 return "VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI" ;
    case VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM:                     return "VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM" ;
    case VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM:                return "VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM" ;
    default:                                                        return "Unknown Image usage Flag Bit" ;
    }
}


static char const *
dump_image_usage_flag_bits(
    VkImageUsageFlagBits const bits
)
{
    VkImageUsageFlagBits const all_bits[] =
    {
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    ,   VK_IMAGE_USAGE_TRANSFER_DST_BIT
    ,   VK_IMAGE_USAGE_SAMPLED_BIT
    ,   VK_IMAGE_USAGE_STORAGE_BIT
    ,   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    ,   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    ,   VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
    ,   VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
    ,   VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR
    ,   VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR
    ,   VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR
    ,   VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT
    ,   VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
    ,   VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT
    ,   VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR
    ,   VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR
    ,   VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR
    ,   VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT
    ,   VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI
    ,   VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM
    ,   VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM
    } ;
    uint32_t const all_bits_count = array_count(all_bits) ;

    static char dump_buffer[max_dump_buffer] = { 0 } ;
    SDL_memset(dump_buffer, 0, max_dump_buffer) ;
    size_t n = 0 ;

    for(
        uint32_t i = 0
    ;   i < all_bits_count
    ;   ++i
    )
    {
        if(bits & all_bits[i])
        {
            n = SDL_strlcat(dump_buffer, dump_image_usage_flag_bit(all_bits[i]), max_dump_buffer) ;
            require(n < max_dump_buffer) ;
            n = SDL_strlcat(dump_buffer, " | ", max_dump_buffer) ;
            require(n < max_dump_buffer) ;
        }
    }
    return dump_buffer ;
}


static void
dump_surface_capabilities(
    VkSurfaceCapabilitiesKHR const * surface_capabilities
)
{
    require(surface_capabilities) ;
    // typedef struct VkSurfaceCapabilitiesKHR {
    //     uint32_t                         minImageCount;
    //     uint32_t                         maxImageCount;
    //     VkExtent2D                       currentExtent;
    //     VkExtent2D                       minImageExtent;
    //     VkExtent2D                       maxImageExtent;
    //     uint32_t                         maxImageArrayLayers;
    //     VkSurfaceTransformFlagsKHR       supportedTransforms;
    //     VkSurfaceTransformFlagBitsKHR    currentTransform;
    //     VkCompositeAlphaFlagsKHR         supportedCompositeAlpha;
    //     VkImageUsageFlags                supportedUsageFlags;
    // } VkSurfaceCapabilitiesKHR;
    // typedef struct VkExtent2D {
    //     uint32_t    width;
    //     uint32_t    height;
    // } VkExtent2D;

    log_debug_u32(surface_capabilities->minImageCount) ;
    log_debug_u32(surface_capabilities->maxImageCount) ;
    log_debug_u32(surface_capabilities->currentExtent.width) ;
    log_debug_u32(surface_capabilities->currentExtent.height) ;
    log_debug_u32(surface_capabilities->minImageExtent.width) ;
    log_debug_u32(surface_capabilities->minImageExtent.height) ;
    log_debug_u32(surface_capabilities->maxImageExtent.width) ;
    log_debug_u32(surface_capabilities->maxImageExtent.height) ;
    log_debug_u32(surface_capabilities->maxImageArrayLayers) ;

    log_debug_u32(surface_capabilities->supportedTransforms) ;
    log_debug_str(dump_surface_transform_flag_bits_khr(surface_capabilities->supportedTransforms)) ;
    log_debug_u32(surface_capabilities->currentTransform) ;
    log_debug_str(dump_surface_transform_flag_bits_khr(surface_capabilities->currentTransform)) ;
    log_debug_u32(surface_capabilities->supportedCompositeAlpha) ;
    log_debug_str(dump_composite_alpha_flag_bits_khr(surface_capabilities->supportedCompositeAlpha)) ;

    log_debug_u32(surface_capabilities->supportedUsageFlags) ;
    log_debug_str(dump_image_usage_flag_bits(surface_capabilities->supportedUsageFlags)) ;
}


static char const *
dump_vk_format(
    VkFormat const format
)
{

    // typedef enum VkFormat {
    //     VK_FORMAT_UNDEFINED = 0,
    //     VK_FORMAT_R4G4_UNORM_PACK8 = 1,
    //     VK_FORMAT_R4G4B4A4_UNORM_PACK16 = 2,
    //     VK_FORMAT_B4G4R4A4_UNORM_PACK16 = 3,
    //     VK_FORMAT_R5G6B5_UNORM_PACK16 = 4,
    //     VK_FORMAT_B5G6R5_UNORM_PACK16 = 5,
    //     VK_FORMAT_R5G5B5A1_UNORM_PACK16 = 6,
    //     VK_FORMAT_B5G5R5A1_UNORM_PACK16 = 7,
    //     VK_FORMAT_A1R5G5B5_UNORM_PACK16 = 8,
    //     VK_FORMAT_R8_UNORM = 9,
    //     VK_FORMAT_R8_SNORM = 10,
    //     VK_FORMAT_R8_USCALED = 11,
    //     VK_FORMAT_R8_SSCALED = 12,
    //     VK_FORMAT_R8_UINT = 13,
    //     VK_FORMAT_R8_SINT = 14,
    //     VK_FORMAT_R8_SRGB = 15,
    //     VK_FORMAT_R8G8_UNORM = 16,
    //     VK_FORMAT_R8G8_SNORM = 17,
    //     VK_FORMAT_R8G8_USCALED = 18,
    //     VK_FORMAT_R8G8_SSCALED = 19,
    //     VK_FORMAT_R8G8_UINT = 20,
    //     VK_FORMAT_R8G8_SINT = 21,
    //     VK_FORMAT_R8G8_SRGB = 22,
    //     VK_FORMAT_R8G8B8_UNORM = 23,
    //     VK_FORMAT_R8G8B8_SNORM = 24,
    //     VK_FORMAT_R8G8B8_USCALED = 25,
    //     VK_FORMAT_R8G8B8_SSCALED = 26,
    //     VK_FORMAT_R8G8B8_UINT = 27,
    //     VK_FORMAT_R8G8B8_SINT = 28,
    //     VK_FORMAT_R8G8B8_SRGB = 29,
    //     VK_FORMAT_B8G8R8_UNORM = 30,
    //     VK_FORMAT_B8G8R8_SNORM = 31,
    //     VK_FORMAT_B8G8R8_USCALED = 32,
    //     VK_FORMAT_B8G8R8_SSCALED = 33,
    //     VK_FORMAT_B8G8R8_UINT = 34,
    //     VK_FORMAT_B8G8R8_SINT = 35,
    //     VK_FORMAT_B8G8R8_SRGB = 36,
    //     VK_FORMAT_R8G8B8A8_UNORM = 37,
    //     VK_FORMAT_R8G8B8A8_SNORM = 38,
    //     VK_FORMAT_R8G8B8A8_USCALED = 39,
    //     VK_FORMAT_R8G8B8A8_SSCALED = 40,
    //     VK_FORMAT_R8G8B8A8_UINT = 41,
    //     VK_FORMAT_R8G8B8A8_SINT = 42,
    //     VK_FORMAT_R8G8B8A8_SRGB = 43,
    //     VK_FORMAT_B8G8R8A8_UNORM = 44,
    //     VK_FORMAT_B8G8R8A8_SNORM = 45,
    //     VK_FORMAT_B8G8R8A8_USCALED = 46,
    //     VK_FORMAT_B8G8R8A8_SSCALED = 47,
    //     VK_FORMAT_B8G8R8A8_UINT = 48,
    //     VK_FORMAT_B8G8R8A8_SINT = 49,
    //     VK_FORMAT_B8G8R8A8_SRGB = 50,
    //     VK_FORMAT_A8B8G8R8_UNORM_PACK32 = 51,
    //     VK_FORMAT_A8B8G8R8_SNORM_PACK32 = 52,
    //     VK_FORMAT_A8B8G8R8_USCALED_PACK32 = 53,
    //     VK_FORMAT_A8B8G8R8_SSCALED_PACK32 = 54,
    //     VK_FORMAT_A8B8G8R8_UINT_PACK32 = 55,
    //     VK_FORMAT_A8B8G8R8_SINT_PACK32 = 56,
    //     VK_FORMAT_A8B8G8R8_SRGB_PACK32 = 57,
    //     VK_FORMAT_A2R10G10B10_UNORM_PACK32 = 58,
    //     VK_FORMAT_A2R10G10B10_SNORM_PACK32 = 59,
    //     VK_FORMAT_A2R10G10B10_USCALED_PACK32 = 60,
    //     VK_FORMAT_A2R10G10B10_SSCALED_PACK32 = 61,
    //     VK_FORMAT_A2R10G10B10_UINT_PACK32 = 62,
    //     VK_FORMAT_A2R10G10B10_SINT_PACK32 = 63,
    //     VK_FORMAT_A2B10G10R10_UNORM_PACK32 = 64,
    //     VK_FORMAT_A2B10G10R10_SNORM_PACK32 = 65,
    //     VK_FORMAT_A2B10G10R10_USCALED_PACK32 = 66,
    //     VK_FORMAT_A2B10G10R10_SSCALED_PACK32 = 67,
    //     VK_FORMAT_A2B10G10R10_UINT_PACK32 = 68,
    //     VK_FORMAT_A2B10G10R10_SINT_PACK32 = 69,
    //     VK_FORMAT_R16_UNORM = 70,
    //     VK_FORMAT_R16_SNORM = 71,
    //     VK_FORMAT_R16_USCALED = 72,
    //     VK_FORMAT_R16_SSCALED = 73,
    //     VK_FORMAT_R16_UINT = 74,
    //     VK_FORMAT_R16_SINT = 75,
    //     VK_FORMAT_R16_SFLOAT = 76,
    //     VK_FORMAT_R16G16_UNORM = 77,
    //     VK_FORMAT_R16G16_SNORM = 78,
    //     VK_FORMAT_R16G16_USCALED = 79,
    //     VK_FORMAT_R16G16_SSCALED = 80,
    //     VK_FORMAT_R16G16_UINT = 81,
    //     VK_FORMAT_R16G16_SINT = 82,
    //     VK_FORMAT_R16G16_SFLOAT = 83,
    //     VK_FORMAT_R16G16B16_UNORM = 84,
    //     VK_FORMAT_R16G16B16_SNORM = 85,
    //     VK_FORMAT_R16G16B16_USCALED = 86,
    //     VK_FORMAT_R16G16B16_SSCALED = 87,
    //     VK_FORMAT_R16G16B16_UINT = 88,
    //     VK_FORMAT_R16G16B16_SINT = 89,
    //     VK_FORMAT_R16G16B16_SFLOAT = 90,
    //     VK_FORMAT_R16G16B16A16_UNORM = 91,
    //     VK_FORMAT_R16G16B16A16_SNORM = 92,
    //     VK_FORMAT_R16G16B16A16_USCALED = 93,
    //     VK_FORMAT_R16G16B16A16_SSCALED = 94,
    //     VK_FORMAT_R16G16B16A16_UINT = 95,
    //     VK_FORMAT_R16G16B16A16_SINT = 96,
    //     VK_FORMAT_R16G16B16A16_SFLOAT = 97,
    //     VK_FORMAT_R32_UINT = 98,
    //     VK_FORMAT_R32_SINT = 99,
    //     VK_FORMAT_R32_SFLOAT = 100,
    //     VK_FORMAT_R32G32_UINT = 101,
    //     VK_FORMAT_R32G32_SINT = 102,
    //     VK_FORMAT_R32G32_SFLOAT = 103,
    //     VK_FORMAT_R32G32B32_UINT = 104,
    //     VK_FORMAT_R32G32B32_SINT = 105,
    //     VK_FORMAT_R32G32B32_SFLOAT = 106,
    //     VK_FORMAT_R32G32B32A32_UINT = 107,
    //     VK_FORMAT_R32G32B32A32_SINT = 108,
    //     VK_FORMAT_R32G32B32A32_SFLOAT = 109,
    //     VK_FORMAT_R64_UINT = 110,
    //     VK_FORMAT_R64_SINT = 111,
    //     VK_FORMAT_R64_SFLOAT = 112,
    //     VK_FORMAT_R64G64_UINT = 113,
    //     VK_FORMAT_R64G64_SINT = 114,
    //     VK_FORMAT_R64G64_SFLOAT = 115,
    //     VK_FORMAT_R64G64B64_UINT = 116,
    //     VK_FORMAT_R64G64B64_SINT = 117,
    //     VK_FORMAT_R64G64B64_SFLOAT = 118,
    //     VK_FORMAT_R64G64B64A64_UINT = 119,
    //     VK_FORMAT_R64G64B64A64_SINT = 120,
    //     VK_FORMAT_R64G64B64A64_SFLOAT = 121,
    //     VK_FORMAT_B10G11R11_UFLOAT_PACK32 = 122,
    //     VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 = 123,
    //     VK_FORMAT_D16_UNORM = 124,
    //     VK_FORMAT_X8_D24_UNORM_PACK32 = 125,
    //     VK_FORMAT_D32_SFLOAT = 126,
    //     VK_FORMAT_S8_UINT = 127,
    //     VK_FORMAT_D16_UNORM_S8_UINT = 128,
    //     VK_FORMAT_D24_UNORM_S8_UINT = 129,
    //     VK_FORMAT_D32_SFLOAT_S8_UINT = 130,
    //     VK_FORMAT_BC1_RGB_UNORM_BLOCK = 131,
    //     VK_FORMAT_BC1_RGB_SRGB_BLOCK = 132,
    //     VK_FORMAT_BC1_RGBA_UNORM_BLOCK = 133,
    //     VK_FORMAT_BC1_RGBA_SRGB_BLOCK = 134,
    //     VK_FORMAT_BC2_UNORM_BLOCK = 135,
    //     VK_FORMAT_BC2_SRGB_BLOCK = 136,
    //     VK_FORMAT_BC3_UNORM_BLOCK = 137,
    //     VK_FORMAT_BC3_SRGB_BLOCK = 138,
    //     VK_FORMAT_BC4_UNORM_BLOCK = 139,
    //     VK_FORMAT_BC4_SNORM_BLOCK = 140,
    //     VK_FORMAT_BC5_UNORM_BLOCK = 141,
    //     VK_FORMAT_BC5_SNORM_BLOCK = 142,
    //     VK_FORMAT_BC6H_UFLOAT_BLOCK = 143,
    //     VK_FORMAT_BC6H_SFLOAT_BLOCK = 144,
    //     VK_FORMAT_BC7_UNORM_BLOCK = 145,
    //     VK_FORMAT_BC7_SRGB_BLOCK = 146,
    //     VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK = 147,
    //     VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK = 148,
    //     VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK = 149,
    //     VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK = 150,
    //     VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK = 151,
    //     VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK = 152,
    //     VK_FORMAT_EAC_R11_UNORM_BLOCK = 153,
    //     VK_FORMAT_EAC_R11_SNORM_BLOCK = 154,
    //     VK_FORMAT_EAC_R11G11_UNORM_BLOCK = 155,
    //     VK_FORMAT_EAC_R11G11_SNORM_BLOCK = 156,
    //     VK_FORMAT_ASTC_4x4_UNORM_BLOCK = 157,
    //     VK_FORMAT_ASTC_4x4_SRGB_BLOCK = 158,
    //     VK_FORMAT_ASTC_5x4_UNORM_BLOCK = 159,
    //     VK_FORMAT_ASTC_5x4_SRGB_BLOCK = 160,
    //     VK_FORMAT_ASTC_5x5_UNORM_BLOCK = 161,
    //     VK_FORMAT_ASTC_5x5_SRGB_BLOCK = 162,
    //     VK_FORMAT_ASTC_6x5_UNORM_BLOCK = 163,
    //     VK_FORMAT_ASTC_6x5_SRGB_BLOCK = 164,
    //     VK_FORMAT_ASTC_6x6_UNORM_BLOCK = 165,
    //     VK_FORMAT_ASTC_6x6_SRGB_BLOCK = 166,
    //     VK_FORMAT_ASTC_8x5_UNORM_BLOCK = 167,
    //     VK_FORMAT_ASTC_8x5_SRGB_BLOCK = 168,
    //     VK_FORMAT_ASTC_8x6_UNORM_BLOCK = 169,
    //     VK_FORMAT_ASTC_8x6_SRGB_BLOCK = 170,
    //     VK_FORMAT_ASTC_8x8_UNORM_BLOCK = 171,
    //     VK_FORMAT_ASTC_8x8_SRGB_BLOCK = 172,
    //     VK_FORMAT_ASTC_10x5_UNORM_BLOCK = 173,
    //     VK_FORMAT_ASTC_10x5_SRGB_BLOCK = 174,
    //     VK_FORMAT_ASTC_10x6_UNORM_BLOCK = 175,
    //     VK_FORMAT_ASTC_10x6_SRGB_BLOCK = 176,
    //     VK_FORMAT_ASTC_10x8_UNORM_BLOCK = 177,
    //     VK_FORMAT_ASTC_10x8_SRGB_BLOCK = 178,
    //     VK_FORMAT_ASTC_10x10_UNORM_BLOCK = 179,
    //     VK_FORMAT_ASTC_10x10_SRGB_BLOCK = 180,
    //     VK_FORMAT_ASTC_12x10_UNORM_BLOCK = 181,
    //     VK_FORMAT_ASTC_12x10_SRGB_BLOCK = 182,
    //     VK_FORMAT_ASTC_12x12_UNORM_BLOCK = 183,
    //     VK_FORMAT_ASTC_12x12_SRGB_BLOCK = 184,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G8B8G8R8_422_UNORM = 1000156000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_B8G8R8G8_422_UNORM = 1000156001,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM = 1000156002,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G8_B8R8_2PLANE_420_UNORM = 1000156003,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM = 1000156004,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G8_B8R8_2PLANE_422_UNORM = 1000156005,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM = 1000156006,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_R10X6_UNORM_PACK16 = 1000156007,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_R10X6G10X6_UNORM_2PACK16 = 1000156008,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 1000156009,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 1000156010,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 1000156011,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 1000156013,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 1000156015,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_R12X4_UNORM_PACK16 = 1000156017,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_R12X4G12X4_UNORM_2PACK16 = 1000156018,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 1000156019,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 1000156020,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 1000156021,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 1000156023,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 1000156025,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G16B16G16R16_422_UNORM = 1000156027,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_B16G16R16G16_422_UNORM = 1000156028,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM = 1000156029,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G16_B16R16_2PLANE_420_UNORM = 1000156030,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM = 1000156031,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G16_B16R16_2PLANE_422_UNORM = 1000156032,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM = 1000156033,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_G8_B8R8_2PLANE_444_UNORM = 1000330000,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16 = 1000330001,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16 = 1000330002,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_G16_B16R16_2PLANE_444_UNORM = 1000330003,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_A4R4G4B4_UNORM_PACK16 = 1000340000,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_A4B4G4R4_UNORM_PACK16 = 1000340001,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK = 1000066000,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK = 1000066001,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK = 1000066002,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK = 1000066003,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK = 1000066004,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK = 1000066005,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK = 1000066006,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK = 1000066007,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK = 1000066008,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK = 1000066009,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK = 1000066010,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK = 1000066011,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK = 1000066012,
    // // Provided by VK_VERSION_1_3
    //     VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK = 1000066013,
    // // Provided by VK_IMG_format_pvrtc
    //     VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG = 1000054000,
    // // Provided by VK_IMG_format_pvrtc
    //     VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG = 1000054001,
    // // Provided by VK_IMG_format_pvrtc
    //     VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG = 1000054002,
    // // Provided by VK_IMG_format_pvrtc
    //     VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG = 1000054003,
    // // Provided by VK_IMG_format_pvrtc
    //     VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG = 1000054004,
    // // Provided by VK_IMG_format_pvrtc
    //     VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG = 1000054005,
    // // Provided by VK_IMG_format_pvrtc
    //     VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG = 1000054006,
    // // Provided by VK_IMG_format_pvrtc
    //     VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG = 1000054007,
    // // Provided by VK_NV_optical_flow
    //     VK_FORMAT_R16G16_SFIXED5_NV = 1000464000,
    //       VK_FORMAT_R16G16_S10_5_NV   = 1000464000,
    // // Provided by VK_KHR_maintenance5
    //     VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR = 1000470000,
    // // Provided by VK_KHR_maintenance5
    //     VK_FORMAT_A8_UNORM_KHR = 1000470001,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
    // // Provided by VK_EXT_texture_compression_astc_hdr
    //     VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G8B8G8R8_422_UNORM_KHR = VK_FORMAT_G8B8G8R8_422_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_B8G8R8G8_422_UNORM_KHR = VK_FORMAT_B8G8R8G8_422_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_R10X6_UNORM_PACK16_KHR = VK_FORMAT_R10X6_UNORM_PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR = VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_R12X4_UNORM_PACK16_KHR = VK_FORMAT_R12X4_UNORM_PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR = VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G16B16G16R16_422_UNORM_KHR = VK_FORMAT_G16B16G16R16_422_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_B16G16R16G16_422_UNORM_KHR = VK_FORMAT_B16G16R16G16_422_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
    // // Provided by VK_EXT_ycbcr_2plane_444_formats
    //     VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
    // // Provided by VK_EXT_ycbcr_2plane_444_formats
    //     VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
    // // Provided by VK_EXT_ycbcr_2plane_444_formats
    //     VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
    // // Provided by VK_EXT_ycbcr_2plane_444_formats
    //     VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT = VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
    // // Provided by VK_EXT_4444_formats
    //     VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT = VK_FORMAT_A4R4G4B4_UNORM_PACK16,
    // // Provided by VK_EXT_4444_formats
    //     VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT = VK_FORMAT_A4B4G4R4_UNORM_PACK16,
    // // Provided by VK_NV_optical_flow
    //     VK_FORMAT_R16G16_S10_5_NV = VK_FORMAT_R16G16_SFIXED5_NV,
    // } VkFormat;

    switch(format)
    {
    case VK_FORMAT_UNDEFINED:                                   return "VK_FORMAT_UNDEFINED" ;
    case VK_FORMAT_R4G4_UNORM_PACK8:                            return "VK_FORMAT_R4G4_UNORM_PACK8" ;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:                       return "VK_FORMAT_R4G4B4A4_UNORM_PACK16" ;
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:                       return "VK_FORMAT_B4G4R4A4_UNORM_PACK16" ;
    case VK_FORMAT_R5G6B5_UNORM_PACK16:                         return "VK_FORMAT_R5G6B5_UNORM_PACK16" ;
    case VK_FORMAT_B5G6R5_UNORM_PACK16:                         return "VK_FORMAT_B5G6R5_UNORM_PACK16" ;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:                       return "VK_FORMAT_R5G5B5A1_UNORM_PACK16" ;
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:                       return "VK_FORMAT_B5G5R5A1_UNORM_PACK16" ;
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:                       return "VK_FORMAT_A1R5G5B5_UNORM_PACK16" ;
    case VK_FORMAT_R8_UNORM:                                    return "VK_FORMAT_R8_UNORM" ;
    case VK_FORMAT_R8_SNORM:                                    return "VK_FORMAT_R8_SNORM" ;
    case VK_FORMAT_R8_USCALED:                                  return "VK_FORMAT_R8_USCALED" ;
    case VK_FORMAT_R8_SSCALED:                                  return "VK_FORMAT_R8_SSCALED" ;
    case VK_FORMAT_R8_UINT:                                     return "VK_FORMAT_R8_UINT" ;
    case VK_FORMAT_R8_SINT:                                     return "VK_FORMAT_R8_SINT" ;
    case VK_FORMAT_R8_SRGB:                                     return "VK_FORMAT_R8_SRGB" ;
    case VK_FORMAT_R8G8_UNORM:                                  return "VK_FORMAT_R8G8_UNORM" ;
    case VK_FORMAT_R8G8_SNORM:                                  return "VK_FORMAT_R8G8_SNORM" ;
    case VK_FORMAT_R8G8_USCALED:                                return "VK_FORMAT_R8G8_USCALED" ;
    case VK_FORMAT_R8G8_SSCALED:                                return "VK_FORMAT_R8G8_SSCALED" ;
    case VK_FORMAT_R8G8_UINT:                                   return "VK_FORMAT_R8G8_UINT" ;
    case VK_FORMAT_R8G8_SINT:                                   return "VK_FORMAT_R8G8_SINT" ;
    case VK_FORMAT_R8G8_SRGB:                                   return "VK_FORMAT_R8G8_SRGB" ;
    case VK_FORMAT_R8G8B8_UNORM:                                return "VK_FORMAT_R8G8B8_UNORM" ;
    case VK_FORMAT_R8G8B8_SNORM:                                return "VK_FORMAT_R8G8B8_SNORM" ;
    case VK_FORMAT_R8G8B8_USCALED:                              return "VK_FORMAT_R8G8B8_USCALED" ;
    case VK_FORMAT_R8G8B8_SSCALED:                              return "VK_FORMAT_R8G8B8_SSCALED" ;
    case VK_FORMAT_R8G8B8_UINT:                                 return "VK_FORMAT_R8G8B8_UINT" ;
    case VK_FORMAT_R8G8B8_SINT:                                 return "VK_FORMAT_R8G8B8_SINT" ;
    case VK_FORMAT_R8G8B8_SRGB:                                 return "VK_FORMAT_R8G8B8_SRGB" ;
    case VK_FORMAT_B8G8R8_UNORM:                                return "VK_FORMAT_B8G8R8_UNORM" ;
    case VK_FORMAT_B8G8R8_SNORM:                                return "VK_FORMAT_B8G8R8_SNORM" ;
    case VK_FORMAT_B8G8R8_USCALED:                              return "VK_FORMAT_B8G8R8_USCALED" ;
    case VK_FORMAT_B8G8R8_SSCALED:                              return "VK_FORMAT_B8G8R8_SSCALED" ;
    case VK_FORMAT_B8G8R8_UINT:                                 return "VK_FORMAT_B8G8R8_UINT" ;
    case VK_FORMAT_B8G8R8_SINT:                                 return "VK_FORMAT_B8G8R8_SINT" ;
    case VK_FORMAT_B8G8R8_SRGB:                                 return "VK_FORMAT_B8G8R8_SRGB" ;
    case VK_FORMAT_R8G8B8A8_UNORM:                              return "VK_FORMAT_R8G8B8A8_UNORM" ;
    case VK_FORMAT_R8G8B8A8_SNORM:                              return "VK_FORMAT_R8G8B8A8_SNORM" ;
    case VK_FORMAT_R8G8B8A8_USCALED:                            return "VK_FORMAT_R8G8B8A8_USCALED" ;
    case VK_FORMAT_R8G8B8A8_SSCALED:                            return "VK_FORMAT_R8G8B8A8_SSCALED" ;
    case VK_FORMAT_R8G8B8A8_UINT:                               return "VK_FORMAT_R8G8B8A8_UINT" ;
    case VK_FORMAT_R8G8B8A8_SINT:                               return "VK_FORMAT_R8G8B8A8_SINT" ;
    case VK_FORMAT_R8G8B8A8_SRGB:                               return "VK_FORMAT_R8G8B8A8_SRGB" ;
    case VK_FORMAT_B8G8R8A8_UNORM:                              return "VK_FORMAT_B8G8R8A8_UNORM" ;
    case VK_FORMAT_B8G8R8A8_SNORM:                              return "VK_FORMAT_B8G8R8A8_SNORM" ;
    case VK_FORMAT_B8G8R8A8_USCALED:                            return "VK_FORMAT_B8G8R8A8_USCALED" ;
    case VK_FORMAT_B8G8R8A8_SSCALED:                            return "VK_FORMAT_B8G8R8A8_SSCALED" ;
    case VK_FORMAT_B8G8R8A8_UINT:                               return "VK_FORMAT_B8G8R8A8_UINT" ;
    case VK_FORMAT_B8G8R8A8_SINT:                               return "VK_FORMAT_B8G8R8A8_SINT" ;
    case VK_FORMAT_B8G8R8A8_SRGB:                               return "VK_FORMAT_B8G8R8A8_SRGB" ;
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:                       return "VK_FORMAT_A8B8G8R8_UNORM_PACK32" ;
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:                       return "VK_FORMAT_A8B8G8R8_SNORM_PACK32" ;
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:                     return "VK_FORMAT_A8B8G8R8_USCALED_PACK32" ;
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:                     return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32" ;
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:                        return "VK_FORMAT_A8B8G8R8_UINT_PACK32" ;
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:                        return "VK_FORMAT_A8B8G8R8_SINT_PACK32" ;
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:                        return "VK_FORMAT_A8B8G8R8_SRGB_PACK32" ;
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:                    return "VK_FORMAT_A2R10G10B10_UNORM_PACK32" ;
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:                    return "VK_FORMAT_A2R10G10B10_SNORM_PACK32" ;
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:                  return "VK_FORMAT_A2R10G10B10_USCALED_PACK32" ;
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:                  return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32" ;
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:                     return "VK_FORMAT_A2R10G10B10_UINT_PACK32" ;
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:                     return "VK_FORMAT_A2R10G10B10_SINT_PACK32" ;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:                    return "VK_FORMAT_A2B10G10R10_UNORM_PACK32" ;
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:                    return "VK_FORMAT_A2B10G10R10_SNORM_PACK32" ;
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:                  return "VK_FORMAT_A2B10G10R10_USCALED_PACK32" ;
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:                  return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32" ;
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:                     return "VK_FORMAT_A2B10G10R10_UINT_PACK32" ;
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:                     return "VK_FORMAT_A2B10G10R10_SINT_PACK32" ;
    case VK_FORMAT_R16_UNORM:                                   return "VK_FORMAT_R16_UNORM" ;
    case VK_FORMAT_R16_SNORM:                                   return "VK_FORMAT_R16_SNORM" ;
    case VK_FORMAT_R16_USCALED:                                 return "VK_FORMAT_R16_USCALED" ;
    case VK_FORMAT_R16_SSCALED:                                 return "VK_FORMAT_R16_SSCALED" ;
    case VK_FORMAT_R16_UINT:                                    return "VK_FORMAT_R16_UINT" ;
    case VK_FORMAT_R16_SINT:                                    return "VK_FORMAT_R16_SINT" ;
    case VK_FORMAT_R16_SFLOAT:                                  return "VK_FORMAT_R16_SFLOAT" ;
    case VK_FORMAT_R16G16_UNORM:                                return "VK_FORMAT_R16G16_UNORM" ;
    case VK_FORMAT_R16G16_SNORM:                                return "VK_FORMAT_R16G16_SNORM" ;
    case VK_FORMAT_R16G16_USCALED:                              return "VK_FORMAT_R16G16_USCALED" ;
    case VK_FORMAT_R16G16_SSCALED:                              return "VK_FORMAT_R16G16_SSCALED" ;
    case VK_FORMAT_R16G16_UINT:                                 return "VK_FORMAT_R16G16_UINT" ;
    case VK_FORMAT_R16G16_SINT:                                 return "VK_FORMAT_R16G16_SINT" ;
    case VK_FORMAT_R16G16_SFLOAT:                               return "VK_FORMAT_R16G16_SFLOAT" ;
    case VK_FORMAT_R16G16B16_UNORM:                             return "VK_FORMAT_R16G16B16_UNORM" ;
    case VK_FORMAT_R16G16B16_SNORM:                             return "VK_FORMAT_R16G16B16_SNORM" ;
    case VK_FORMAT_R16G16B16_USCALED:                           return "VK_FORMAT_R16G16B16_USCALED" ;
    case VK_FORMAT_R16G16B16_SSCALED:                           return "VK_FORMAT_R16G16B16_SSCALED" ;
    case VK_FORMAT_R16G16B16_UINT:                              return "VK_FORMAT_R16G16B16_UINT" ;
    case VK_FORMAT_R16G16B16_SINT:                              return "VK_FORMAT_R16G16B16_SINT" ;
    case VK_FORMAT_R16G16B16_SFLOAT:                            return "VK_FORMAT_R16G16B16_SFLOAT" ;
    case VK_FORMAT_R16G16B16A16_UNORM:                          return "VK_FORMAT_R16G16B16A16_UNORM" ;
    case VK_FORMAT_R16G16B16A16_SNORM:                          return "VK_FORMAT_R16G16B16A16_SNORM" ;
    case VK_FORMAT_R16G16B16A16_USCALED:                        return "VK_FORMAT_R16G16B16A16_USCALED" ;
    case VK_FORMAT_R16G16B16A16_SSCALED:                        return "VK_FORMAT_R16G16B16A16_SSCALED" ;
    case VK_FORMAT_R16G16B16A16_UINT:                           return "VK_FORMAT_R16G16B16A16_UINT" ;
    case VK_FORMAT_R16G16B16A16_SINT:                           return "VK_FORMAT_R16G16B16A16_SINT" ;
    case VK_FORMAT_R16G16B16A16_SFLOAT:                         return "VK_FORMAT_R16G16B16A16_SFLOAT" ;
    case VK_FORMAT_R32_UINT:                                    return "VK_FORMAT_R32_UINT" ;
    case VK_FORMAT_R32_SINT:                                    return "VK_FORMAT_R32_SINT" ;
    case VK_FORMAT_R32_SFLOAT:                                  return "VK_FORMAT_R32_SFLOAT" ;
    case VK_FORMAT_R32G32_UINT:                                 return "VK_FORMAT_R32G32_UINT" ;
    case VK_FORMAT_R32G32_SINT:                                 return "VK_FORMAT_R32G32_SINT" ;
    case VK_FORMAT_R32G32_SFLOAT:                               return "VK_FORMAT_R32G32_SFLOAT" ;
    case VK_FORMAT_R32G32B32_UINT:                              return "VK_FORMAT_R32G32B32_UINT" ;
    case VK_FORMAT_R32G32B32_SINT:                              return "VK_FORMAT_R32G32B32_SINT" ;
    case VK_FORMAT_R32G32B32_SFLOAT:                            return "VK_FORMAT_R32G32B32_SFLOAT" ;
    case VK_FORMAT_R32G32B32A32_UINT:                           return "VK_FORMAT_R32G32B32A32_UINT" ;
    case VK_FORMAT_R32G32B32A32_SINT:                           return "VK_FORMAT_R32G32B32A32_SINT" ;
    case VK_FORMAT_R32G32B32A32_SFLOAT:                         return "VK_FORMAT_R32G32B32A32_SFLOAT" ;
    case VK_FORMAT_R64_UINT:                                    return "VK_FORMAT_R64_UINT" ;
    case VK_FORMAT_R64_SINT:                                    return "VK_FORMAT_R64_SINT" ;
    case VK_FORMAT_R64_SFLOAT:                                  return "VK_FORMAT_R64_SFLOAT" ;
    case VK_FORMAT_R64G64_UINT:                                 return "VK_FORMAT_R64G64_UINT" ;
    case VK_FORMAT_R64G64_SINT:                                 return "VK_FORMAT_R64G64_SINT" ;
    case VK_FORMAT_R64G64_SFLOAT:                               return "VK_FORMAT_R64G64_SFLOAT" ;
    case VK_FORMAT_R64G64B64_UINT:                              return "VK_FORMAT_R64G64B64_UINT" ;
    case VK_FORMAT_R64G64B64_SINT:                              return "VK_FORMAT_R64G64B64_SINT" ;
    case VK_FORMAT_R64G64B64_SFLOAT:                            return "VK_FORMAT_R64G64B64_SFLOAT" ;
    case VK_FORMAT_R64G64B64A64_UINT:                           return "VK_FORMAT_R64G64B64A64_UINT" ;
    case VK_FORMAT_R64G64B64A64_SINT:                           return "VK_FORMAT_R64G64B64A64_SINT" ;
    case VK_FORMAT_R64G64B64A64_SFLOAT:                         return "VK_FORMAT_R64G64B64A64_SFLOAT" ;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:                     return "VK_FORMAT_B10G11R11_UFLOAT_PACK32" ;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:                      return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32" ;
    case VK_FORMAT_D16_UNORM:                                   return "VK_FORMAT_D16_UNORM" ;
    case VK_FORMAT_X8_D24_UNORM_PACK32:                         return "VK_FORMAT_X8_D24_UNORM_PACK32" ;
    case VK_FORMAT_D32_SFLOAT:                                  return "VK_FORMAT_D32_SFLOAT" ;
    case VK_FORMAT_S8_UINT:                                     return "VK_FORMAT_S8_UINT" ;
    case VK_FORMAT_D16_UNORM_S8_UINT:                           return "VK_FORMAT_D16_UNORM_S8_UINT" ;
    case VK_FORMAT_D24_UNORM_S8_UINT:                           return "VK_FORMAT_D24_UNORM_S8_UINT" ;
    case VK_FORMAT_D32_SFLOAT_S8_UINT:                          return "VK_FORMAT_D32_SFLOAT_S8_UINT" ;
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:                         return "VK_FORMAT_BC1_RGB_UNORM_BLOCK" ;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:                          return "VK_FORMAT_BC1_RGB_SRGB_BLOCK" ;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:                        return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK" ;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:                         return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK" ;
    case VK_FORMAT_BC2_UNORM_BLOCK:                             return "VK_FORMAT_BC2_UNORM_BLOCK" ;
    case VK_FORMAT_BC2_SRGB_BLOCK:                              return "VK_FORMAT_BC2_SRGB_BLOCK" ;
    case VK_FORMAT_BC3_UNORM_BLOCK:                             return "VK_FORMAT_BC3_UNORM_BLOCK" ;
    case VK_FORMAT_BC3_SRGB_BLOCK:                              return "VK_FORMAT_BC3_SRGB_BLOCK" ;
    case VK_FORMAT_BC4_UNORM_BLOCK:                             return "VK_FORMAT_BC4_UNORM_BLOCK" ;
    case VK_FORMAT_BC4_SNORM_BLOCK:                             return "VK_FORMAT_BC4_SNORM_BLOCK" ;
    case VK_FORMAT_BC5_UNORM_BLOCK:                             return "VK_FORMAT_BC5_UNORM_BLOCK" ;
    case VK_FORMAT_BC5_SNORM_BLOCK:                             return "VK_FORMAT_BC5_SNORM_BLOCK" ;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:                           return "VK_FORMAT_BC6H_UFLOAT_BLOCK" ;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:                           return "VK_FORMAT_BC6H_SFLOAT_BLOCK" ;
    case VK_FORMAT_BC7_UNORM_BLOCK:                             return "VK_FORMAT_BC7_UNORM_BLOCK" ;
    case VK_FORMAT_BC7_SRGB_BLOCK:                              return "VK_FORMAT_BC7_SRGB_BLOCK" ;
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:                     return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK" ;
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:                      return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK" ;
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:                   return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK" ;
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:                    return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK" ;
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:                   return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK" ;
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:                    return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK" ;
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:                         return "VK_FORMAT_EAC_R11_UNORM_BLOCK" ;
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:                         return "VK_FORMAT_EAC_R11_SNORM_BLOCK" ;
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:                      return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK" ;
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:                      return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK" ;
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:                        return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:                         return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:                        return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:                         return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:                        return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:                         return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:                        return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:                         return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:                        return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:                         return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:                        return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:                         return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:                        return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:                         return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:                        return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:                         return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:                       return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:                        return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:                       return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:                        return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:                       return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:                        return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:                      return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:                       return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:                      return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:                       return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK" ;
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:                      return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK" ;
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:                       return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK" ;
    case VK_FORMAT_G8B8G8R8_422_UNORM:                          return "VK_FORMAT_G8B8G8R8_422_UNORM" ;
    case VK_FORMAT_B8G8R8G8_422_UNORM:                          return "VK_FORMAT_B8G8R8G8_422_UNORM" ;
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:                   return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM" ;
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:                    return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM" ;
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:                   return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM" ;
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:                    return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM" ;
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:                   return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM" ;
    case VK_FORMAT_R10X6_UNORM_PACK16:                          return "VK_FORMAT_R10X6_UNORM_PACK16" ;
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:                    return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16" ;
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:          return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16" ;
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:      return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16" ;
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:      return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16" ;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:  return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16" ;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:   return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16" ;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:  return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16" ;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:   return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16" ;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:  return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16" ;
    case VK_FORMAT_R12X4_UNORM_PACK16:                          return "VK_FORMAT_R12X4_UNORM_PACK16" ;
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:                    return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16" ;
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:          return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16" ;
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:      return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16" ;
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:      return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16" ;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:  return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16" ;
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:   return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16" ;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:  return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16" ;
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:   return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16" ;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:  return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16" ;
    case VK_FORMAT_G16B16G16R16_422_UNORM:                      return "VK_FORMAT_G16B16G16R16_422_UNORM" ;
    case VK_FORMAT_B16G16R16G16_422_UNORM:                      return "VK_FORMAT_B16G16R16G16_422_UNORM" ;
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:                return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM" ;
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:                 return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM" ;
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:                return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM" ;
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:                 return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM" ;
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:                return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM" ;
    case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:                    return "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM" ;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:   return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16" ;
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:   return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16" ;
    case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:                 return "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM" ;
    case VK_FORMAT_A4R4G4B4_UNORM_PACK16:                       return "VK_FORMAT_A4R4G4B4_UNORM_PACK16" ;
    case VK_FORMAT_A4B4G4R4_UNORM_PACK16:                       return "VK_FORMAT_A4B4G4R4_UNORM_PACK16" ;
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:                       return "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:                       return "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:                       return "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:                       return "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:                       return "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:                       return "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:                       return "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:                       return "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:                      return "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:                      return "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:                      return "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:                     return "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:                     return "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK" ;
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:                     return "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK" ;
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:                 return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG" ;
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:                 return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG" ;
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:                 return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG" ;
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:                 return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG" ;
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:                  return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG" ;
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:                  return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG" ;
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:                  return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG" ;
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:                  return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG" ;
    case VK_FORMAT_R16G16_S10_5_NV:                             return "VK_FORMAT_R16G16_S10_5_NV" ;
    case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR:                   return "VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR" ;
    case VK_FORMAT_A8_UNORM_KHR:                                return "VK_FORMAT_A8_UNORM_KHR" ;
    default:                                                    return "Unknown Format" ;
    }
}


static char const *
dump_color_space(
    VkColorSpaceKHR const color_space
)
{
    // // Provided by VK_KHR_surface
    // typedef enum VkColorSpaceKHR {
    //     VK_COLOR_SPACE_SRGB_NONLINEAR_KHR = 0,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT = 1000104001,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT = 1000104002,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT = 1000104003,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT = 1000104004,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_BT709_LINEAR_EXT = 1000104005,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_BT709_NONLINEAR_EXT = 1000104006,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_BT2020_LINEAR_EXT = 1000104007,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_HDR10_ST2084_EXT = 1000104008,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_DOLBYVISION_EXT = 1000104009,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_HDR10_HLG_EXT = 1000104010,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT = 1000104011,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT = 1000104012,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_PASS_THROUGH_EXT = 1000104013,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT = 1000104014,
    //   // Provided by VK_AMD_display_native_hdr
    //     VK_COLOR_SPACE_DISPLAY_NATIVE_AMD = 1000213000,
    //     VK_COLORSPACE_SRGB_NONLINEAR_KHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    //   // Provided by VK_EXT_swapchain_colorspace
    //     VK_COLOR_SPACE_DCI_P3_LINEAR_EXT = VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,
    // } VkColorSpaceKHR;

    switch(color_space)
    {
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:             return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR" ;
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:       return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT" ;
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:       return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT" ;
    case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:          return "VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT" ;
    case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:           return "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT" ;
    case VK_COLOR_SPACE_BT709_LINEAR_EXT:               return "VK_COLOR_SPACE_BT709_LINEAR_EXT" ;
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:            return "VK_COLOR_SPACE_BT709_NONLINEAR_EXT" ;
    case VK_COLOR_SPACE_BT2020_LINEAR_EXT:              return "VK_COLOR_SPACE_BT2020_LINEAR_EXT" ;
    case VK_COLOR_SPACE_HDR10_ST2084_EXT:               return "VK_COLOR_SPACE_HDR10_ST2084_EXT" ;
    case VK_COLOR_SPACE_DOLBYVISION_EXT:                return "VK_COLOR_SPACE_DOLBYVISION_EXT" ;
    case VK_COLOR_SPACE_HDR10_HLG_EXT:                  return "VK_COLOR_SPACE_HDR10_HLG_EXT" ;
    case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:            return "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT" ;
    case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:         return "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT" ;
    case VK_COLOR_SPACE_PASS_THROUGH_EXT:               return "VK_COLOR_SPACE_PASS_THROUGH_EXT" ;
    case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:    return "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT" ;
    case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:             return "VK_COLOR_SPACE_DISPLAY_NATIVE_AMD" ;
    //case VK_COLORSPACE_SRGB_NONLINEAR_KHR:              return "VK_COLORSPACE_SRGB_NONLINEAR_KHR" ;
    //case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT:              return "VK_COLOR_SPACE_DCI_P3_LINEAR_EXT" ;
    default:                                            return "Unknow Color Space" ;
    }
}


static void
dump_surface_format_khr(
    VkSurfaceFormatKHR const * surface_format
)
{
    require(surface_format) ;

    // typedef struct VkSurfaceFormatKHR {
    //     VkFormat           format;
    //     VkColorSpaceKHR    colorSpace;
    // } VkSurfaceFormatKHR;

    log_debug_u32(surface_format->format) ;
    log_debug_u32(surface_format->colorSpace) ;
    log_debug_str(dump_vk_format(surface_format->format)) ;
    log_debug_str(dump_color_space(surface_format->colorSpace)) ;
}



static char const *
dump_present_mode_khr(
    VkPresentModeKHR const present_mode
)
{
    // typedef enum VkPresentModeKHR {
    //     VK_PRESENT_MODE_IMMEDIATE_KHR = 0,
    //     VK_PRESENT_MODE_MAILBOX_KHR = 1,
    //     VK_PRESENT_MODE_FIFO_KHR = 2,
    //     VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
    // // Provided by VK_KHR_shared_presentable_image
    //     VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR = 1000111000,
    // // Provided by VK_KHR_shared_presentable_image
    //     VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR = 1000111001,
    // } VkPresentModeKHR;
    switch(present_mode)
    {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:                 return "VK_PRESENT_MODE_IMMEDIATE_KHR" ;
    case VK_PRESENT_MODE_MAILBOX_KHR:                   return "VK_PRESENT_MODE_MAILBOX_KHR" ;
    case VK_PRESENT_MODE_FIFO_KHR:                      return "VK_PRESENT_MODE_FIFO_KHR" ;
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:              return "VK_PRESENT_MODE_FIFO_RELAXED_KHR" ;
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:     return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR" ;
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR: return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR" ;
    default:                                            return "Unknown Present Mode" ;
    }
}


static char const *
dump_format_feature_flag_bit(
    VkFormatFeatureFlagBits const bit
)
{
    // typedef enum VkFormatFeatureFlagBits {
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT = 0x00000001,
    //     VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT = 0x00000002,
    //     VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT = 0x00000004,
    //     VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT = 0x00000008,
    //     VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT = 0x00000010,
    //     VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT = 0x00000020,
    //     VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT = 0x00000040,
    //     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT = 0x00000080,
    //     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT = 0x00000100,
    //     VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000200,
    //     VK_FORMAT_FEATURE_BLIT_SRC_BIT = 0x00000400,
    //     VK_FORMAT_FEATURE_BLIT_DST_BIT = 0x00000800,
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT = 0x00001000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT = 0x00004000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_TRANSFER_DST_BIT = 0x00008000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT = 0x00020000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT = 0x00040000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT = 0x00080000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT = 0x00100000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT = 0x00200000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_DISJOINT_BIT = 0x00400000,
    // // Provided by VK_VERSION_1_1
    //     VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT = 0x00800000,
    // // Provided by VK_VERSION_1_2
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT = 0x00010000,
    // // Provided by VK_KHR_video_decode_queue
    //     VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR = 0x02000000,
    // // Provided by VK_KHR_video_decode_queue
    //     VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR = 0x04000000,
    // // Provided by VK_KHR_acceleration_structure
    //     VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR = 0x20000000,
    // // Provided by VK_EXT_filter_cubic
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT = 0x00002000,
    // // Provided by VK_EXT_fragment_density_map
    //     VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT = 0x01000000,
    // // Provided by VK_KHR_fragment_shading_rate
    //     VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR = 0x40000000,
    // // Provided by VK_KHR_video_encode_queue
    //     VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR = 0x08000000,
    // // Provided by VK_KHR_video_encode_queue
    //     VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR = 0x10000000,
    // // Provided by VK_IMG_filter_cubic
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT,
    // // Provided by VK_KHR_maintenance1
    //     VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT,
    // // Provided by VK_KHR_maintenance1
    //     VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR = VK_FORMAT_FEATURE_TRANSFER_DST_BIT,
    // // Provided by VK_EXT_sampler_filter_minmax
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT = VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT_KHR = VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT_KHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT_KHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT_KHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT_KHR = VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_FEATURE_DISJOINT_BIT_KHR = VK_FORMAT_FEATURE_DISJOINT_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT_KHR = VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT,
    // } VkFormatFeatureFlagBits;

    switch(bit)
    {
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT:                                                           return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT" ;
    case VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT:                                                           return "VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT" ;
    case VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT:                                                    return "VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT" ;
    case VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT:                                                    return "VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT" ;
    case VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT:                                                    return "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT" ;
    case VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT:                                             return "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT" ;
    case VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT:                                                           return "VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT" ;
    case VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT:                                                        return "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT" ;
    case VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT:                                                  return "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT" ;
    case VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT:                                                return "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT" ;
    case VK_FORMAT_FEATURE_BLIT_SRC_BIT:                                                                return "VK_FORMAT_FEATURE_BLIT_SRC_BIT" ;
    case VK_FORMAT_FEATURE_BLIT_DST_BIT:                                                                return "VK_FORMAT_FEATURE_BLIT_DST_BIT" ;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT:                                             return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT" ;
    case VK_FORMAT_FEATURE_TRANSFER_SRC_BIT:                                                            return "VK_FORMAT_FEATURE_TRANSFER_SRC_BIT" ;
    case VK_FORMAT_FEATURE_TRANSFER_DST_BIT:                                                            return "VK_FORMAT_FEATURE_TRANSFER_DST_BIT" ;
    case VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT:                                                 return "VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT" ;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT:                            return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT" ;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT:           return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT" ;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT:           return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT" ;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT: return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT" ;
    case VK_FORMAT_FEATURE_DISJOINT_BIT:                                                                return "VK_FORMAT_FEATURE_DISJOINT_BIT" ;
    case VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT:                                                  return "VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT" ;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT:                                             return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT" ;
    case VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR:                                                 return "VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR" ;
    case VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR:                                                    return "VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR" ;
    case VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR:                                return "VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR" ;
    case VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT:                                          return "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT" ;
    case VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT:                                                return "VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT" ;
    case VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR:                                    return "VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR" ;
    case VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR:                                                  return "VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR" ;
    case VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR:                                                    return "VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR" ;
    default:                                                                                            return "Unknown Format Feature Bit" ;
    }
}


static char const *
dump_format_feature_flag_bits(
    VkFormatFeatureFlagBits const bits
)
{
    VkFormatFeatureFlagBits const all_bits[] =
    {
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
    ,   VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT
    ,   VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT
    ,   VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT
    ,   VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT
    ,   VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT
    ,   VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT
    ,   VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
    ,   VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT
    ,   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    ,   VK_FORMAT_FEATURE_BLIT_SRC_BIT
    ,   VK_FORMAT_FEATURE_BLIT_DST_BIT
    ,   VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
    ,   VK_FORMAT_FEATURE_TRANSFER_SRC_BIT
    ,   VK_FORMAT_FEATURE_TRANSFER_DST_BIT
    ,   VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT
    ,   VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT
    ,   VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT
    ,   VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT
    ,   VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT
    ,   VK_FORMAT_FEATURE_DISJOINT_BIT
    ,   VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT
    ,   VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT
    ,   VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR
    ,   VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR
    ,   VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR
    ,   VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_CUBIC_BIT_EXT
    ,   VK_FORMAT_FEATURE_FRAGMENT_DENSITY_MAP_BIT_EXT
    ,   VK_FORMAT_FEATURE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR
    ,   VK_FORMAT_FEATURE_VIDEO_ENCODE_INPUT_BIT_KHR
    ,   VK_FORMAT_FEATURE_VIDEO_ENCODE_DPB_BIT_KHR
    } ;
    uint32_t const all_bits_count = array_count(all_bits) ;

    static char dump_buffer[max_dump_buffer] = { 0 } ;
    SDL_memset(dump_buffer, 0, max_dump_buffer) ;
    size_t n = 0 ;

    for(
        uint32_t i = 0
    ;   i < all_bits_count
    ;   ++i
    )
    {
        if(bits & all_bits[i])
        {
            n = SDL_strlcat(dump_buffer, dump_format_feature_flag_bit(all_bits[i]), max_dump_buffer) ;
            require(n < max_dump_buffer) ;
            n = SDL_strlcat(dump_buffer, " | ", max_dump_buffer) ;
            require(n < max_dump_buffer) ;
        }
    }
    return dump_buffer ;
}


static void
dump_format_properties(
    VkFormatProperties const *  format_properties
)
{
    require(format_properties) ;

    // typedef struct VkFormatProperties {
    //     VkFormatFeatureFlags    linearTilingFeatures;
    //     VkFormatFeatureFlags    optimalTilingFeatures;
    //     VkFormatFeatureFlags    bufferFeatures;
    // } VkFormatProperties;

    log_debug_u32(format_properties->linearTilingFeatures) ;
    log_debug_str(dump_format_feature_flag_bits(format_properties->linearTilingFeatures)) ;
    log_debug_u32(format_properties->optimalTilingFeatures) ;
    log_debug_str(dump_format_feature_flag_bits(format_properties->optimalTilingFeatures)) ;
    log_debug_u32(format_properties->bufferFeatures) ;
    log_debug_str(dump_format_feature_flag_bits(format_properties->bufferFeatures)) ;
}



static bool
has_extension(
    VkExtensionProperties const *   haystack
,   uint32_t const                  haystack_count
,   char const *                    needle
)
{
    for(
        uint32_t i = 0
    ;   i < haystack_count
    ;   ++i
    )
    {
        require(haystack) ;
        require(needle) ;
        if(0 == SDL_strcmp(haystack[i].extensionName, needle))
        {
            return true ;
        }
    }
    return false ;
}


static bool
has_all_extensions(
    VkExtensionProperties const *   haystack
,   uint32_t const                  haystack_count
,   char const * const *            needles
,   uint32_t const                  needles_count
)
{
    uint32_t found_count = 0 ;
    for(
        uint32_t i = 0
    ;   i < needles_count
    ;   ++i
    )
    {
        require(needles) ;
        found_count += has_extension(haystack, haystack_count, needles[i]) ;
    }

    return needles_count == found_count ;
}


static void
add_to_extensions(
    char const **   extensions
,   uint32_t *      extensions_count
,   char const *    desired_extension
)
{
    require(*extensions_count < max_vulkan_desired_extensions) ;
    extensions[*extensions_count] = desired_extension ;
    ++*extensions_count ;
}


static void
dump_layer_property(
    VkLayerProperties const *   lp
)
{
    require(lp) ;
    // typedef struct VkLayerProperties {
    //     char        layerName[VK_MAX_EXTENSION_NAME_SIZE];
    //     uint32_t    specVersion;
    //     uint32_t    implementationVersion;
    //     char        description[VK_MAX_DESCRIPTION_SIZE];
    // } VkLayerProperties;
    log_debug_str(lp->layerName) ;
    log_debug_u32(lp->specVersion) ;
    log_debug_u32(lp->implementationVersion) ;
    log_debug_str(lp->description) ;
}


static void
dump_layer_properties(
    VkLayerProperties const *   lp
,   uint32_t const              lp_count
)
{
    log_debug_u32(lp_count) ;
    for(
        uint32_t i = 0
    ;   i < lp_count
    ;   ++i
    )
    {
        log_debug_u32(i) ;
        require(lp) ;
        dump_layer_property(&lp[i]) ;
    }
}


static bool
has_layer_property(
    VkLayerProperties const *   haystack
,   uint32_t const              haystack_count
,   char const *                needle
)
{
    for(
        uint32_t i = 0
    ;   i < haystack_count
    ;   ++i
    )
    {
        require(haystack) ;
        require(needle) ;
        if(0 == SDL_strcmp(haystack[i].layerName, needle))
        {
            return true ;
        }
    }
    return false ;
}


static bool
has_all_layer_properties(
    VkLayerProperties const *   haystack
,   uint32_t const              haystack_count
,   char const * const *        needles
,   uint32_t const              needles_count
)
{
    uint32_t found_count = 0 ;
    for(
        uint32_t i = 0
    ;   i < needles_count
    ;   ++i
    )
    {
        require(needles) ;
        found_count += has_layer_property(haystack, haystack_count, needles[i]) ;
    }

    return needles_count == found_count ;
}


static void
add_to_layers(
    char const **   layers
,   uint32_t *      layers_count
,   char const *    desired_layer
)
{
    require(*layers_count < max_vulkan_desired_layers) ;
    layers[*layers_count] = desired_layer ;
    ++*layers_count ;
}


static bool
create_instance_extensions_properties(
    VkExtensionProperties **    out_instance_extensions_properties
,   uint32_t *                  out_instance_extensions_properties_count
)
{
    require(out_instance_extensions_properties) ;
    require(out_instance_extensions_properties_count) ;

    begin_timed_block() ;

    // VkResult vkEnumerateInstanceExtensionProperties(
    //     const char*                                 pLayerName,
    //     uint32_t*                                   pPropertyCount,
    //     VkExtensionProperties*                      pProperties);
    if(check_vulkan(vkEnumerateInstanceExtensionProperties(
                NULL
            ,   out_instance_extensions_properties_count
            ,   NULL
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(*out_instance_extensions_properties = alloc_array(
                VkExtensionProperties
            ,   *out_instance_extensions_properties_count
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    //require(*out_instance_extensions_properties) ;

    if(check_vulkan(vkEnumerateInstanceExtensionProperties(
                NULL
            ,   out_instance_extensions_properties_count
            ,   *out_instance_extensions_properties
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_layer_properties(
    VkLayerProperties **    out_layer_properties
,   uint32_t *              out_layer_properties_count
)
{
    require(out_layer_properties) ;
    require(out_layer_properties_count) ;
    begin_timed_block() ;

    // VkResult vkEnumerateInstanceLayerProperties(
    //     uint32_t*                                   pPropertyCount,
    //     VkLayerProperties*                          pProperties);
    if(check_vulkan(
            vkEnumerateInstanceLayerProperties(
                out_layer_properties_count
            ,   NULL
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_layer_properties_count) ;

    if(check(*out_layer_properties = alloc_array(
                VkLayerProperties
            ,   *out_layer_properties_count
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    //require(*out_layer_properties) ;

    if(check_vulkan(
            vkEnumerateInstanceLayerProperties(
                out_layer_properties_count
            ,   *out_layer_properties
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_function_pointers(
    VkInstance          instance
,   vulkan_context *    vc
)
{
    require(instance) ;
    require(vc) ;

    vc->create_debug_messenger_func_ = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance
    ,   vk_create_debug_utils_messenger_ext_name
    ) ;

    require(vc->create_debug_messenger_func_) ;

    vc->destroy_debug_messenger_func_ = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
        instance
    ,   vk_destroy_debug_utils_messenger_ext_name
    ) ;

    require(vc->destroy_debug_messenger_func_) ;

    return true ;
}


static void
fill_debug_utils_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT *    out_dumcie
)
{
    require(out_dumcie) ;

    // typedef struct VkDebugUtilsMessengerCreateInfoEXT {
    //     VkStructureType                         sType;
    //     const void*                             pNext;
    //     VkDebugUtilsMessengerCreateFlagsEXT     flags;
    //     VkDebugUtilsMessageSeverityFlagsEXT     messageSeverity;
    //     VkDebugUtilsMessageTypeFlagsEXT         messageType;
    //     PFN_vkDebugUtilsMessengerCallbackEXT    pfnUserCallback;
    //     void*                                   pUserData;
    // } VkDebugUtilsMessengerCreateInfoEXT;
    out_dumcie->sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT ;
    out_dumcie->messageSeverity  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ;
    out_dumcie->messageType      = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ;
    out_dumcie->pfnUserCallback  = vulkan_debug_callback ;
    out_dumcie->pUserData        = NULL ;
}


static bool
create_debug_messenger(
    VkDebugUtilsMessengerEXT *          out_debug_messenger
,   VkInstance                          instance
,   PFN_vkCreateDebugUtilsMessengerEXT  create_func
)
{
    require(out_debug_messenger) ;
    require(instance) ;
    require(create_func) ;

    static VkDebugUtilsMessengerCreateInfoEXT dumcie = { 0 } ;
    fill_debug_utils_messenger_create_info(&dumcie) ;

    // VkResult vkCreateDebugUtilsMessengerEXT(
    //     VkInstance                                  instance,
    //     const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkDebugUtilsMessengerEXT*                   pMessenger);
    if(check_vulkan(create_func(
                instance
            ,   &dumcie
            ,   NULL
            ,   out_debug_messenger
            )
        )
    )
    {
        return false ;
    }

    return true ;
}


static void
destroy_debug_messenger(
    VkDebugUtilsMessengerEXT *          out_debug_messenger
,   VkInstance                          instance
,   PFN_vkDestroyDebugUtilsMessengerEXT destroy_func
)
{
    require(out_debug_messenger) ;
    require(instance) ;
    require(destroy_func) ;

    // void vkDestroyDebugUtilsMessengerEXT(
    //     VkInstance                                  instance,
    //     VkDebugUtilsMessengerEXT                    messenger,
    //     const VkAllocationCallbacks*                pAllocator);
    if(*out_debug_messenger)
    {
        destroy_func(instance, *out_debug_messenger, NULL) ;
        *out_debug_messenger = NULL ;
    }
}


static bool
create_vulkan_instance(
    VkInstance *            out_instance
,   char const * const *    desired_instance_extensions
,   uint32_t const          desired_instance_extensions_count
,   char const * const *    desired_layers
,   uint32_t const          desired_layers_count
,   VkBool32 const          enable_validation
)
{
    require(out_instance) ;
    begin_timed_block() ;


    // typedef struct VkApplicationInfo {
    //     VkStructureType    sType;
    //     const void*        pNext;
    //     const char*        pApplicationName;
    //     uint32_t           applicationVersion;
    //     const char*        pEngineName;
    //     uint32_t           engineVersion;
    //     uint32_t           apiVersion;
    // } VkApplicationInfo;
    static VkApplicationInfo   application_info = { 0 } ;
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO ;
    application_info.pNext              = NULL ;
    application_info.pApplicationName   = "threed" ;
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0) ;
    application_info.pEngineName        = "No Engine" ;
    application_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0) ;
    application_info.apiVersion         = VK_API_VERSION_1_0 ;


    static VkDebugUtilsMessengerCreateInfoEXT dumcie = { 0 } ;
    fill_debug_utils_messenger_create_info(&dumcie) ;

    // typedef struct VkInstanceCreateInfo {
    //     VkStructureType             sType;
    //     const void*                 pNext;
    //     VkInstanceCreateFlags       flags;
    //     const VkApplicationInfo*    pApplicationInfo;
    //     uint32_t                    enabledLayerCount;
    //     const char* const*          ppEnabledLayerNames;
    //     uint32_t                    enabledExtensionCount;
    //     const char* const*          ppEnabledExtensionNames;
    // } VkInstanceCreateInfo;
    static VkInstanceCreateInfo instance_create_info = { 0 } ;
    instance_create_info.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO ;
    instance_create_info.pNext                      = NULL ;
    if(enable_validation)
    {
    instance_create_info.pNext                      = (VkDebugUtilsMessengerCreateInfoEXT *)&dumcie ;
    }
    instance_create_info.flags                      = 0 ;
    instance_create_info.pApplicationInfo           = &application_info ;
    instance_create_info.enabledLayerCount          = 0 ;
    instance_create_info.ppEnabledLayerNames        = NULL ;
    if(enable_validation)
    {
    instance_create_info.enabledLayerCount          = desired_layers_count ;
    instance_create_info.ppEnabledLayerNames        = desired_layers ;
    }
    instance_create_info.enabledExtensionCount      = desired_instance_extensions_count ;
    instance_create_info.ppEnabledExtensionNames    = desired_instance_extensions ;


    // VkResult vkCreateInstance(
    //     const VkInstanceCreateInfo*                 pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkInstance*                                 pInstance);
    if(check_vulkan(vkCreateInstance(
                &instance_create_info
            ,   NULL
            ,   out_instance
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_instance) ;

    end_timed_block() ;
    return true ;
}


static bool
create_surface(
    VkSurfaceKHR *  out_surface
,   VkInstance      instance
)
{
    require(out_surface) ;
    require(instance) ;
    require(app_) ;
    require(app_->window_) ;

    if(check_sdl(SDL_TRUE == SDL_Vulkan_CreateSurface(
                app_->window_
            ,   instance
            ,   NULL
            ,   out_surface
            )
        )
    )
    {
        return false ;
    }
    require(*out_surface) ;
    return true ;
}

static bool
create_physical_devices(
    VkPhysicalDevice *  out_physical_devices
,   uint32_t *          out_physical_devices_count
,   VkInstance          instance
)
{
    require(out_physical_devices) ;
    require(out_physical_devices_count) ;
    require(instance) ;

    begin_timed_block() ;

    // VkResult vkEnumeratePhysicalDevices(
    //     VkInstance                                  instance,
    //     uint32_t*                                   pPhysicalDeviceCount,
    //     VkPhysicalDevice*                           pPhysicalDevices);
    if(check_vulkan(vkEnumeratePhysicalDevices(
                instance
            ,   out_physical_devices_count
            ,   NULL
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    require(*out_physical_devices_count < max_vulkan_physical_devices) ;

    *out_physical_devices_count = min_u32(*out_physical_devices_count, max_vulkan_physical_devices) ;

    VkResult const res = vkEnumeratePhysicalDevices(
        instance
    ,   out_physical_devices_count
    ,   out_physical_devices
    ) ;
    if(check(res == VK_SUCCESS || res == VK_INCOMPLETE))
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
is_queue_family_complete(
    vulkan_queue_family_indices const * queue_family_indices
)
{
    require(queue_family_indices) ;

    return
        queue_family_indices->graphics_family_valid_
    &&  queue_family_indices->present_family_valid_
        ;
}


static bool
find_queue_families(
    vulkan_queue_family_indices *   out_queue_family_indices
,   VkPhysicalDevice const          physical_device
,   VkQueueFamilyProperties const * queue_family_properties
,   uint32_t const                  queue_family_properties_count
,   VkSurfaceKHR const              surface
)
{
    require(out_queue_family_indices) ;
    require(physical_device) ;
    require(queue_family_properties) ;
    require(surface) ;

    SDL_memset(out_queue_family_indices, 0, sizeof(vulkan_queue_family_indices)) ;

    for(
        uint32_t i = 0
    ;   i < queue_family_properties_count
    ;   ++i
    )
    {
        VkQueueFamilyProperties const * qfp = &queue_family_properties[i] ;
        if(qfp->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            out_queue_family_indices->graphics_family_          = i ;
            out_queue_family_indices->graphics_family_valid_    = true ;
        }

        VkBool32 present_support = VK_FALSE ;
        VkResult const present_support_okay = vkGetPhysicalDeviceSurfaceSupportKHR(
            physical_device
        ,   i
        ,   surface
        ,   &present_support
        ) ;

        if(VK_SUCCESS == present_support_okay)
        {
            if(present_support)
            {
                out_queue_family_indices->present_family_          = i ;
                out_queue_family_indices->present_family_valid_    = true ;
            }
        }
        else
        {
            log_debug(
                "Error during vkGetPhysicalDeviceSurfaceSupportKHR queue_index=%d, device=%p, surface=%p, VkResult=%d"
            ,   i
            ,   physical_device
            ,   surface
            ,   present_support_okay
            ) ;
        }

        if(is_queue_family_complete(out_queue_family_indices))
        {
            return true ;
        }
    }

    log_debug("Physical Device %p does not support queue families for surface %p"
        , physical_device
        , surface
    ) ;

    return false ;
}


static void
add_to_unique_queue_families_indices(
    uint32_t *      unique_queue_families_indices
,   uint32_t *      unique_queue_families_indices_count
,   uint32_t const  unique_queue_families_index
)
{
    require(unique_queue_families_indices) ;
    require(unique_queue_families_indices_count) ;
    require(*unique_queue_families_indices_count < max_vulkan_unique_queue_family_indices) ;

    for(
        uint32_t i = 0
    ;   i < *unique_queue_families_indices_count
    ;   ++i
    )
    {
        if(unique_queue_families_index == unique_queue_families_indices[i])
        {
            return ;
        }
    }
    unique_queue_families_indices[(*unique_queue_families_indices_count)++] = unique_queue_families_index ;
}


static void
add_to_desired_device_extension(
    char const **           desired_device_extensions
,   uint32_t *              desired_device_extensions_count
,   char const *            desired_extension
)
{
    require(desired_device_extensions) ;
    require(desired_device_extensions_count) ;
    require(*desired_device_extensions_count < max_vulkan_desired_device_extensions) ;
    require(desired_extension) ;
    require(*desired_extension) ;

    desired_device_extensions[(*desired_device_extensions_count)++] = desired_extension ;
}


static VkSampleCountFlagBits
get_max_usuable_sample_count(
    VkPhysicalDeviceProperties const * pdp
)
{
    require(pdp) ;

    VkSampleCountFlagBits const counts =
        pdp->limits.framebufferColorSampleCounts
    &   pdp->limits.framebufferDepthSampleCounts
    ;

    if(counts & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT ;
    }

    if(counts & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT ;
    }

    if(counts & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT ;
    }

    if(counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT ;
    }

    if(counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT ;
    }

    if(counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT ;
    }

    return VK_SAMPLE_COUNT_1_BIT ;
}


static bool
create_swapchain_support_details(
    vulkan_swapchain_support_details *  out_scsd
,   VkPhysicalDevice const              physical_device
,   VkSurfaceKHR const                  surface
)
{
    require(out_scsd) ;
    require(physical_device) ;
    require(surface) ;

    out_scsd->physical_device_ = physical_device ;
    out_scsd->surface_ = surface ;

    // VkResult vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkSurfaceKHR                                surface,
    //     VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities);
    if(check_vulkan(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                physical_device
            ,   surface
            ,   &out_scsd->capabilities_
            )
        )
    )
    {
        return false ;
    }


    // VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkSurfaceKHR                                surface,
    //     uint32_t*                                   pSurfaceFormatCount,
    //     VkSurfaceFormatKHR*                         pSurfaceFormats);
    if(check_vulkan(vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device
            ,   surface
            ,   &out_scsd->formats_count_
            ,   NULL
            )
        )
    )
    {
        return false ;
    }

    require(out_scsd->formats_count_ < max_vulkan_surface_formats) ;
    out_scsd->formats_count_ = min_u32(out_scsd->formats_count_, max_vulkan_surface_formats) ;

    if(check_vulkan(vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device
            ,   surface
            ,   &out_scsd->formats_count_
            ,   out_scsd->formats_
            )
        )
    )
    {
        return false ;
    }


    for(
        uint32_t i = 0
    ;   i < out_scsd->formats_count_
    ;   ++i
    )
    {
        // void vkGetPhysicalDeviceFormatProperties(
        //     VkPhysicalDevice                            physicalDevice,
        //     VkFormat                                    format,
        //     VkFormatProperties*                         pFormatProperties);
        //     vkGetPhysicalDeviceFormatProperties()
        vkGetPhysicalDeviceFormatProperties(
            physical_device
        ,   out_scsd->formats_[i].format
        ,   &out_scsd->formats_properties_[i]
        ) ;
    }


    // VkResult vkGetPhysicalDeviceSurfacePresentModesKHR(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkSurfaceKHR                                surface,
    //     uint32_t*                                   pPresentModeCount,
    //     VkPresentModeKHR*                           pPresentModes);
    if(check_vulkan(vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device
            ,   surface
            ,   &out_scsd->modes_count_
            ,   NULL
            )
        )
    )
    {
        return false ;
    }

    require(out_scsd->modes_count_ < max_vulkan_present_modes) ;
    out_scsd->modes_count_ = min_u32(out_scsd->modes_count_, max_vulkan_present_modes) ;

    if(check_vulkan(vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device
            ,   surface
            ,   &out_scsd->modes_count_
            ,   out_scsd->modes_
            )
        )
    )
    {
        return false ;
    }

    return true ;
}


static bool
check_if_swapchain_support_is_adequate(
    vulkan_swapchain_support_details const * scsd
)
{
    require(scsd) ;
    bool const is_adequate =
        boolify(scsd->formats_count_)
    &&  boolify(scsd->modes_count_)
    ;

    return is_adequate ;
}


static void
create_desired_format_properties(
    vulkan_desired_format_properties *  out_desired_format_properties
,   uint32_t *                          out_desired_format_properties_count
,   VkPhysicalDevice const              physical_device
)
{
    require(out_desired_format_properties) ;
    require(out_desired_format_properties_count) ;
    require(physical_device) ;

    *out_desired_format_properties_count = desired_formats_count ;
    for(
        uint32_t i = 0
    ;   i < desired_formats_count
    ;   ++i
    )
    {
        out_desired_format_properties[i].format_ = desired_formats[i] ;
        vkGetPhysicalDeviceFormatProperties(
            physical_device
        ,   out_desired_format_properties[i].format_
        ,   &out_desired_format_properties[i].properties_
        ) ;
    }
}


static bool
find_format_properties(
    VkFormatProperties *                        out_format_properties
,   vulkan_desired_format_properties const *    desired_format_properties
,   uint32_t const                              desired_format_properties_count
,   VkFormat const                              format
)
{
    require(out_format_properties) ;
    require(desired_format_properties) ;
    for(
        uint32_t i = 0
    ;   i < desired_format_properties_count
    ;   ++i
    )
    {
        if(format == desired_format_properties[i].format_)
        {
            *out_format_properties = desired_format_properties[i].properties_ ;
            return true ;
        }
    }

    return false ;
}


static bool
find_supported_format(
    VkFormat *                                  out_format
,   vulkan_desired_format_properties const *    desired_format_properties
,   uint32_t const                              desired_format_properties_count
,   VkFormat const *                            candidates
,   uint32_t const                              candidates_count
,   VkImageTiling const                         tiling
,   VkFormatFeatureFlags const                  features
)
{
    require(out_format) ;
    require(desired_format_properties) ;
    require(candidates) ;

    for(
        uint32_t i = 0
    ;   i < candidates_count
    ;   ++i
    )
    {
        VkFormatProperties props = { 0 } ;
        VkFormat const format = candidates[i] ;

        if(check(find_format_properties(
                    &props
                ,   desired_format_properties
                ,   desired_format_properties_count
                ,   format
                )
            )
        )
        {
            return false ;
        }

        if(
            tiling == VK_IMAGE_TILING_LINEAR
        &&  (props.linearTilingFeatures & features) == features
        )
        {
            *out_format = format ;
            return true ;
        }
        else if(
            tiling == VK_IMAGE_TILING_OPTIMAL
        &&  (props.optimalTilingFeatures & features) == features
        )
        {
            *out_format = format ;
            return true ;
        }
    }

    require(0) ;
    return false ;
}


static bool
find_depth_format(
    VkFormat *                                  out_format
,   vulkan_desired_format_properties const *    desired_format_properties
,   uint32_t const                              desired_format_properties_count
)
{
    require(out_format) ;
    require(desired_format_properties) ;

    static VkFormat const candidates[] =
    {
        VK_FORMAT_D32_SFLOAT
    ,   VK_FORMAT_D32_SFLOAT_S8_UINT
    ,   VK_FORMAT_D24_UNORM_S8_UINT
    } ;
    static uint32_t const candidates_count = array_count(candidates) ;

    VkFormat found_format = 0 ;

    if(check(find_supported_format(
                &found_format
            ,   desired_format_properties
            ,   desired_format_properties_count
            ,   candidates
            ,   candidates_count
            ,   VK_IMAGE_TILING_OPTIMAL
            ,   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            )
        )
    )
    {
        return false ;
    }

    *out_format = found_format ;
    return true ;
}


static bool
has_stencil_component(
    VkFormat const format
)
{
    return
        format == VK_FORMAT_D32_SFLOAT_S8_UINT
    ||  format == VK_FORMAT_D24_UNORM_S8_UINT
    ;
}


static bool
fill_physical_device_info(
    vulkan_physical_device_info *   out_physical_device_info
,   VkPhysicalDevice                physical_device
,   VkSurfaceKHR const              surface
)
{
    require(out_physical_device_info) ;
    require(physical_device) ;

    out_physical_device_info->device_ = physical_device ;

    // void vkGetPhysicalDeviceProperties(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkPhysicalDeviceProperties*                 pProperties);
    vkGetPhysicalDeviceProperties(
        physical_device
    ,   &out_physical_device_info->properties_
    ) ;


    // void vkGetPhysicalDeviceFeatures(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkPhysicalDeviceFeatures*                   pFeatures);
    vkGetPhysicalDeviceFeatures(
        physical_device
    ,   &out_physical_device_info->features_
    ) ;

    // void vkGetPhysicalDeviceMemoryProperties(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkPhysicalDeviceMemoryProperties*           pMemoryProperties);
    vkGetPhysicalDeviceMemoryProperties(
        physical_device
    ,   &out_physical_device_info->memory_properties_
    ) ;

    // void vkGetPhysicalDeviceQueueFamilyProperties(
    //     VkPhysicalDevice                            physicalDevice,
    //     uint32_t*                                   pQueueFamilyPropertyCount,
    //     VkQueueFamilyProperties*                    pQueueFamilyProperties);
    //     vkGetPhysicalDeviceQueueFamilyProperties()
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device
    ,   &out_physical_device_info->queue_family_properties_count_
    ,   NULL
    ) ;

    log_debug_u32(out_physical_device_info->queue_family_properties_count_) ;
    require(out_physical_device_info->queue_family_properties_count_ < max_vulkan_queue_family_properties) ;
    out_physical_device_info->queue_family_properties_count_ = min_u32(
        out_physical_device_info->queue_family_properties_count_
    ,   max_vulkan_queue_family_properties
    ) ;

    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device
    ,   &out_physical_device_info->queue_family_properties_count_
    ,   out_physical_device_info->queue_family_properties_
    ) ;


    // VkResult vkEnumerateDeviceExtensionProperties(
    //     VkPhysicalDevice                            physicalDevice,
    //     const char*                                 pLayerName,
    //     uint32_t*                                   pPropertyCount,
    //     VkExtensionProperties*                      pProperties);
    if(check_vulkan(vkEnumerateDeviceExtensionProperties(
                physical_device
            ,   NULL
            ,   &out_physical_device_info->device_extensions_count_
            ,   NULL
            )
        )
    )
    {
        return false ;
    }

    log_debug_u32(sizeof(VkExtensionProperties)) ;
    log_debug_u32(out_physical_device_info->device_extensions_count_) ;
    require(out_physical_device_info->device_extensions_count_ < max_vulkan_device_extensions)
    out_physical_device_info->device_extensions_count_ = min_u32(out_physical_device_info->device_extensions_count_, max_vulkan_device_extensions) ;

    if(check_vulkan(vkEnumerateDeviceExtensionProperties(
                physical_device
            ,   NULL
            ,   &out_physical_device_info->device_extensions_count_
            ,   out_physical_device_info->device_extensions_
            )
        )
    )
    {
        return false ;
    }

    out_physical_device_info->queue_families_indices_complete_ = find_queue_families(
        &out_physical_device_info->queue_families_indices_
    ,   physical_device
    ,   out_physical_device_info->queue_family_properties_
    ,   out_physical_device_info->queue_family_properties_count_
    ,   surface
    ) ;

    if(out_physical_device_info->queue_families_indices_complete_)
    {
        add_to_unique_queue_families_indices(
            out_physical_device_info->unique_queue_families_indices_
        ,   &out_physical_device_info->unique_queue_families_indices_count_
        ,   out_physical_device_info->queue_families_indices_.graphics_family_
        ) ;

        add_to_unique_queue_families_indices(
            out_physical_device_info->unique_queue_families_indices_
        ,   &out_physical_device_info->unique_queue_families_indices_count_
        ,   out_physical_device_info->queue_families_indices_.present_family_
        ) ;
    }

    add_to_desired_device_extension(
        out_physical_device_info->desired_device_extensions_
    ,   &out_physical_device_info->desired_device_extensions_count_
    ,   vk_khr_swapchain_extension_name
    ) ;


    out_physical_device_info->desired_device_extensions_okay_ = has_all_extensions(
        out_physical_device_info->device_extensions_
    ,   out_physical_device_info->device_extensions_count_
    ,   out_physical_device_info->desired_device_extensions_
    ,   out_physical_device_info->desired_device_extensions_count_
    ) ;

    out_physical_device_info->max_usable_sample_count_ = get_max_usuable_sample_count(
        &out_physical_device_info->properties_
    ) ;

    out_physical_device_info->sample_count_ = VK_SAMPLE_COUNT_1_BIT ;

    if(check(create_swapchain_support_details(
                &out_physical_device_info->swapchain_support_details_
            ,   physical_device
            ,   surface
            )
        )
    )
    {
        return false ;
    }

    out_physical_device_info->swapchain_support_details_okay_ = check_if_swapchain_support_is_adequate(
        &out_physical_device_info->swapchain_support_details_
    ) ;

    create_desired_format_properties(
        out_physical_device_info->desired_format_properties_
    ,   &out_physical_device_info->desired_format_properties_count_
    ,   physical_device
    ) ;

    if(check(find_depth_format(
                &out_physical_device_info->depth_format_
            ,   out_physical_device_info->desired_format_properties_
            ,   out_physical_device_info->desired_format_properties_count_
            )
        )
    )
    {
        return false ;
    }

    return true ;
}


// static void
// rate_device_suitability(
//     uint32_t *                          out_score
// ,   VkPhysicalDeviceProperties const *  properties
// ,   VkPhysicalDeviceFeatures const *    features
// )
// {
//     require(out_score) ;
//     require(properties) ;
//     require(features) ;

//     uint32_t score = 0 ;

//     *out_score = score ;

//     if(!features->geometryShader)
//     {
//         return ;
//     }

//     if(properties->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
//     {
//         score += 1000 ;
//     }

//     score += properties->limits.maxImageDimension2D ;

//     *out_score = score ;
// }



static void
fill_physical_devices_info(
    vulkan_physical_device_info *   out_physical_device_info
,   VkPhysicalDevice *              devices
,   uint32_t const                  devices_count
,   VkSurfaceKHR const              surface
)
{
    require(out_physical_device_info) ;
    require(devices) ;

    for(
        uint32_t i = 0
    ;   i < devices_count
    ;   ++i
    )
    {
        log_debug_u32(i) ;

        if(check(fill_physical_device_info(
                    &out_physical_device_info[i]
                ,   devices[i]
                ,   surface
                )
            )
        )
        {
            require(0) ;
        }
    }

}


static void
dump_physical_device_info(
    vulkan_physical_device_info const * physical_device_info
,   uint32_t const                      physical_device_info_count
)
{
    for(
        uint32_t i = 0
    ;   i < physical_device_info_count
    ;   ++i
    )
    {
        vulkan_physical_device_info const * pdi = &physical_device_info[i] ;
        dump_physical_device_properties(&pdi->properties_) ;
        dump_physical_device_features(&pdi->features_) ;
        dump_physical_device_memory_properties(&pdi->memory_properties_) ;
        dump_queue_family_properties(pdi->queue_family_properties_, pdi->queue_family_properties_count_) ;
        dump_extension_properties(pdi->device_extensions_, pdi->device_extensions_count_) ;
        log_debug_u32(pdi->queue_families_indices_complete_) ;

        log_debug_u32(pdi->unique_queue_families_indices_count_) ;
        for(
            uint32_t j = 0
        ;   j < pdi->unique_queue_families_indices_count_
        ;   ++j
        )
        {
            log_debug_u32(j) ;
            log_debug_u32(pdi->unique_queue_families_indices_[j]) ;
        }

        log_debug_u32(pdi->desired_device_extensions_okay_) ;
        log_debug_u32(pdi->desired_device_extensions_count_) ;
        log_debug_u32(pdi->max_usable_sample_count_) ;
        log_debug_u32(pdi->swapchain_support_details_.formats_count_) ;
        log_debug_u32(pdi->swapchain_support_details_.modes_count_) ;

        dump_surface_capabilities(&pdi->swapchain_support_details_.capabilities_) ;

        for(
            uint32_t j = 0
        ;   j < pdi->swapchain_support_details_.formats_count_
        ;   ++j
        )
        {
            log_debug_u32(j) ;
            dump_surface_format_khr(&pdi->swapchain_support_details_.formats_[j]) ;
            dump_format_properties(&pdi->swapchain_support_details_.formats_properties_[j]) ;
        }

        for(
            uint32_t j = 0
        ;   j < pdi->swapchain_support_details_.modes_count_
        ;   ++j
        )
        {
            log_debug_u32(j) ;
            log_debug_str(dump_present_mode_khr(pdi->swapchain_support_details_.modes_[j])) ;
        }

        log_debug_u32(pdi->desired_format_properties_count_) ;
        for(
            uint32_t j = 0
        ;   j < pdi->desired_format_properties_count_
        ;   ++j
        )
        {
            log_debug_u32(j) ;
            dump_format_properties(&pdi->desired_format_properties_[j].properties_) ;
        }

        log_debug_u32(pdi->depth_format_) ;
        log_debug_str(dump_vk_format(pdi->depth_format_)) ;

    }
}



static bool
is_physical_device_suiteable(
    vulkan_physical_device_info const * physical_device
)
{
    require(physical_device) ;

    VkPhysicalDeviceProperties const *  pdp = &physical_device->properties_ ;
    VkPhysicalDeviceFeatures const *    pdf = &physical_device->features_ ;

    bool const type_ok      = pdp->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || true ;
    bool const shader_ok    = boolify(pdf->geometryShader) ;
    bool const aniso_ok     = boolify(pdf->samplerAnisotropy) ;
    bool const qf_ok        = boolify(physical_device->queue_families_indices_complete_) ;
    bool const ext_ok       = boolify(physical_device->desired_device_extensions_okay_) ;
    bool const swap_ok      = boolify(physical_device->swapchain_support_details_okay_) ;

    bool const all_ok =
        type_ok
    &&  shader_ok
    &&  aniso_ok
    &&  qf_ok
    &&  ext_ok
    &&  swap_ok
    ;

    return all_ok ;
}


static bool
pick_physical_device(
    vulkan_physical_device_info **      out_physical_device
,   vulkan_physical_device_info *       physical_devices
,   uint32_t const                      physical_devices_count
)
{
    require(out_physical_device) ;
    require(physical_devices) ;
    require(physical_devices_count) ;

    for(
        uint32_t i = 0
    ;   i < physical_devices_count
    ;   ++i
    )
    {
        vulkan_physical_device_info * pd = &physical_devices[i] ;
        if(is_physical_device_suiteable(pd))
        {
            *out_physical_device = pd ;
            log_debug("picking device %s (%p) max_msaa_samples=%d", pd->properties_.deviceName, pd->device_, pd->max_usable_sample_count_) ;
            return true ;
        }
    }

    *out_physical_device = NULL ;
    require(0) ;
    return false ;

}


static bool
create_logical_device(
    VkDevice *                          out_device
,   VkPhysicalDevice const              physical_device
,   VkPhysicalDeviceFeatures const *    physical_device_features
,   uint32_t const *                    unique_queue_family_indices
,   uint32_t const                      unique_queue_family_indices_count
,   char const * const *                desired_device_extensions
,   uint32_t const                      desired_device_extensions_count
,   char const * const *                desired_instance_layers
,   uint32_t const                      desired_instance_layers_count
,   VkBool32 const                      enable_validation
)
{
    require(out_device) ;
    require(physical_device) ;
    require(physical_device_features) ;
    require(unique_queue_family_indices) ;
    require(unique_queue_family_indices_count) ;
    require(desired_device_extensions) ;
    require(desired_device_extensions_count) ;
    require(desired_instance_layers) ;
    require(desired_instance_layers_count) ;


    begin_timed_block() ;

    static float queue_priority = 1.0 ;

    // typedef struct VkDeviceQueueCreateInfo {
    //     VkStructureType             sType;
    //     const void*                 pNext;
    //     VkDeviceQueueCreateFlags    flags;
    //     uint32_t                    queueFamilyIndex;
    //     uint32_t                    queueCount;
    //     const float*                pQueuePriorities;
    // } VkDeviceQueueCreateInfo;
    static VkDeviceQueueCreateInfo  dqci[max_vulkan_unique_queue_family_indices] = { 0 } ;
    for(
        uint32_t i = 0
    ;   i < unique_queue_family_indices_count
    ;   ++i
    )
    {
        dqci[i].sType              = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO ;
        dqci[i].pNext              = NULL ;
        dqci[i].flags              = 0 ;
        dqci[i].queueFamilyIndex   = unique_queue_family_indices[i] ;
        dqci[i].queueCount         = 1 ;
        dqci[i].pQueuePriorities   = &queue_priority ;
    }


    // typedef struct VkDeviceCreateInfo {
    //     VkStructureType                    sType;
    //     const void*                        pNext;
    //     VkDeviceCreateFlags                flags;
    //     uint32_t                           queueCreateInfoCount;
    //     const VkDeviceQueueCreateInfo*     pQueueCreateInfos;
    //     uint32_t                           enabledLayerCount;
    //     const char* const*                 ppEnabledLayerNames;
    //     uint32_t                           enabledExtensionCount;
    //     const char* const*                 ppEnabledExtensionNames;
    //     const VkPhysicalDeviceFeatures*    pEnabledFeatures;
    // } VkDeviceCreateInfo;
    static VkDeviceCreateInfo dci = { 0 } ;
    dci.sType                       = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO ;
    dci.pNext                       = NULL ;
    dci.flags                       = 0 ;
    dci.queueCreateInfoCount        = unique_queue_family_indices_count ;
    dci.pQueueCreateInfos           = dqci ;
    dci.enabledLayerCount           = 0 ;
    dci.ppEnabledLayerNames         = NULL ;
    if(enable_validation)
    {
    dci.enabledLayerCount           = desired_instance_layers_count ;
    dci.ppEnabledLayerNames         = desired_instance_layers ;
    }
    dci.enabledExtensionCount       = desired_device_extensions_count ;
    dci.ppEnabledExtensionNames     = desired_device_extensions ;
    dci.pEnabledFeatures            = physical_device_features ;


    // VkResult vkCreateDevice(
    //     VkPhysicalDevice                            physicalDevice,
    //     const VkDeviceCreateInfo*                   pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkDevice*                                   pDevice);
    if(check_vulkan(vkCreateDevice(
                physical_device
            ,   &dci
            ,   NULL
            ,   out_device
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    require(out_device) ;

    end_timed_block() ;
    return true ;
}


static bool
create_queues(
    VkQueue *       out_graphics_queue
,   VkQueue *       out_present_queue
,   VkDevice const  device
,   uint32_t        graphics_family
,   uint32_t        present_family
)
{
    require(out_graphics_queue) ;
    require(out_present_queue) ;
    require(device) ;
    begin_timed_block() ;

    // void vkGetDeviceQueue(
    //     VkDevice                                    device,
    //     uint32_t                                    queueFamilyIndex,
    //     uint32_t                                    queueIndex,
    //     VkQueue*                                    pQueue);
    vkGetDeviceQueue(
        device
    ,   graphics_family
    ,   0
    ,   out_graphics_queue
    ) ;
    require(out_graphics_queue) ;

    vkGetDeviceQueue(
        device
    ,   present_family
    ,   0
    ,   out_present_queue
    ) ;
    require(out_present_queue) ;

    end_timed_block() ;
    return true ;
}


static bool
create_framebuffers(
    vulkan_context *    vc
)
{
    require(vc) ;
    require(vc->device_) ;
    begin_timed_block() ;
    require(vc->swapchain_images_count_ < max_vulkan_swapchain_images) ;
    static VkImageView  attachments[3] = { 0 } ;
    static uint32_t     attachments_count = 0 ;

    for(
        uint32_t i = 0
    ;   i < vc->swapchain_images_count_
    ;   ++i
    )
    {
        if(vc->enable_sampling_)
        {
            attachments[0] = vc->color_image_view_ ;
            attachments[1] = vc->depth_image_view_ ;
            attachments[2] = vc->swapchain_views_[i] ;
            attachments_count = 3 ;
            require(attachments[0]) ;
            require(attachments[1]) ;
            require(attachments[2]) ;
        }
        else
        {
            attachments[0] = vc->swapchain_views_[i] ;
            attachments[1] = vc->depth_image_view_ ;
            attachments_count = 2 ;
            require(attachments[0]) ;
            require(attachments[1]) ;
        }

        // typedef struct VkFramebufferCreateInfo {
        //     VkStructureType             sType;
        //     const void*                 pNext;
        //     VkFramebufferCreateFlags    flags;
        //     VkRenderPass                renderPass;
        //     uint32_t                    attachmentCount;
        //     const VkImageView*          pAttachments;
        //     uint32_t                    width;
        //     uint32_t                    height;
        //     uint32_t                    layers;
        // } VkFramebufferCreateInfo;
        VkFramebufferCreateInfo fbci = { 0 } ;
        fbci.sType              = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO ;
        fbci.pNext              = NULL ;
        fbci.flags              = 0 ;
        fbci.renderPass         = vc->render_pass_ ;
        fbci.attachmentCount    = attachments_count ;
        fbci.pAttachments       = attachments ;
        fbci.width              = vc->swapchain_extent_.width ;
        fbci.height             = vc->swapchain_extent_.height ;
        fbci.layers             = 1 ;

        // VkResult vkCreateFramebuffer(
        //     VkDevice                                    device,
        //     const VkFramebufferCreateInfo*              pCreateInfo,
        //     const VkAllocationCallbacks*                pAllocator,
        //     VkFramebuffer*                              pFramebuffer);
        if(check_vulkan(vkCreateFramebuffer(
                    vc->device_
                ,   &fbci
                ,   NULL
                ,   &vc->framebuffers_[i]
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
        require(vc->framebuffers_[i]) ;
    }

    end_timed_block() ;
    return true ;

}


static VkSurfaceFormatKHR
choose_swapchain_surface_format(
    VkSurfaceFormatKHR const *  surface_formats
,   uint32_t const              surface_formats_count
)
{
    require(surface_formats) ;
    require(surface_formats_count) ;

    for(
        uint32_t i = 0
    ;   i < surface_formats_count
    ;   ++i
    )
    {
        bool const format_ok =
            VK_FORMAT_B8G8R8A8_SRGB == surface_formats[i].format ;

        bool const color_ok =
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == surface_formats[i].colorSpace ;

        bool const all_ok =
            format_ok
        &&  color_ok
        ;

        if(all_ok)
        {
            return surface_formats[i] ;
        }
    }

    return surface_formats[0] ;
}


static VkPresentModeKHR
choose_swapchain_present_mode(
    VkPresentModeKHR const *    present_modes
,   uint32_t const              present_modes_count
)
{
    require(present_modes) ;
    require(present_modes_count) ;

    for(
        uint32_t i = 0
    ;   i < present_modes_count
    ;   ++i
    )
    {
        bool const mode_ok =
            //present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR ;
            //present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR ;
            //present_modes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR ;
            present_modes[i] == VK_PRESENT_MODE_FIFO_KHR ;

        if(mode_ok)
        {
            return present_modes[i] ;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR ;
}


static VkExtent2D
choose_swapchain_extent(
    VkSurfaceCapabilitiesKHR const *    surface_caps
)
{
    require(surface_caps) ;
    require(surface_caps->currentExtent.width < 0xffff) ;
    require(surface_caps->currentExtent.width <= surface_caps->maxImageExtent.width) ;
    require(surface_caps->currentExtent.width >= surface_caps->minImageExtent.width) ;
    require(surface_caps->currentExtent.height < 0xffff) ;
    require(surface_caps->currentExtent.height <= surface_caps->maxImageExtent.height) ;
    require(surface_caps->currentExtent.height >= surface_caps->minImageExtent.height) ;
    return surface_caps->currentExtent ;
}


static bool
create_swapchain(
    VkSwapchainKHR *                            out_swapchain
,   VkSurfaceFormatKHR *                        out_surface_format
,   VkPresentModeKHR *                          out_present_mode
,   VkExtent2D *                                out_extent
,   VkImage *                                   out_images
,   uint32_t *                                  out_image_count
,   VkDevice const                              device
,   VkSurfaceKHR const                          surface
,   vulkan_swapchain_support_details const *    scsd
,   vulkan_queue_family_indices const *         qfi
,   uint32_t                                    desired_image_count
)
{
    require(out_swapchain) ;
    require(out_surface_format) ;
    require(out_present_mode) ;
    require(out_extent) ;
    require(out_image_count) ;
    require(device) ;
    require(surface) ;
    require(scsd) ;
    require(qfi) ;
    require(desired_image_count < max_vulkan_swapchain_images) ;
    begin_timed_block() ;

    static VkSurfaceCapabilitiesKHR caps = { 0 } ;

    require(scsd->physical_device_) ;
    if(check_vulkan(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                scsd->physical_device_
            ,   surface
            ,   &caps
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    *out_surface_format = choose_swapchain_surface_format(
        scsd->formats_
    ,   scsd->formats_count_
    ) ;

    *out_present_mode = choose_swapchain_present_mode(
        scsd->modes_
    ,   scsd->modes_count_
    ) ;

    *out_extent = choose_swapchain_extent(
        &caps
    ) ;

    log_debug("swapchain extent (%d, %d)", out_extent->width, out_extent->height) ;

    // 0 == scsd->capabilities_.maxImageCount == no maximum.
    if(0 == scsd->capabilities_.maxImageCount)
    {
        desired_image_count = max_u32(desired_image_count, scsd->capabilities_.minImageCount) ;
    }
    else
    {
        require(scsd->capabilities_.minImageCount <= scsd->capabilities_.maxImageCount) ;
        desired_image_count = clamp_u32(desired_image_count, scsd->capabilities_.minImageCount, scsd->capabilities_.maxImageCount) ;
        require(desired_image_count <= scsd->capabilities_.maxImageCount) ;
    }
    require(desired_image_count >= scsd->capabilities_.minImageCount) ;
    require(desired_image_count < max_vulkan_swapchain_images) ;


    uint32_t queue_family_indices[max_vulkan_unique_queue_family_indices] = { 0 } ;
    require(2 < max_vulkan_unique_queue_family_indices) ;
    queue_family_indices[0] = qfi->graphics_family_ ;
    queue_family_indices[1] = qfi->present_family_ ;


    // typedef struct VkSwapchainCreateInfoKHR {
    //     VkStructureType                  sType;
    //     const void*                      pNext;
    //     VkSwapchainCreateFlagsKHR        flags;
    //     VkSurfaceKHR                     surface;
    //     uint32_t                         minImageCount;
    //     VkFormat                         imageFormat;
    //     VkColorSpaceKHR                  imageColorSpace;
    //     VkExtent2D                       imageExtent;
    //     uint32_t                         imageArrayLayers;
    //     VkImageUsageFlags                imageUsage;
    //     VkSharingMode                    imageSharingMode;
    //     uint32_t                         queueFamilyIndexCount;
    //     const uint32_t*                  pQueueFamilyIndices;
    //     VkSurfaceTransformFlagBitsKHR    preTransform;
    //     VkCompositeAlphaFlagBitsKHR      compositeAlpha;
    //     VkPresentModeKHR                 presentMode;
    //     VkBool32                         clipped;
    //     VkSwapchainKHR                   oldSwapchain;
    // } VkSwapchainCreateInfoKHR;
    static VkSwapchainCreateInfoKHR scci = { 0 } ;
    scci.sType                      = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR ;
    scci.pNext                      = 0 ;
    scci.flags                      = 0 ;
    scci.surface                    = surface ;
    scci.minImageCount              = desired_image_count ;
    scci.imageFormat                = out_surface_format->format ;
    scci.imageColorSpace            = out_surface_format->colorSpace ;
    scci.imageExtent                = *out_extent ;
    scci.imageArrayLayers           = 1 ;
    scci.imageUsage                 = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ;
    if(queue_family_indices[0] == queue_family_indices[1])
    {
    scci.imageSharingMode           = VK_SHARING_MODE_EXCLUSIVE ;
    scci.queueFamilyIndexCount      = 0 ;
    scci.pQueueFamilyIndices        = NULL ;
    }
    else
    {
    scci.imageSharingMode           = VK_SHARING_MODE_CONCURRENT ;
    scci.queueFamilyIndexCount      = array_count(queue_family_indices) ;
    scci.pQueueFamilyIndices        = queue_family_indices ;
    }
    scci.preTransform               = scsd->capabilities_.currentTransform ;
    scci.compositeAlpha             = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR ;
    scci.presentMode                = *out_present_mode ;
    scci.clipped                    = VK_TRUE ;
    scci.oldSwapchain               = VK_NULL_HANDLE ;

    // VkResult vkCreateSwapchainKHR(
    //     VkDevice                                    device,
    //     const VkSwapchainCreateInfoKHR*             pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkSwapchainKHR*                             pSwapchain)
    if(check_vulkan(vkCreateSwapchainKHR(
                device
            ,   &scci
            ,   NULL
            ,   out_swapchain
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(out_swapchain) ;
    *out_image_count = desired_image_count ;

    uint32_t image_count = 0 ;
    // VkResult vkGetSwapchainImagesKHR(
    //     VkDevice                                    device,
    //     VkSwapchainKHR                              swapchain,
    //     uint32_t*                                   pSwapchainImageCount,
    //     VkImage*                                    pSwapchainImages);
    if(check_vulkan(vkGetSwapchainImagesKHR(
                device
            ,   *out_swapchain
            ,   &image_count
            ,   NULL
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    require(image_count == *out_image_count) ;

    if(check_vulkan(vkGetSwapchainImagesKHR(
                device
            ,   *out_swapchain
            ,   &image_count
            ,   out_images
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_image_views(
    VkImageView *               out_views
,   VkImage *                   images
,   uint32_t const              images_count
,   VkDevice const              device
,   VkFormat const              format
,   VkImageAspectFlags const    aspect_flags
,   uint32_t const              mip_levels
)
{
    require(out_views) ;
    require(images) ;
    require(device) ;
    begin_timed_block() ;

    // typedef struct VkImageViewCreateInfo {
    //     VkStructureType            sType;
    //     const void*                pNext;
    //     VkImageViewCreateFlags     flags;
    //     VkImage                    image;
    //     VkImageViewType            viewType;
    //     VkFormat                   format;
    //     VkComponentMapping         components;
    //     VkImageSubresourceRange    subresourceRange;
    // } VkImageViewCreateInfo;
    // typedef struct VkComponentMapping {
    //     VkComponentSwizzle    r;
    //     VkComponentSwizzle    g;
    //     VkComponentSwizzle    b;
    //     VkComponentSwizzle    a;
    // } VkComponentMapping;
    // typedef struct VkImageSubresourceRange {
    //     VkImageAspectFlags    aspectMask;
    //     uint32_t              baseMipLevel;
    //     uint32_t              levelCount;
    //     uint32_t              baseArrayLayer;
    //     uint32_t              layerCount;
    // } VkImageSubresourceRange;
    // typedef enum VkImageAspectFlagBits {
    //     VK_IMAGE_ASPECT_COLOR_BIT = 0x00000001,
    //     VK_IMAGE_ASPECT_DEPTH_BIT = 0x00000002,
    //     VK_IMAGE_ASPECT_STENCIL_BIT = 0x00000004,
    //     VK_IMAGE_ASPECT_METADATA_BIT = 0x00000008,
    // // Provided by VK_VERSION_1_1
    //     VK_IMAGE_ASPECT_PLANE_0_BIT = 0x00000010,
    // // Provided by VK_VERSION_1_1
    //     VK_IMAGE_ASPECT_PLANE_1_BIT = 0x00000020,
    // // Provided by VK_VERSION_1_1
    //     VK_IMAGE_ASPECT_PLANE_2_BIT = 0x00000040,
    // // Provided by VK_VERSION_1_3
    //     VK_IMAGE_ASPECT_NONE = 0,
    // // Provided by VK_EXT_image_drm_format_modifier
    //     VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT = 0x00000080,
    // // Provided by VK_EXT_image_drm_format_modifier
    //     VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT = 0x00000100,
    // // Provided by VK_EXT_image_drm_format_modifier
    //     VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT = 0x00000200,
    // // Provided by VK_EXT_image_drm_format_modifier
    //     VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT = 0x00000400,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_IMAGE_ASPECT_PLANE_0_BIT_KHR = VK_IMAGE_ASPECT_PLANE_0_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_IMAGE_ASPECT_PLANE_1_BIT_KHR = VK_IMAGE_ASPECT_PLANE_1_BIT,
    // // Provided by VK_KHR_sampler_ycbcr_conversion
    //     VK_IMAGE_ASPECT_PLANE_2_BIT_KHR = VK_IMAGE_ASPECT_PLANE_2_BIT,
    // // Provided by VK_KHR_maintenance4
    //     VK_IMAGE_ASPECT_NONE_KHR = VK_IMAGE_ASPECT_NONE,
    // } VkImageAspectFlagBits;
    static VkImageViewCreateInfo ivci = { 0 } ;
    ivci.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO ;
    ivci.pNext                              = NULL ;
    ivci.flags                              = 0 ;
    ivci.image                              = NULL ;
    ivci.viewType                           = VK_IMAGE_VIEW_TYPE_2D ;
    ivci.format                             = format ;
    ivci.components.a                       = VK_COMPONENT_SWIZZLE_IDENTITY ;
    ivci.components.r                       = VK_COMPONENT_SWIZZLE_IDENTITY ;
    ivci.components.g                       = VK_COMPONENT_SWIZZLE_IDENTITY ;
    ivci.components.b                       = VK_COMPONENT_SWIZZLE_IDENTITY ;
    ivci.subresourceRange.aspectMask        = aspect_flags ;
    ivci.subresourceRange.baseMipLevel      = 0 ;
    ivci.subresourceRange.levelCount        = mip_levels ;
    ivci.subresourceRange.baseArrayLayer    = 0 ;
    ivci.subresourceRange.layerCount        = 1 ;

    for(
        uint32_t i = 0
    ;   i < images_count
    ;   ++i
    )
    {
        require(images[i]) ;
        ivci.image  = images[i] ;

        // VkResult vkCreateImageView(
        //     VkDevice                                    device,
        //     const VkImageViewCreateInfo*                pCreateInfo,
        //     const VkAllocationCallbacks*                pAllocator,
        //     VkImageView*                                pView);
        if(check_vulkan(vkCreateImageView(
                    device
                ,   &ivci
                ,   NULL
                ,   &out_views[i]
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
        require(out_views[i]) ;
    }

    end_timed_block() ;
    return true ;
}



static bool
begin_single_time_commands(
    VkCommandBuffer *   out_command_buffer
,   VkDevice const      device
,   VkCommandPool const command_pool
)
{
    require(out_command_buffer) ;
    require(device) ;
    require(command_pool) ;

    begin_timed_block() ;

    static VkCommandBufferAllocateInfo cbai = { 0 } ;
    cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO ;
    cbai.pNext              = NULL ;
    cbai.commandPool        = command_pool ;
    cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY ;
    cbai.commandBufferCount = 1 ;

    if(check_vulkan(vkAllocateCommandBuffers(
                device
            ,   &cbai
            ,   out_command_buffer
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_command_buffer) ;

    static VkCommandBufferBeginInfo cbbi = { 0 } ;
    cbbi.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO ;
    cbbi.pNext              = NULL ;
    cbbi.flags              = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;
    cbbi.pInheritanceInfo   = NULL ;

    if(check_vulkan(vkBeginCommandBuffer(*out_command_buffer, &cbbi)))
    {
        end_timed_block() ;
        return false ;
    }


    end_timed_block() ;
    return true ;
}


static bool
end_single_time_commands(
    VkDevice const          device
,   VkCommandPool const     command_pool
,   VkQueue const           graphics_queue
,   VkCommandBuffer const   command_buffer
)
{
    require(device) ;
    require(command_pool) ;
    require(graphics_queue) ;
    require(command_buffer) ;
    begin_timed_block() ;

    if(check_vulkan(vkEndCommandBuffer(command_buffer)))
    {
        end_timed_block() ;
        return false ;
    }


    static VkSubmitInfo si = { 0 } ;
    si.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO ;
    si.pNext                    = NULL ;
    si.waitSemaphoreCount       = 0 ;
    si.pWaitSemaphores          = NULL ;
    si.pWaitDstStageMask        = NULL ;
    si.commandBufferCount       = 1 ;
    si.pCommandBuffers          = &command_buffer ;
    si.signalSemaphoreCount     = 0 ;
    si.pSignalSemaphores        = NULL ;

    if(check_vulkan(vkQueueSubmit(
                graphics_queue
            ,   1
            ,   &si
            ,   VK_NULL_HANDLE
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check_vulkan(vkQueueWaitIdle(graphics_queue)))
    {
        end_timed_block() ;
        return false ;
    }

    vkFreeCommandBuffers(
        device
    ,   command_pool
    ,   1
    ,   &command_buffer
    ) ;

    end_timed_block() ;
    return true ;
}


static bool
transition_image_layout(
    VkDevice const      device
,   VkCommandPool const command_pool
,   VkQueue const       graphics_queue
,   VkImage const       image
,   VkFormat const      format
,   VkImageLayout const old_layout
,   VkImageLayout const new_layout
,   uint32_t const      mip_levels
)
{
    require(device) ;
    require(command_pool) ;
    require(graphics_queue) ;
    require(image) ;
    begin_timed_block() ;

    VkCommandBuffer command_buffer = NULL ;
    if(check(begin_single_time_commands(
                &command_buffer
            ,   device
            ,   command_pool
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    // typedef struct VkImageMemoryBarrier {
    //     VkStructureType            sType;
    //     const void*                pNext;
    //     VkAccessFlags              srcAccessMask;
    //     VkAccessFlags              dstAccessMask;
    //     VkImageLayout              oldLayout;
    //     VkImageLayout              newLayout;
    //     uint32_t                   srcQueueFamilyIndex;
    //     uint32_t                   dstQueueFamilyIndex;
    //     VkImage                    image;
    //     VkImageSubresourceRange    subresourceRange;
    // } VkImageMemoryBarrier;
    // typedef struct VkImageSubresourceRange {
    //     VkImageAspectFlags    aspectMask;
    //     uint32_t              baseMipLevel;
    //     uint32_t              levelCount;
    //     uint32_t              baseArrayLayer;
    //     uint32_t              layerCount;
    // } VkImageSubresourceRange;
    static VkImageMemoryBarrier imb = { 0 } ;
    imb.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER ;
    imb.pNext                           = NULL ;
    imb.srcAccessMask                   = 0 ;
    imb.dstAccessMask                   = 0 ;
    imb.oldLayout                       = old_layout ;
    imb.newLayout                       = new_layout ;
    imb.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED ;
    imb.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED ;
    imb.image                           = image ;
    imb.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT ;
    imb.subresourceRange.baseMipLevel   = 0 ;
    imb.subresourceRange.levelCount     = mip_levels ;
    imb.subresourceRange.baseArrayLayer = 0 ;
    imb.subresourceRange.layerCount     = 1 ;

    VkPipelineStageFlags src_stage ;
    VkPipelineStageFlags dst_stage ;

    if(new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT ;

        if(has_stencil_component(format))
        {
            imb.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT ;
        }
    }
    else
    {
        imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT ;
    }

    if(
        old_layout == VK_IMAGE_LAYOUT_UNDEFINED
    &&  new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    )
    {
        imb.srcAccessMask = 0 ;
        imb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT ;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT ;
    }
    else if(
        old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    &&  new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    )
    {
        imb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT ;
        imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT ;

        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT ;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ;
    }
    else if(
        old_layout == VK_IMAGE_LAYOUT_UNDEFINED
    &&  new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    )
    {
        imb.srcAccessMask = 0 ;
        imb.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT ;

        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ;
        dst_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ;
    }
    else
    {
        require(0) ;
        end_timed_block() ;
        return false ;
    }


    // void vkCmdPipelineBarrier(
    //     VkCommandBuffer                             commandBuffer,
    //     VkPipelineStageFlags                        srcStageMask,
    //     VkPipelineStageFlags                        dstStageMask,
    //     VkDependencyFlags                           dependencyFlags,
    //     uint32_t                                    memoryBarrierCount,
    //     const VkMemoryBarrier*                      pMemoryBarriers,
    //     uint32_t                                    bufferMemoryBarrierCount,
    //     const VkBufferMemoryBarrier*                pBufferMemoryBarriers,
    //     uint32_t                                    imageMemoryBarrierCount,
    //     const VkImageMemoryBarrier*                 pImageMemoryBarriers);
    vkCmdPipelineBarrier(
        command_buffer
    ,   src_stage
    ,   dst_stage
    ,   0
    ,   0
    ,   NULL
    ,   0
    ,   NULL
    ,   1
    ,   &imb
    ) ;

    if(check(end_single_time_commands(
                device
            ,   command_pool
            ,   graphics_queue
            ,   command_buffer
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_depth_resource(
    vulkan_context *    vc
)
{
    require(vc) ;
    require(vc->device_) ;
    begin_timed_block() ;

    if(check(create_image(
                &vc->depth_image_
            ,   &vc->depth_image_memory_
            ,   vc->device_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   vc->swapchain_extent_.width
            ,   vc->swapchain_extent_.height
            ,   1
            ,   vc->sample_count_
            ,   vc->picked_physical_device_->depth_format_
            ,   VK_IMAGE_TILING_OPTIMAL
            ,   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            ,   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->depth_image_) ;
    require(vc->depth_image_memory_) ;


    if(check(create_image_views(
                &vc->depth_image_view_
            ,   &vc->depth_image_
            ,   1
            ,   vc->device_
            ,   vc->picked_physical_device_->depth_format_
            ,   VK_IMAGE_ASPECT_DEPTH_BIT
            ,   1
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->depth_image_view_) ;

    if(check(transition_image_layout(
                vc->device_
            ,   vc->command_pool_
            ,   vc->graphics_queue_
            ,   vc->depth_image_
            ,   vc->picked_physical_device_->depth_format_
            ,   VK_IMAGE_LAYOUT_UNDEFINED
            ,   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            ,   1
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    end_timed_block() ;
    return true ;
}


static void
cleanup_swapchain(
    vulkan_context * vc
)
{
    require(vc) ;
    require(vc->device_) ;
    require(vc->swapchain_images_count_ < max_vulkan_swapchain_images) ;

    begin_timed_block() ;

    vkDeviceWaitIdle(vc->device_);

    if(vc->color_image_view_)
    {
        vkDestroyImageView(vc->device_, vc->color_image_view_, NULL) ;
        vc->color_image_view_ = NULL ;
    }

    if(vc->color_image_)
    {
        vkDestroyImage(vc->device_, vc->color_image_, NULL) ;
        vc->color_image_ = NULL ;
    }

    if(vc->color_image_memory_)
    {
        vkFreeMemory(vc->device_, vc->color_image_memory_, NULL) ;
        vc->color_image_memory_ = NULL ;
    }

    if(vc->depth_image_view_)
    {
        vkDestroyImageView(vc->device_, vc->depth_image_view_, NULL) ;
        vc->depth_image_view_ = NULL ;
    }

    if(vc->depth_image_)
    {
        vkDestroyImage(vc->device_, vc->depth_image_, NULL) ;
        vc->depth_image_ = NULL ;
    }

    if(vc->depth_image_memory_)
    {
        vkFreeMemory(vc->device_, vc->depth_image_memory_, NULL) ;
        vc->depth_image_memory_ = NULL ;
    }

    for(
        uint32_t i = 0
    ;   i < vc_->swapchain_images_count_
    ;   ++i
    )
    {
        if(vc_->framebuffers_[i])
        {
            // void vkDestroyFramebuffer(
            //     VkDevice                                    device,
            //     VkFramebuffer                               framebuffer,
            //     const VkAllocationCallbacks*                pAllocator);
            vkDestroyFramebuffer(vc_->device_, vc_->framebuffers_[i], NULL) ;
            vc_->framebuffers_[i] = NULL ;
        }

        if(vc_->swapchain_views_[i])
        {
            // void vkDestroyImageView(
            //     VkDevice                                    device,
            //     VkImageView                                 imageView,
            //     const VkAllocationCallbacks*                pAllocator);
            vkDestroyImageView(vc_->device_, vc_->swapchain_views_[i], NULL) ;
            vc_->swapchain_views_[i] = NULL ;
        }
    }

    if(vc_->swapchain_)
    {
        // void vkDestroySwapchainKHR(
        //     VkDevice                                    device,
        //     VkSwapchainKHR                              swapchain,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroySwapchainKHR(vc_->device_, vc_->swapchain_, NULL) ;
        vc_->swapchain_ = NULL ;
    }

    end_timed_block() ;
}


static bool
create_color_resource(
    vulkan_context *    vc
)
{
    require(vc) ;
    require(vc->device_) ;
    begin_timed_block() ;

    if(!vc->enable_sampling_)
    {
        end_timed_block() ;
        return true ;
    }

    if(check(create_image(
                &vc->color_image_
            ,   &vc->color_image_memory_
            ,   vc->device_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   vc->swapchain_extent_.width
            ,   vc->swapchain_extent_.height
            ,   1
            ,   vc->sample_count_
            ,   vc->swapchain_surface_format_.format
            ,   VK_IMAGE_TILING_OPTIMAL
            ,   VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            ,   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->color_image_) ;
    require(vc->color_image_memory_) ;

    if(check(create_image_views(
                &vc->color_image_view_
            ,   &vc->color_image_
            ,   1
            ,   vc->device_
            ,   vc->swapchain_surface_format_.format
            ,   VK_IMAGE_ASPECT_COLOR_BIT
            ,   1
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->color_image_view_) ;

    end_timed_block() ;
    return true ;
}


static bool
recreate_swapchain(
    vulkan_context *    vc
)
{
    require(vc) ;
    require(vc->device_) ;
    begin_timed_block() ;

    //vkDeviceWaitIdle(vc->device_) ;
    cleanup_swapchain(vc) ;

    if(check(create_swapchain(
                &vc->swapchain_
            ,   &vc->swapchain_surface_format_
            ,   &vc->swapchain_present_mode_
            ,   &vc->swapchain_extent_
            ,   vc->swapchain_images_
            ,   &vc->swapchain_images_count_
            ,   vc->device_
            ,   vc->surface_
            ,   &vc->picked_physical_device_->swapchain_support_details_
            ,   &vc->picked_physical_device_->queue_families_indices_
            ,   vc->desired_swapchain_image_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    if(check(create_image_views(
                vc->swapchain_views_
            ,   vc->swapchain_images_
            ,   vc->swapchain_images_count_
            ,   vc->device_
            ,   vc->swapchain_surface_format_.format
            ,   VK_IMAGE_ASPECT_COLOR_BIT
            ,   1
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_color_resource(vc)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_depth_resource(vc)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_framebuffers(vc)))
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_shader_module(
    VkShaderModule *    out_shader_module
,   VkDevice const      device
,   void const *        shader_code
,   uint64_t const      shader_code_size
)
{
    require(out_shader_module) ;
    require(shader_code) ;
    require(shader_code_size) ;
    require(0 == (((uintptr_t)shader_code)&3)) ;

    begin_timed_block() ;

    // typedef struct VkShaderModuleCreateInfo {
    //     VkStructureType              sType;
    //     const void*                  pNext;
    //     VkShaderModuleCreateFlags    flags;
    //     size_t                       codeSize;
    //     const uint32_t*              pCode;
    // } VkShaderModuleCreateInfo;
    static VkShaderModuleCreateInfo smci = { 0 } ;
    smci.sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO ;
    smci.pNext      = NULL ;
    smci.flags      = 0 ;
    smci.codeSize   = shader_code_size ;
    smci.pCode      = (uint32_t const *) shader_code ;

    // VkResult vkCreateShaderModule(
    //     VkDevice                                    device,
    //     const VkShaderModuleCreateInfo*             pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkShaderModule*                             pShaderModule);
    if(check_vulkan(vkCreateShaderModule(
                device
            ,   &smci
            ,   NULL
            ,   out_shader_module
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_shader_module) ;

    end_timed_block() ;
    return true ;

}


static bool
create_graphics_pipeline(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

    if(check(load_file(
                &vc->vert_shader_memory_
            ,   &vc->vert_shader_memory_size_
            ,   "ass/shaders/shader.vert.spv"
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    log_debug_ptr(vc->vert_shader_memory_) ;
    log_debug_u64(vc->vert_shader_memory_size_) ;

    if(check(load_file(
                &vc->frag_shader_memory_
            ,   &vc->frag_shader_memory_size_
            ,   "ass/shaders/shader.frag.spv"
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    log_debug_ptr(vc->frag_shader_memory_) ;
    log_debug_u64(vc->frag_shader_memory_size_) ;

    if(check(create_shader_module(
                &vc->vert_shader_
            ,   vc->device_
            ,   vc->vert_shader_memory_
            ,   vc->vert_shader_memory_size_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->vert_shader_) ;
    log_debug_ptr(vc->vert_shader_) ;

    if(check(create_shader_module(
                &vc->frag_shader_
            ,   vc->device_
            ,   vc->frag_shader_memory_
            ,   vc->frag_shader_memory_size_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->frag_shader_) ;
    log_debug_ptr(vc->frag_shader_) ;


    // typedef struct VkPipelineShaderStageCreateInfo {
    //     VkStructureType                     sType;
    //     const void*                         pNext;
    //     VkPipelineShaderStageCreateFlags    flags;
    //     VkShaderStageFlagBits               stage;
    //     VkShaderModule                      module;
    //     const char*                         pName;
    //     const VkSpecializationInfo*         pSpecializationInfo;
    // } VkPipelineShaderStageCreateInfo;
    static VkPipelineShaderStageCreateInfo pssci[2] = { 0 } ;
    pssci[0].sType                 = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ;
    pssci[0].pNext                 = NULL ;
    pssci[0].flags                 = 0 ;
    pssci[0].stage                 = VK_SHADER_STAGE_VERTEX_BIT ;
    pssci[0].module                = vc->vert_shader_ ;
    pssci[0].pName                 = "main" ;
    pssci[0].pSpecializationInfo   = NULL ;

    pssci[1].sType                 = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO ;
    pssci[1].pNext                 = NULL ;
    pssci[1].flags                 = 0 ;
    pssci[1].stage                 = VK_SHADER_STAGE_FRAGMENT_BIT ;
    pssci[1].module                = vc->frag_shader_ ;
    pssci[1].pName                 = "main" ;
    pssci[1].pSpecializationInfo   = NULL ;


    // typedef struct VkPipelineDynamicStateCreateInfo {
    //     VkStructureType                      sType;
    //     const void*                          pNext;
    //     VkPipelineDynamicStateCreateFlags    flags;
    //     uint32_t                             dynamicStateCount;
    //     const VkDynamicState*                pDynamicStates;
    // } VkPipelineDynamicStateCreateInfo;
    // typedef enum VkDynamicState {
    //     VK_DYNAMIC_STATE_VIEWPORT = 0,
    //     VK_DYNAMIC_STATE_SCISSOR = 1,
    //     VK_DYNAMIC_STATE_LINE_WIDTH = 2,
    //     VK_DYNAMIC_STATE_DEPTH_BIAS = 3,
    //     VK_DYNAMIC_STATE_BLEND_CONSTANTS = 4,
    //     VK_DYNAMIC_STATE_DEPTH_BOUNDS = 5,
    //     VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK = 6,
    //     VK_DYNAMIC_STATE_STENCIL_WRITE_MASK = 7,
    //     VK_DYNAMIC_STATE_STENCIL_REFERENCE = 8,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_CULL_MODE = 1000267000,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_FRONT_FACE = 1000267001,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY = 1000267002,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT = 1000267003,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT = 1000267004,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE = 1000267005,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE = 1000267006,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE = 1000267007,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_DEPTH_COMPARE_OP = 1000267008,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE = 1000267009,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE = 1000267010,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_STENCIL_OP = 1000267011,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE = 1000377001,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE = 1000377002,
    // // Provided by VK_VERSION_1_3
    //     VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE = 1000377004,
    // // Provided by VK_NV_clip_space_w_scaling
    //     VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV = 1000087000,
    // // Provided by VK_EXT_discard_rectangles
    //     VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT = 1000099000,
    // // Provided by VK_EXT_discard_rectangles
    //     VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT = 1000099001,
    // // Provided by VK_EXT_discard_rectangles
    //     VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT = 1000099002,
    // // Provided by VK_EXT_sample_locations
    //     VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT = 1000143000,
    // // Provided by VK_KHR_ray_tracing_pipeline
    //     VK_DYNAMIC_STATE_RAY_TRACING_PIPELINE_STACK_SIZE_KHR = 1000347000,
    // // Provided by VK_NV_shading_rate_image
    //     VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV = 1000164004,
    // // Provided by VK_NV_shading_rate_image
    //     VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV = 1000164006,
    // // Provided by VK_NV_scissor_exclusive
    //     VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV = 1000205000,
    // // Provided by VK_NV_scissor_exclusive
    //     VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV = 1000205001,
    // // Provided by VK_KHR_fragment_shading_rate
    //     VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR = 1000226000,
    // // Provided by VK_EXT_vertex_input_dynamic_state
    //     VK_DYNAMIC_STATE_VERTEX_INPUT_EXT = 1000352000,
    // // Provided by VK_EXT_extended_dynamic_state2
    //     VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT = 1000377000,
    // // Provided by VK_EXT_extended_dynamic_state2
    //     VK_DYNAMIC_STATE_LOGIC_OP_EXT = 1000377003,
    // // Provided by VK_EXT_color_write_enable
    //     VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT = 1000381000,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT = 1000455003,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_POLYGON_MODE_EXT = 1000455004,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT = 1000455005,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_SAMPLE_MASK_EXT = 1000455006,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT = 1000455007,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT = 1000455008,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT = 1000455009,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT = 1000455010,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT = 1000455011,
    // // Provided by VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT = 1000455012,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_KHR_maintenance2 or VK_VERSION_1_1
    //     VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT = 1000455002,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_EXT_transform_feedback
    //     VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT = 1000455013,
    // // Provided by VK_EXT_conservative_rasterization with VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT = 1000455014,
    // // Provided by VK_EXT_conservative_rasterization with VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT = 1000455015,
    // // Provided by VK_EXT_depth_clip_enable with VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT = 1000455016,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_EXT_sample_locations
    //     VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT = 1000455017,
    // // Provided by VK_EXT_blend_operation_advanced with VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT = 1000455018,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_EXT_provoking_vertex
    //     VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT = 1000455019,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_EXT_line_rasterization
    //     VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT = 1000455020,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_EXT_line_rasterization
    //     VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT = 1000455021,
    // // Provided by VK_EXT_depth_clip_control with VK_EXT_extended_dynamic_state3
    //     VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT = 1000455022,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_clip_space_w_scaling
    //     VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV = 1000455023,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_viewport_swizzle
    //     VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV = 1000455024,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_fragment_coverage_to_color
    //     VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV = 1000455025,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_fragment_coverage_to_color
    //     VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV = 1000455026,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_framebuffer_mixed_samples
    //     VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV = 1000455027,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_framebuffer_mixed_samples
    //     VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV = 1000455028,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_framebuffer_mixed_samples
    //     VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV = 1000455029,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_shading_rate_image
    //     VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV = 1000455030,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_representative_fragment_test
    //     VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV = 1000455031,
    // // Provided by VK_EXT_extended_dynamic_state3 with VK_NV_coverage_reduction_mode
    //     VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV = 1000455032,
    // // Provided by VK_EXT_attachment_feedback_loop_dynamic_state
    //     VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT = 1000524000,
    // // Provided by VK_KHR_line_rasterization
    //     VK_DYNAMIC_STATE_LINE_STIPPLE_KHR = 1000259000,
    // // Provided by VK_EXT_line_rasterization
    //     VK_DYNAMIC_STATE_LINE_STIPPLE_EXT = VK_DYNAMIC_STATE_LINE_STIPPLE_KHR,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_CULL_MODE_EXT = VK_DYNAMIC_STATE_CULL_MODE,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_FRONT_FACE_EXT = VK_DYNAMIC_STATE_FRONT_FACE,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT_EXT = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT = VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
    // // Provided by VK_EXT_extended_dynamic_state
    //     VK_DYNAMIC_STATE_STENCIL_OP_EXT = VK_DYNAMIC_STATE_STENCIL_OP,
    // // Provided by VK_EXT_extended_dynamic_state2
    //     VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE,
    // // Provided by VK_EXT_extended_dynamic_state2
    //     VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE_EXT = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
    // // Provided by VK_EXT_extended_dynamic_state2
    //     VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE_EXT = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE,
    // } VkDynamicState;

    static VkDynamicState   dynamic_states[2] = { 0 } ;
    dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT ;
    dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR ;


    // typedef struct VkPipelineDynamicStateCreateInfo {
    //     VkStructureType                      sType;
    //     const void*                          pNext;
    //     VkPipelineDynamicStateCreateFlags    flags;
    //     uint32_t                             dynamicStateCount;
    //     const VkDynamicState*                pDynamicStates;
    // } VkPipelineDynamicStateCreateInfo;

    static VkPipelineDynamicStateCreateInfo pdsci = { 0 } ;
    pdsci.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO ;
    pdsci.pNext                 = NULL ;
    pdsci.flags                 = 0 ;
    pdsci.dynamicStateCount     = array_count(dynamic_states) ;
    pdsci.pDynamicStates        = dynamic_states ;



    VkVertexInputBindingDescription     vibd = get_binding_description() ;
    VkVertexInputAttributeDescription   viad[3] = { 0 } ;
    get_attribute_descriptions(viad, 3) ;

    // typedef struct VkPipelineVertexInputStateCreateInfo {
    //     VkStructureType                             sType;
    //     const void*                                 pNext;
    //     VkPipelineVertexInputStateCreateFlags       flags;
    //     uint32_t                                    vertexBindingDescriptionCount;
    //     const VkVertexInputBindingDescription*      pVertexBindingDescriptions;
    //     uint32_t                                    vertexAttributeDescriptionCount;
    //     const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
    // } VkPipelineVertexInputStateCreateInfo;
    static VkPipelineVertexInputStateCreateInfo pvisci = { 0 } ;
    pvisci.sType                            = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO ;
    pvisci.pNext                            = NULL ;
    pvisci.flags                            = 0 ;
    pvisci.vertexBindingDescriptionCount    = 1 ;
    pvisci.pVertexBindingDescriptions       = &vibd ;
    pvisci.vertexAttributeDescriptionCount  = array_count(viad) ;
    pvisci.pVertexAttributeDescriptions     = viad ;


    // typedef struct VkPipelineInputAssemblyStateCreateInfo {
    //     VkStructureType                            sType;
    //     const void*                                pNext;
    //     VkPipelineInputAssemblyStateCreateFlags    flags;
    //     VkPrimitiveTopology                        topology;
    //     VkBool32                                   primitiveRestartEnable;
    // } VkPipelineInputAssemblyStateCreateInfo;
    static VkPipelineInputAssemblyStateCreateInfo piasci = { 0 } ;
    piasci.sType                    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO ;
    piasci.pNext                    = NULL ;
    piasci.flags                    = 0 ;
    piasci.topology                 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST ;
    piasci.primitiveRestartEnable   = VK_FALSE ;



    // typedef struct VkViewport {
    //     float    x;
    //     float    y;
    //     float    width;
    //     float    height;
    //     float    minDepth;
    //     float    maxDepth;
    // } VkViewport;
    // static VkViewport viewport = { 0 } ;
    // viewport.x          = 0.0f ;
    // viewport.y          = 0.0f ;
    // viewport.width      = (float) vc->swapchain_extent_.width ;
    // viewport.height     = (float) vc->swapchain_extent_.height ;
    // viewport.minDepth   = 0.0f ;
    // viewport.maxDepth   = 1.0f ;


    // typedef struct VkRect2D {
    //     VkOffset2D    offset;
    //     VkExtent2D    extent;
    // } VkRect2D;
    // typedef struct VkOffset2D {
    //     int32_t    x;
    //     int32_t    y;
    // } VkOffset2D;
    // typedef struct VkExtent2D {
    //     uint32_t    width;
    //     uint32_t    height;
    // } VkExtent2D;
    // static VkRect2D scissor = { 0 } ;
    // scissor.offset.x        = 0 ;
    // scissor.offset.y        = 0 ;
    // scissor.extent.width    = vc->swapchain_extent_.width ;
    // scissor.extent.height   = vc->swapchain_extent_.height ;


    // typedef struct VkPipelineViewportStateCreateInfo {
    //     VkStructureType                       sType;
    //     const void*                           pNext;
    //     VkPipelineViewportStateCreateFlags    flags;
    //     uint32_t                              viewportCount;
    //     const VkViewport*                     pViewports;
    //     uint32_t                              scissorCount;
    //     const VkRect2D*                       pScissors;
    // } VkPipelineViewportStateCreateInfo;
    static VkPipelineViewportStateCreateInfo pvsci = { 0 } ;
    pvsci.sType             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO ;
    pvsci.pNext             = NULL ;
    pvsci.flags             = 0 ;
    pvsci.viewportCount     = 1 ;
    pvsci.pViewports        = NULL ;
    pvsci.scissorCount      = 1 ;
    pvsci.pScissors         = NULL ;


    // typedef struct VkPipelineRasterizationStateCreateInfo {
    //     VkStructureType                            sType;
    //     const void*                                pNext;
    //     VkPipelineRasterizationStateCreateFlags    flags;
    //     VkBool32                                   depthClampEnable;
    //     VkBool32                                   rasterizerDiscardEnable;
    //     VkPolygonMode                              polygonMode;
    //     VkCullModeFlags                            cullMode;
    //     VkFrontFace                                frontFace;
    //     VkBool32                                   depthBiasEnable;
    //     float                                      depthBiasConstantFactor;
    //     float                                      depthBiasClamp;
    //     float                                      depthBiasSlopeFactor;
    //     float                                      lineWidth;
    // } VkPipelineRasterizationStateCreateInfo;
    static VkPipelineRasterizationStateCreateInfo prsci = { 0 } ;
    prsci.sType                     = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO ;
    prsci.pNext                     = NULL ;
    prsci.flags                     = 0 ;
    prsci.depthClampEnable          = VK_FALSE ;
    prsci.rasterizerDiscardEnable   = VK_FALSE ;
    prsci.polygonMode               = VK_POLYGON_MODE_FILL ;
    prsci.cullMode                  = VK_CULL_MODE_BACK_BIT ;
    prsci.frontFace                 = VK_FRONT_FACE_COUNTER_CLOCKWISE ;
    prsci.depthBiasEnable           = VK_FALSE ;
    prsci.depthBiasConstantFactor   = 0.0f ;
    prsci.depthBiasClamp            = 0.0f ;
    prsci.depthBiasSlopeFactor      = 0.0f ;
    prsci.lineWidth                 = 1.0f ;


    // typedef struct VkPipelineMultisampleStateCreateInfo {
    //     VkStructureType                          sType;
    //     const void*                              pNext;
    //     VkPipelineMultisampleStateCreateFlags    flags;
    //     VkSampleCountFlagBits                    rasterizationSamples;
    //     VkBool32                                 sampleShadingEnable;
    //     float                                    minSampleShading;
    //     const VkSampleMask*                      pSampleMask;
    //     VkBool32                                 alphaToCoverageEnable;
    //     VkBool32                                 alphaToOneEnable;
    // } VkPipelineMultisampleStateCreateInfo;
    static VkPipelineMultisampleStateCreateInfo pmssci = { 0 } ;
    pmssci.sType                    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO ;
    pmssci.pNext                    = NULL ;
    pmssci.flags                    = 0 ;
    if(vc->enable_sampling_)
    {
        pmssci.rasterizationSamples = vc->sample_count_ ;
    }
    else
    {
        pmssci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT ;
    }
    if(vc->enable_sample_shading_)
    {
        pmssci.sampleShadingEnable  = vc->enable_sample_shading_ ;
        pmssci.minSampleShading     = vc->min_sample_shading_ ;
    }
    else
    {
        pmssci.sampleShadingEnable  = VK_FALSE ;
        pmssci.minSampleShading     = 1.0f ;
    }
    pmssci.pSampleMask              = NULL ;
    pmssci.alphaToCoverageEnable    = VK_FALSE ;
    pmssci.alphaToOneEnable         = VK_FALSE ;

    // typedef struct VkPipelineColorBlendAttachmentState {
    //     VkBool32                 blendEnable;
    //     VkBlendFactor            srcColorBlendFactor;
    //     VkBlendFactor            dstColorBlendFactor;
    //     VkBlendOp                colorBlendOp;
    //     VkBlendFactor            srcAlphaBlendFactor;
    //     VkBlendFactor            dstAlphaBlendFactor;
    //     VkBlendOp                alphaBlendOp;
    //     VkColorComponentFlags    colorWriteMask;
    // } VkPipelineColorBlendAttachmentState;
    static VkPipelineColorBlendAttachmentState pcbas = { 0 } ;
    pcbas.blendEnable           = VK_FALSE ;
    pcbas.srcColorBlendFactor   = VK_BLEND_FACTOR_ONE ;
    pcbas.dstColorBlendFactor   = VK_BLEND_FACTOR_ZERO ;
    pcbas.colorBlendOp          = VK_BLEND_OP_ADD ;
    pcbas.srcAlphaBlendFactor   = VK_BLEND_FACTOR_ONE ;
    pcbas.dstAlphaBlendFactor   = VK_BLEND_FACTOR_ZERO ;
    pcbas.alphaBlendOp          = VK_BLEND_OP_ADD ;
    pcbas.colorWriteMask        = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT ;
    // transparency
    // pcbas.blendEnable           = VK_TRUE ;
    // pcbas.srcColorBlendFactor   = VK_BLEND_FACTOR_SRC_ALPHA ;
    // pcbas.dstColorBlendFactor   = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA ;
    // pcbas.colorBlendOp          = VK_BLEND_OP_ADD ;
    // pcbas.srcAlphaBlendFactor   = VK_BLEND_FACTOR_ONE ;
    // pcbas.dstAlphaBlendFactor   = VK_BLEND_FACTOR_ZERO ;
    // pcbas.alphaBlendOp          = VK_BLEND_OP_ADD ;
    // pcbas.colorWriteMask        = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT ;


    // typedef struct VkPipelineColorBlendStateCreateInfo {
    //     VkStructureType                               sType;
    //     const void*                                   pNext;
    //     VkPipelineColorBlendStateCreateFlags          flags;
    //     VkBool32                                      logicOpEnable;
    //     VkLogicOp                                     logicOp;
    //     uint32_t                                      attachmentCount;
    //     const VkPipelineColorBlendAttachmentState*    pAttachments;
    //     float                                         blendConstants[4];
    // } VkPipelineColorBlendStateCreateInfo;
    static VkPipelineColorBlendStateCreateInfo pcbsci = { 0 } ;
    pcbsci.sType                = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO ;
    pcbsci.pNext                = NULL ;
    pcbsci.flags                = 0 ;
    pcbsci.logicOpEnable        = VK_FALSE ;
    pcbsci.logicOp              = VK_LOGIC_OP_COPY ;
    pcbsci.attachmentCount      = 1 ;
    pcbsci.pAttachments         = &pcbas ;
    pcbsci.blendConstants[0]    = 0.0f ;
    pcbsci.blendConstants[1]    = 0.0f ;
    pcbsci.blendConstants[2]    = 0.0f ;
    pcbsci.blendConstants[3]    = 0.0f ;

    // typedef struct VkPipelineDepthStencilStateCreateInfo {
    //     VkStructureType                           sType;
    //     const void*                               pNext;
    //     VkPipelineDepthStencilStateCreateFlags    flags;
    //     VkBool32                                  depthTestEnable;
    //     VkBool32                                  depthWriteEnable;
    //     VkCompareOp                               depthCompareOp;
    //     VkBool32                                  depthBoundsTestEnable;
    //     VkBool32                                  stencilTestEnable;
    //     VkStencilOpState                          front;
    //     VkStencilOpState                          back;
    //     float                                     minDepthBounds;
    //     float                                     maxDepthBounds;
    // } VkPipelineDepthStencilStateCreateInfo;
    // typedef struct VkStencilOpState {
    //     VkStencilOp    failOp;
    //     VkStencilOp    passOp;
    //     VkStencilOp    depthFailOp;
    //     VkCompareOp    compareOp;
    //     uint32_t       compareMask;
    //     uint32_t       writeMask;
    //     uint32_t       reference;
    // } VkStencilOpState;
    // typedef enum VkStencilOp {
    //     VK_STENCIL_OP_KEEP = 0,
    //     VK_STENCIL_OP_ZERO = 1,
    //     VK_STENCIL_OP_REPLACE = 2,
    //     VK_STENCIL_OP_INCREMENT_AND_CLAMP = 3,
    //     VK_STENCIL_OP_DECREMENT_AND_CLAMP = 4,
    //     VK_STENCIL_OP_INVERT = 5,
    //     VK_STENCIL_OP_INCREMENT_AND_WRAP = 6,
    //     VK_STENCIL_OP_DECREMENT_AND_WRAP = 7,
    // } VkStencilOp;

    static VkPipelineDepthStencilStateCreateInfo pdssci = { 0 } ;
    pdssci.sType                    = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO ;
    pdssci.pNext                    = NULL ;
    pdssci.flags                    = 0 ;
    pdssci.depthTestEnable          = VK_TRUE ;
    pdssci.depthWriteEnable         = VK_TRUE ;
    pdssci.depthCompareOp           = VK_COMPARE_OP_LESS ;
    pdssci.depthBoundsTestEnable    = VK_FALSE ;
    pdssci.stencilTestEnable        = VK_FALSE ;
    pdssci.front.failOp             = 0 ;
    pdssci.front.passOp             = 0 ;
    pdssci.front.depthFailOp        = 0 ;
    pdssci.front.compareOp          = 0 ;
    pdssci.front.compareMask        = 0 ;
    pdssci.front.writeMask          = 0 ;
    pdssci.front.reference          = 0 ;
    pdssci.back.failOp              = 0 ;
    pdssci.back.passOp              = 0 ;
    pdssci.back.depthFailOp         = 0 ;
    pdssci.back.compareOp           = 0 ;
    pdssci.back.compareMask         = 0 ;
    pdssci.back.writeMask           = 0 ;
    pdssci.back.reference           = 0 ;
    pdssci.minDepthBounds           = 0.0f ;
    pdssci.maxDepthBounds           = 1.0f ;


    // typedef struct VkGraphicsPipelineCreateInfo {
    //     VkStructureType                                  sType;
    //     const void*                                      pNext;
    //     VkPipelineCreateFlags                            flags;
    //     uint32_t                                         stageCount;
    //     const VkPipelineShaderStageCreateInfo*           pStages;
    //     const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
    //     const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
    //     const VkPipelineTessellationStateCreateInfo*     pTessellationState;
    //     const VkPipelineViewportStateCreateInfo*         pViewportState;
    //     const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
    //     const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
    //     const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
    //     const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
    //     const VkPipelineDynamicStateCreateInfo*          pDynamicState;
    //     VkPipelineLayout                                 layout;
    //     VkRenderPass                                     renderPass;
    //     uint32_t                                         subpass;
    //     VkPipeline                                       basePipelineHandle;
    //     int32_t                                          basePipelineIndex;
    // } VkGraphicsPipelineCreateInfo;
    static VkGraphicsPipelineCreateInfo gpci = { 0 } ;
    // away
    // gpci.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO ;
    // gpci.pNext                  = NULL ;
    // gpci.flags                  = 0 ;
    // gpci.stageCount             = array_count(pssci) ;
    // gpci.pStages                = pssci ;
    // gpci.pVertexInputState      = &pvisci ;
    // gpci.pInputAssemblyState    = &piasci ;
    // gpci.pTessellationState     = NULL ;
    // gpci.pViewportState         = &pvsci ;
    // gpci.pRasterizationState    = &prsci ;
    // gpci.pMultisampleState      = &pmssci ;
    // gpci.pDepthStencilState     = &pdssci ;
    // gpci.pColorBlendState       = &pcbsci ;
    // gpci.pDynamicState          = &pdsci ;
    // gpci.layout                 = vc->pipeline_layout_ ;
    // gpci.renderPass             = vc->render_pass_ ;
    // gpci.subpass                = 0 ;
    // gpci.basePipelineHandle     = VK_NULL_HANDLE ;
    // gpci.basePipelineIndex      = -1 ;



    // VkResult vkCreateGraphicsPipelines(
    //     VkDevice                                    device,
    //     VkPipelineCache                             pipelineCache,
    //     uint32_t                                    createInfoCount,
    //     const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkPipeline*                                 pPipelines);
    if(check_vulkan(vkCreateGraphicsPipelines(
                vc->device_
            ,   VK_NULL_HANDLE
            ,   1
            ,   &gpci
            ,   NULL
            ,   &vc->graphics_pipeline_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->graphics_pipeline_) ;


    // void vkDestroyShaderModule(
    //     VkDevice                                    device,
    //     VkShaderModule                              shaderModule,
    //     const VkAllocationCallbacks*                pAllocator);
    vkDestroyShaderModule(vc->device_, vc->vert_shader_, NULL) ;
    vkDestroyShaderModule(vc->device_, vc->frag_shader_, NULL) ;
    vc->vert_shader_ = NULL ;
    vc->frag_shader_ = NULL ;

    end_timed_block() ;
    return true ;
}


static bool
create_render_pass(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

    // typedef struct VkAttachmentDescription {
    //     VkAttachmentDescriptionFlags    flags;
    //     VkFormat                        format;
    //     VkSampleCountFlagBits           samples;
    //     VkAttachmentLoadOp              loadOp;
    //     VkAttachmentStoreOp             storeOp;
    //     VkAttachmentLoadOp              stencilLoadOp;
    //     VkAttachmentStoreOp             stencilStoreOp;
    //     VkImageLayout                   initialLayout;
    //     VkImageLayout                   finalLayout;
    // } VkAttachmentDescription;

    static VkAttachmentDescription  attachments[3] = { 0 } ;
    static uint32_t                 attachments_count = 0 ;

    if(vc->enable_sampling_)
    {
        attachments_count               = 3 ;

        attachments[0].flags            = 0 ;
        attachments[0].format           = vc->swapchain_surface_format_.format ;
        attachments[0].samples          = vc->sample_count_ ;
        attachments[0].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR ;
        attachments[0].storeOp          = VK_ATTACHMENT_STORE_OP_STORE ;
        attachments[0].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE ;
        attachments[0].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE ;
        attachments[0].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED ;
        attachments[0].finalLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ;

        attachments[1].flags            = 0 ;
        attachments[1].format           = vc->swapchain_surface_format_.format ;
        attachments[1].samples          = vc->sample_count_ ;
        attachments[1].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR ;
        attachments[1].storeOp          = VK_ATTACHMENT_STORE_OP_STORE ;
        attachments[1].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE ;
        attachments[1].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE ;
        attachments[1].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED ;
        attachments[1].finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ;

        attachments[2].flags            = 0 ;
        attachments[2].format           = vc->picked_physical_device_->depth_format_ ;
        attachments[2].samples          = VK_SAMPLE_COUNT_1_BIT ;
        attachments[2].loadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE ;
        attachments[2].storeOp          = VK_ATTACHMENT_STORE_OP_STORE ;
        attachments[2].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE ;
        attachments[2].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE ;
        attachments[2].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED ;
        attachments[2].finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ;
    }
    else
    {
        attachments_count               = 2 ;

        attachments[0].flags            = 0 ;
        attachments[0].format           = vc->swapchain_surface_format_.format ;
        attachments[0].samples          = VK_SAMPLE_COUNT_1_BIT ;
        attachments[0].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR ;
        attachments[0].storeOp          = VK_ATTACHMENT_STORE_OP_STORE ;
        attachments[0].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE ;
        attachments[0].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE ;
        attachments[0].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED ;
        attachments[0].finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ;

        attachments[1].flags            = 0 ;
        attachments[1].format           = vc->picked_physical_device_->depth_format_ ;
        attachments[1].samples          = VK_SAMPLE_COUNT_1_BIT ;
        attachments[1].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR ;
        attachments[1].storeOp          = VK_ATTACHMENT_STORE_OP_STORE ;
        attachments[1].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE ;
        attachments[1].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE ;
        attachments[1].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED ;
        attachments[1].finalLayout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ;
    }

    // typedef struct VkAttachmentReference {
    //     uint32_t         attachment;
    //     VkImageLayout    layout;
    // } VkAttachmentReference;
    static VkAttachmentReference color_ar = { 0 } ;
    color_ar.attachment   = 0 ;
    color_ar.layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ;

    static VkAttachmentReference depth_ar = { 0 } ;
    depth_ar.attachment    = 1 ;
    depth_ar.layout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ;

    static VkAttachmentReference color_resolve_ar = { 0 } ;
    color_resolve_ar.attachment = 2 ;
    color_resolve_ar.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ;

    // typedef struct VkSubpassDescription {
    //     VkSubpassDescriptionFlags       flags;
    //     VkPipelineBindPoint             pipelineBindPoint;
    //     uint32_t                        inputAttachmentCount;
    //     const VkAttachmentReference*    pInputAttachments;
    //     uint32_t                        colorAttachmentCount;
    //     const VkAttachmentReference*    pColorAttachments;
    //     const VkAttachmentReference*    pResolveAttachments;
    //     const VkAttachmentReference*    pDepthStencilAttachment;
    //     uint32_t                        preserveAttachmentCount;
    //     const uint32_t*                 pPreserveAttachments;
    // } VkSubpassDescription;
    static VkSubpassDescription sd = { 0 } ;
    sd.flags                        = 0 ;
    sd.pipelineBindPoint            = VK_PIPELINE_BIND_POINT_GRAPHICS ;
    sd.inputAttachmentCount         = 0 ;
    sd.pInputAttachments            = NULL ;
    sd.colorAttachmentCount         = 1 ;
    sd.pColorAttachments            = &color_ar ;
    if(vc->enable_sampling_)
    {
        sd.pResolveAttachments      = &color_resolve_ar ;
    }
    else
    {
        sd.pResolveAttachments      = NULL ;
    }
    sd.pDepthStencilAttachment      = &depth_ar ;
    sd.preserveAttachmentCount      = 0 ;
    sd.pPreserveAttachments         = NULL ;


    // typedef struct VkSubpassDependency {
    //     uint32_t                srcSubpass;
    //     uint32_t                dstSubpass;
    //     VkPipelineStageFlags    srcStageMask;
    //     VkPipelineStageFlags    dstStageMask;
    //     VkAccessFlags           srcAccessMask;
    //     VkAccessFlags           dstAccessMask;
    //     VkDependencyFlags       dependencyFlags;
    // } VkSubpassDependency;
    static VkSubpassDependency sde = { 0 } ;
    sde.srcSubpass       = VK_SUBPASS_EXTERNAL ;
    sde.dstSubpass       = 0 ;
    sde.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ;
    sde.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ;
    sde.srcAccessMask    = 0 ;
    sde.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ;
    sde.dependencyFlags  = 0 ;

    require(attachments_count) ;
    // typedef struct VkRenderPassCreateInfo {
    //     VkStructureType                   sType;
    //     const void*                       pNext;
    //     VkRenderPassCreateFlags           flags;
    //     uint32_t                          attachmentCount;
    //     const VkAttachmentDescription*    pAttachments;
    //     uint32_t                          subpassCount;
    //     const VkSubpassDescription*       pSubpasses;
    //     uint32_t                          dependencyCount;
    //     const VkSubpassDependency*        pDependencies;
    // } VkRenderPassCreateInfo;
    static VkRenderPassCreateInfo rpci = { 0 } ;
    rpci.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO ;
    rpci.pNext              = NULL ;
    rpci.flags              = 0 ;
    rpci.attachmentCount    = attachments_count ;
    rpci.pAttachments       = attachments ;
    rpci.subpassCount       = 1 ;
    rpci.pSubpasses         = &sd ;
    rpci.dependencyCount    = 1 ;
    rpci.pDependencies      = &sde ;

    // VkResult vkCreateRenderPass(
    //     VkDevice                                    device,
    //     const VkRenderPassCreateInfo*               pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkRenderPass*                               pRenderPass);
    if(check_vulkan(vkCreateRenderPass(
                vc->device_
            ,   &rpci
            ,   NULL
            ,   &vc->render_pass_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->render_pass_) ;

    end_timed_block() ;
    return true ;
}


static bool
create_command_pool(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;


    // typedef struct VkCommandPoolCreateInfo {
    //     VkStructureType             sType;
    //     const void*                 pNext;
    //     VkCommandPoolCreateFlags    flags;
    //     uint32_t                    queueFamilyIndex;
    // } VkCommandPoolCreateInfo;
    static VkCommandPoolCreateInfo cpci = { 0 } ;
    cpci.sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO ;
    cpci.pNext              = NULL ;
    cpci.flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT ;
    cpci.queueFamilyIndex   = vc->picked_physical_device_->queue_families_indices_.graphics_family_ ;


    // VkResult vkCreateCommandPool(
    //     VkDevice                                    device,
    //     const VkCommandPoolCreateInfo*              pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkCommandPool*                              pCommandPool);
    if(check_vulkan(vkCreateCommandPool(
                vc->device_
            ,   &cpci
            ,   NULL
            ,   &vc->command_pool_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_command_buffer(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

    // typedef struct VkCommandBufferAllocateInfo {
    //     VkStructureType         sType;
    //     const void*             pNext;
    //     VkCommandPool           commandPool;
    //     VkCommandBufferLevel    level;
    //     uint32_t                commandBufferCount;
    // } VkCommandBufferAllocateInfo;
    static VkCommandBufferAllocateInfo cbai = { 0 } ;
    cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO ;
    cbai.pNext              = NULL ;
    cbai.commandPool        = vc->command_pool_ ;
    cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY ;
    cbai.commandBufferCount = vc->frames_in_flight_count_ ;

    // VkResult vkAllocateCommandBuffers(
    //     VkDevice                                    device,
    //     const VkCommandBufferAllocateInfo*          pAllocateInfo,
    //     VkCommandBuffer*                            pCommandBuffers);
    if(check_vulkan(vkAllocateCommandBuffers(
                vc->device_
            ,   &cbai
            ,   vc->command_buffer_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc->command_buffer_) ;

    end_timed_block() ;
    return true ;
}


static bool
record_command_buffer(
    vulkan_context *    vc
,   VkCommandBuffer     command_buffer
,   uint32_t const      image_index
)
{
    require(vc) ;
    require(command_buffer) ;
    require(image_index < max_vulkan_swapchain_images) ;
    require(image_index < vc->swapchain_images_count_) ;

    begin_timed_block() ;

    // typedef struct VkCommandBufferBeginInfo {
    //     VkStructureType                          sType;
    //     const void*                              pNext;
    //     VkCommandBufferUsageFlags                flags;
    //     const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    // } VkCommandBufferBeginInfo;
    static VkCommandBufferBeginInfo cbbi = { 0 } ;
    cbbi.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO ;
    cbbi.pNext              = NULL ;
    cbbi.flags              = 0 ;
    cbbi.pInheritanceInfo   = NULL ;

    // VkResult vkBeginCommandBuffer(
    //     VkCommandBuffer                             commandBuffer,
    //     const VkCommandBufferBeginInfo*             pBeginInfo);
    if(check_vulkan(vkBeginCommandBuffer(command_buffer, &cbbi)))
    {
        end_timed_block() ;
        return false ;
    }


    // typedef struct VkRenderPassBeginInfo {
    //     VkStructureType        sType;
    //     const void*            pNext;
    //     VkRenderPass           renderPass;
    //     VkFramebuffer          framebuffer;
    //     VkRect2D               renderArea;
    //     uint32_t               clearValueCount;
    //     const VkClearValue*    pClearValues;
    // } VkRenderPassBeginInfo;
    // typedef union VkClearValue {
    //     VkClearColorValue           color;
    //     VkClearDepthStencilValue    depthStencil;
    // } VkClearValue;
    // typedef union VkClearColorValue {
    //     float       float32[4];
    //     int32_t     int32[4];
    //     uint32_t    uint32[4];
    // } VkClearColorValue;
    // typedef struct VkClearDepthStencilValue {
    //     float       depth;
    //     uint32_t    stencil;
    // } VkClearDepthStencilValue;
    static VkClearValue    cv[2] = { 0 } ;
    cv[0].color.float32[0]      = 0.0f ;
    cv[0].color.float32[1]      = 0.0f ;
    cv[0].color.float32[2]      = 0.0f ;
    cv[0].color.float32[3]      = 1.0f ;
    cv[1].depthStencil.depth    = 1.0f ;
    cv[1].depthStencil.stencil  = 0 ;

    static VkRenderPassBeginInfo rpbi = { 0 } ;
    rpbi.sType                      = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO ;
    rpbi.pNext                      = NULL ;
    rpbi.renderPass                 = vc->render_pass_ ;
    rpbi.framebuffer                = vc->framebuffers_[image_index] ;
    rpbi.renderArea.offset.x        = 0 ;
    rpbi.renderArea.offset.y        = 0 ;
    rpbi.renderArea.extent.width    = vc->swapchain_extent_.width ;
    rpbi.renderArea.extent.height   = vc->swapchain_extent_.height ;
    rpbi.clearValueCount            = array_count(cv) ;
    rpbi.pClearValues               = cv ;

    // void vkCmdBeginRenderPass(
    //     VkCommandBuffer                             commandBuffer,
    //     const VkRenderPassBeginInfo*                pRenderPassBegin,
    //     VkSubpassContents                           contents);
    vkCmdBeginRenderPass(command_buffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE) ;

    // void vkCmdBindPipeline(
    //     VkCommandBuffer                             commandBuffer,
    //     VkPipelineBindPoint                         pipelineBindPoint,
    //     VkPipeline                                  pipeline);
    vkCmdBindPipeline(
        command_buffer
    ,   VK_PIPELINE_BIND_POINT_GRAPHICS
    ,   vc->graphics_pipeline_
    ) ;


    VkBuffer vertex_buffers[] = { vc->vertex_buffer_} ;
    VkDeviceSize offsets[] = { 0 } ;

    // void vkCmdBindVertexBuffers(
    //     VkCommandBuffer                             commandBuffer,
    //     uint32_t                                    firstBinding,
    //     uint32_t                                    bindingCount,
    //     const VkBuffer*                             pBuffers,
    //     const VkDeviceSize*                         pOffsets);
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets) ;


    // void vkCmdBindIndexBuffer(
    //     VkCommandBuffer                             commandBuffer,
    //     VkBuffer                                    buffer,
    //     VkDeviceSize                                offset,
    //     VkIndexType                                 indexType);
    vkCmdBindIndexBuffer(
        command_buffer
    ,   vc->index_buffer_
    ,   0
    ,   VK_INDEX_TYPE_UINT16
    ) ;


    static VkViewport viewport = { 0 } ;
    viewport.x          = 0.0f ;
    viewport.y          = 0.0f ;
    viewport.width      = (float) vc->swapchain_extent_.width ;
    viewport.height     = (float) vc->swapchain_extent_.height ;
    viewport.minDepth   = 0.0f ;
    viewport.maxDepth   = 1.0f ;

    // void vkCmdSetViewport(
    //     VkCommandBuffer                             commandBuffer,
    //     uint32_t                                    firstViewport,
    //     uint32_t                                    viewportCount,
    //     const VkViewport*                           pViewports);
    vkCmdSetViewport(command_buffer, 0, 1, &viewport) ;


    static VkRect2D scissor = { 0 } ;
    scissor.offset.x        = 0 ;
    scissor.offset.y        = 0 ;
    scissor.extent.width    = vc->swapchain_extent_.width ;
    scissor.extent.height   = vc->swapchain_extent_.height ;

    // void vkCmdSetScissor(
    //     VkCommandBuffer                             commandBuffer,
    //     uint32_t                                    firstScissor,
    //     uint32_t                                    scissorCount,
    //     const VkRect2D*                             pScissors);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor) ;

    // // void vkCmdDraw(
    // //     VkCommandBuffer                             commandBuffer,
    // //     uint32_t                                    vertexCount,
    // //     uint32_t                                    instanceCount,
    // //     uint32_t                                    firstVertex,
    // //     uint32_t                                    firstInstance);
    // vkCmdDraw(command_buffer, vertices_count, 1, 0, 0) ;

    // void vkCmdBindDescriptorSets(
    //     VkCommandBuffer                             commandBuffer,
    //     VkPipelineBindPoint                         pipelineBindPoint,
    //     VkPipelineLayout                            layout,
    //     uint32_t                                    firstSet,
    //     uint32_t                                    descriptorSetCount,
    //     const VkDescriptorSet*                      pDescriptorSets,
    //     uint32_t                                    dynamicOffsetCount,
    //     const uint32_t*                             pDynamicOffsets);
    // away
    // vkCmdBindDescriptorSets(
    //     command_buffer
    // ,   VK_PIPELINE_BIND_POINT_GRAPHICS
    // ,   vc->pipeline_layout_
    // ,   0
    // ,   1
    // ,   &vc->descriptor_sets_[vc->current_frame_]
    // ,   0
    // ,   NULL
    // ) ;

    // void vkCmdDrawIndexed(
    //     VkCommandBuffer                             commandBuffer,
    //     uint32_t                                    indexCount,
    //     uint32_t                                    instanceCount,
    //     uint32_t                                    firstIndex,
    //     int32_t                                     vertexOffset,
    //     uint32_t                                    firstInstance);
    vkCmdDrawIndexed(command_buffer, indices_count, 1, 0, 0, 0) ;

    // void vkCmdEndRenderPass(
    //     VkCommandBuffer                             commandBuffer);
    vkCmdEndRenderPass(command_buffer) ;

    // VkResult vkEndCommandBuffer(
    //     VkCommandBuffer                             commandBuffer);
    if(check_vulkan(vkEndCommandBuffer(command_buffer)))
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;

}


static bool
create_sync_objects(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;


    // typedef struct VkSemaphoreCreateInfo {
    //     VkStructureType           sType;
    //     const void*               pNext;
    //     VkSemaphoreCreateFlags    flags;
    // } VkSemaphoreCreateInfo;
    static VkSemaphoreCreateInfo sci = { 0 } ;
    sci.sType   = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO ;
    sci.pNext   = NULL ;
    sci.flags   = 0 ;


    // typedef struct VkFenceCreateInfo {
    //     VkStructureType       sType;
    //     const void*           pNext;
    //     VkFenceCreateFlags    flags;
    // } VkFenceCreateInfo;
    static VkFenceCreateInfo fci = { 0 } ;
    fci.sType   = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO ;
    fci.pNext   = NULL ;
    fci.flags   = VK_FENCE_CREATE_SIGNALED_BIT ;

    for(
        uint32_t i = 0
    ;   i < vc->frames_in_flight_count_
    ;   ++i
    )
    {
        // VkResult vkCreateSemaphore(
        //     VkDevice                                    device,
        //     const VkSemaphoreCreateInfo*                pCreateInfo,
        //     const VkAllocationCallbacks*                pAllocator,
        //     VkSemaphore*                                pSemaphore);
        if(check_vulkan(vkCreateSemaphore(
                    vc->device_
                ,   &sci
                ,   NULL
                ,   &vc->image_available_semaphore_[i]
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
        require(vc->image_available_semaphore_[i]) ;

        if(check_vulkan(vkCreateSemaphore(
                    vc->device_
                ,   &sci
                ,   NULL
                ,   &vc->render_finished_semaphore_[i]
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
        require(vc->render_finished_semaphore_[i]) ;


        // VkResult vkCreateFence(
        //     VkDevice                                    device,
        //     const VkFenceCreateInfo*                    pCreateInfo,
        //     const VkAllocationCallbacks*                pAllocator,
        //     VkFence*                                    pFence);
        if(check_vulkan(vkCreateFence(
                    vc->device_
                ,   &fci
                ,   NULL
                ,   &vc->in_flight_fence_[i]
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
        require(vc->in_flight_fence_[i]) ;
    }

    end_timed_block() ;
    return true ;
}


static bool
draw_frame(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

    require(vc->current_frame_ < max_vulkan_frames_in_flight) ;
    require(vc->current_frame_ < vc->frames_in_flight_count_) ;

    // VkResult vkWaitForFences(
    //     VkDevice                                    device,
    //     uint32_t                                    fenceCount,
    //     const VkFence*                              pFences,
    //     VkBool32                                    waitAll,
    //     uint64_t                                    timeout);
    if(check_vulkan(vkWaitForFences(
                vc->device_
            ,   1
            ,   &vc->in_flight_fence_[vc->current_frame_]
            ,   VK_TRUE
            ,   UINT64_MAX
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    uint32_t image_index = 0 ;

    // VkResult vkAcquireNextImageKHR(
    //     VkDevice                                    device,
    //     VkSwapchainKHR                              swapchain,
    //     uint64_t                                    timeout,
    //     VkSemaphore                                 semaphore,
    //     VkFence                                     fence,
    //     uint32_t*                                   pImageIndex);
    VkResult const aquire_ok = vkAcquireNextImageKHR(
        vc->device_
    ,   vc->swapchain_
    ,   UINT64_MAX
    ,   vc->image_available_semaphore_[vc->current_frame_]
    ,   VK_NULL_HANDLE
    ,   &image_index
    ) ;

    check(
        aquire_ok == VK_SUCCESS
    ||  aquire_ok == VK_SUBOPTIMAL_KHR
    ||  aquire_ok == VK_ERROR_OUT_OF_DATE_KHR
    ) ;

    if(aquire_ok == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if(check(recreate_swapchain(vc)))
        {
            end_timed_block() ;
            return false ;
        }
        end_timed_block() ;
        return true ;
    }

    update_uniform_buffer(vc, vc->current_frame_) ;

    // VkResult vkResetFences(
    //     VkDevice                                    device,
    //     uint32_t                                    fenceCount,
    //     const VkFence*                              pFences);
    if(check_vulkan(vkResetFences(
                vc->device_
            ,   1
            ,   &vc->in_flight_fence_[vc->current_frame_]
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }



    // VkResult vkResetCommandBuffer(
    //     VkCommandBuffer                             commandBuffer,
    //     VkCommandBufferResetFlags                   flags);
    if(check_vulkan(vkResetCommandBuffer(
                vc->command_buffer_[vc->current_frame_]
            ,   0
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(record_command_buffer(
                vc
            ,   vc->command_buffer_[vc->current_frame_]
            ,   image_index
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    VkSemaphore wait_semaphores[] = {
        vc->image_available_semaphore_[vc->current_frame_]
    } ;

    VkSemaphore signal_semaphores[] = {
        vc->render_finished_semaphore_[vc->current_frame_]
    } ;

    // typedef enum VkPipelineStageFlagBits {
    //     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT = 0x00000001,
    //     VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT = 0x00000002,
    //     VK_PIPELINE_STAGE_VERTEX_INPUT_BIT = 0x00000004,
    //     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT = 0x00000008,
    //     VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT = 0x00000010,
    //     VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020,
    //     VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT = 0x00000040,
    //     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT = 0x00000080,
    //     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT = 0x00000100,
    //     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT = 0x00000200,
    //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400,
    //     VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT = 0x00000800,
    //     VK_PIPELINE_STAGE_TRANSFER_BIT = 0x00001000,
    //     VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT = 0x00002000,
    //     VK_PIPELINE_STAGE_HOST_BIT = 0x00004000,
    //     VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT = 0x00008000,
    //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT = 0x00010000,
    // // Provided by VK_VERSION_1_3
    //     VK_PIPELINE_STAGE_NONE = 0,
    // // Provided by VK_EXT_transform_feedback
    //     VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT = 0x01000000,
    // // Provided by VK_EXT_conditional_rendering
    //     VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT = 0x00040000,
    // // Provided by VK_KHR_acceleration_structure
    //     VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR = 0x02000000,
    // // Provided by VK_KHR_ray_tracing_pipeline
    //     VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR = 0x00200000,
    // // Provided by VK_EXT_fragment_density_map
    //     VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT = 0x00800000,
    // // Provided by VK_KHR_fragment_shading_rate
    //     VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR = 0x00400000,
    // // Provided by VK_NV_device_generated_commands
    //     VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV = 0x00020000,
    // // Provided by VK_EXT_mesh_shader
    //     VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT = 0x00080000,
    // // Provided by VK_EXT_mesh_shader
    //     VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT = 0x00100000,
    // // Provided by VK_NV_shading_rate_image
    //     VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
    // // Provided by VK_NV_ray_tracing
    //     VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
    // // Provided by VK_NV_ray_tracing
    //     VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
    // // Provided by VK_NV_mesh_shader
    //     VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV = VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
    // // Provided by VK_NV_mesh_shader
    //     VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV = VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT,
    // // Provided by VK_KHR_synchronization2
    //     VK_PIPELINE_STAGE_NONE_KHR = VK_PIPELINE_STAGE_NONE,
    // } VkPipelineStageFlagBits;


    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    } ;


    // typedef struct VkSubmitInfo {
    //     VkStructureType                sType;
    //     const void*                    pNext;
    //     uint32_t                       waitSemaphoreCount;
    //     const VkSemaphore*             pWaitSemaphores;
    //     const VkPipelineStageFlags*    pWaitDstStageMask;
    //     uint32_t                       commandBufferCount;
    //     const VkCommandBuffer*         pCommandBuffers;
    //     uint32_t                       signalSemaphoreCount;
    //     const VkSemaphore*             pSignalSemaphores;
    // } VkSubmitInfo;
    static VkSubmitInfo si = { 0 } ;
    si.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO ;
    si.pNext                    = NULL ;
    si.waitSemaphoreCount       = array_count(wait_semaphores) ;
    si.pWaitSemaphores          = wait_semaphores ;
    si.pWaitDstStageMask        = wait_stages ;
    si.commandBufferCount       = 1 ;
    si.pCommandBuffers          = &vc->command_buffer_[vc->current_frame_] ;
    si.signalSemaphoreCount     = array_count(signal_semaphores) ;
    si.pSignalSemaphores        = signal_semaphores ;

    // VkResult vkQueueSubmit(
    //     VkQueue                                     queue,
    //     uint32_t                                    submitCount,
    //     const VkSubmitInfo*                         pSubmits,
    //     VkFence                                     fence);
    if(check_vulkan(vkQueueSubmit(
                vc->graphics_queue_
            ,   1
            ,   &si
            ,   vc->in_flight_fence_[vc->current_frame_]
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    VkSwapchainKHR swap_chains[] = {
        vc->swapchain_
    } ;


    // typedef struct VkPresentInfoKHR {
    //     VkStructureType          sType;
    //     const void*              pNext;
    //     uint32_t                 waitSemaphoreCount;
    //     const VkSemaphore*       pWaitSemaphores;
    //     uint32_t                 swapchainCount;
    //     const VkSwapchainKHR*    pSwapchains;
    //     const uint32_t*          pImageIndices;
    //     VkResult*                pResults;
    // } VkPresentInfoKHR;
    static VkPresentInfoKHR pi = { 0 } ;
    pi.sType                = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR ;
    pi.pNext                = NULL ;
    pi.waitSemaphoreCount   = array_count(signal_semaphores) ;
    pi.pWaitSemaphores      = signal_semaphores ;
    pi.swapchainCount       = array_count(swap_chains) ;
    pi.pSwapchains          = swap_chains ;
    pi.pImageIndices        = &image_index ;
    pi.pResults             = NULL ;

    // VkResult vkQueuePresentKHR(
    //     VkQueue                                     queue,
    //     const VkPresentInfoKHR*                     pPresentInfo);
    VkResult const present_ok = vkQueuePresentKHR(vc->graphics_queue_, &pi) ;
    check(
        present_ok == VK_SUCCESS
    ||  present_ok == VK_SUBOPTIMAL_KHR
    ||  present_ok == VK_ERROR_OUT_OF_DATE_KHR
    ) ;

    if(
        present_ok == VK_ERROR_OUT_OF_DATE_KHR
    ||  present_ok == VK_SUBOPTIMAL_KHR
    ||  vc->resizing_
    )
    {
        vc->resizing_ = VK_FALSE ;
        if(check(recreate_swapchain(vc)))
        {
            end_timed_block() ;
            return false ;
        }
    }

    log_debug("vc->current_frame_=%d, image_index=%d", vc->current_frame_, image_index) ;

    vc->current_frame_ = (vc->current_frame_ + 1) % vc->frames_in_flight_count_ ;

    end_timed_block() ;
    return true ;
}


static bool
find_memory_type(
    uint32_t *                                  out_mem_type
,   VkPhysicalDeviceMemoryProperties const *    mp
,   uint32_t const                              type_filter
,   VkMemoryPropertyFlags const                 prop
)
{
    require(mp) ;
    begin_timed_block() ;

    for(
        uint32_t i = 0
    ;   i < mp->memoryTypeCount
    ;   ++i
    )
    {
        bool const filter_ok = type_filter & (1 << i) ;
        bool const prop_ok = (mp->memoryTypes[i].propertyFlags & prop) == prop ;
        bool const all_ok =
            filter_ok
        &&  prop_ok
        ;
        if(all_ok)
        {
            *out_mem_type = i ;
            end_timed_block() ;
            return true ;
        }
    }

    end_timed_block() ;
    require(0) ;
    return false ;
}


static bool
create_buffer(
    VkBuffer *                                  out_buffer
,   VkDeviceMemory *                            out_buffer_memory
,   VkDevice const                              device
,   VkPhysicalDeviceMemoryProperties const *    pdmp
,   uint32_t const                              size
,   VkBufferUsageFlags const                    usage
,   VkMemoryPropertyFlags const                 mem_prop
)
{
    require(out_buffer) ;
    require(out_buffer_memory) ;
    require(device) ;
    require(pdmp) ;
    require(size) ;
    begin_timed_block() ;


    // typedef struct VkBufferCreateInfo {
    //     VkStructureType        sType;
    //     const void*            pNext;
    //     VkBufferCreateFlags    flags;
    //     VkDeviceSize           size;
    //     VkBufferUsageFlags     usage;
    //     VkSharingMode          sharingMode;
    //     uint32_t               queueFamilyIndexCount;
    //     const uint32_t*        pQueueFamilyIndices;
    // } VkBufferCreateInfo;
    static VkBufferCreateInfo bci = { 0 } ;
    bci.sType                   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO ;
    bci.pNext                   = NULL ;
    bci.flags                   = 0 ;
    bci.size                    = size ;
    bci.usage                   = usage ;
    bci.sharingMode             = VK_SHARING_MODE_EXCLUSIVE ;
    bci.queueFamilyIndexCount   = 0 ;
    bci.pQueueFamilyIndices     = NULL ;

    // VkResult vkCreateBuffer(
    //     VkDevice                                    device,
    //     const VkBufferCreateInfo*                   pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkBuffer*                                   pBuffer);
    if(check_vulkan(vkCreateBuffer(
                device
            ,   &bci
            ,   NULL
            ,   out_buffer
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_buffer) ;


    // typedef struct VkMemoryRequirements {
    //     VkDeviceSize    size;
    //     VkDeviceSize    alignment;
    //     uint32_t        memoryTypeBits;
    // } VkMemoryRequirements;
    VkMemoryRequirements mem_requirements = { 0 } ;

    // void vkGetBufferMemoryRequirements(
    //     VkDevice                                    device,
    //     VkBuffer                                    buffer,
    //     VkMemoryRequirements*                       pMemoryRequirements);
    vkGetBufferMemoryRequirements(device, *out_buffer, &mem_requirements) ;

    uint32_t desired_memory_type = 0 ;
    if(check(find_memory_type(
                &desired_memory_type
            ,   pdmp
            ,   mem_requirements.memoryTypeBits
            ,   mem_prop
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    // typedef struct VkMemoryAllocateInfo {
    //     VkStructureType    sType;
    //     const void*        pNext;
    //     VkDeviceSize       allocationSize;
    //     uint32_t           memoryTypeIndex;
    // } VkMemoryAllocateInfo;
    static VkMemoryAllocateInfo mai = { 0 } ;
    mai.sType               = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO ;
    mai.pNext               = NULL ;
    mai.allocationSize      = mem_requirements.size ;
    mai.memoryTypeIndex     = desired_memory_type ;


    // VkResult vkAllocateMemory(
    //     VkDevice                                    device,
    //     const VkMemoryAllocateInfo*                 pAllocateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkDeviceMemory*                             pMemory);
    if(check_vulkan(vkAllocateMemory(
                device
            ,   &mai
            ,   NULL
            ,   out_buffer_memory
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_buffer_memory) ;


    // VkResult vkBindBufferMemory(
    //     VkDevice                                    device,
    //     VkBuffer                                    buffer,
    //     VkDeviceMemory                              memory,
    //     VkDeviceSize                                memoryOffset);
    if(check_vulkan(vkBindBufferMemory(
                device
            ,   *out_buffer
            ,   *out_buffer_memory
            ,   0
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    end_timed_block() ;
    return true ;
}


static bool
copy_buffer(
    vulkan_context *    vc
,   VkBuffer const      src_buffer
,   VkBuffer const      dst_buffer
,   VkDeviceSize const  size
)
{
    require(vc) ;
    require(vc->device_) ;
    require(vc->command_pool_) ;
    require(vc->graphics_queue_) ;
    require(src_buffer) ;
    require(dst_buffer) ;
    require(size) ;

    begin_timed_block() ;

    VkCommandBuffer command_buffer = NULL ;
    if(check(begin_single_time_commands(&command_buffer, vc->device_, vc->command_pool_)))
    {
        end_timed_block() ;
        return false ;
    }
    require(command_buffer) ;


    // typedef struct VkBufferCopy {
    //     VkDeviceSize    srcOffset;
    //     VkDeviceSize    dstOffset;
    //     VkDeviceSize    size;
    // } VkBufferCopy;
    static VkBufferCopy copy_region = { 0 } ;
    copy_region.srcOffset   = 0 ;
    copy_region.dstOffset   = 0 ;
    copy_region.size        = size ;

    // void vkCmdCopyBuffer(
    //     VkCommandBuffer                             commandBuffer,
    //     VkBuffer                                    srcBuffer,
    //     VkBuffer                                    dstBuffer,
    //     uint32_t                                    regionCount,
    //     const VkBufferCopy*                         pRegions);
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region) ;

    if(check(end_single_time_commands(
                vc->device_
            ,   vc->command_pool_
            ,   vc->graphics_queue_
            ,   command_buffer
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
copy_buffer_to_image(
    VkDevice const      device
,   VkCommandPool const command_pool
,   VkQueue const       graphics_queue
,   VkBuffer const      buffer
,   VkImage const       image
,   uint32_t const      width
,   uint32_t const      height
)
{
    require(device) ;
    require(command_pool) ;
    require(graphics_queue) ;
    require(buffer) ;
    require(image) ;
    require(width) ;
    require(height) ;

    begin_timed_block() ;

    VkCommandBuffer command_buffer = NULL ;
    if(check(begin_single_time_commands(
                &command_buffer
            ,   device
            ,   command_pool
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    // typedef struct VkBufferImageCopy {
    //     VkDeviceSize                bufferOffset;
    //     uint32_t                    bufferRowLength;
    //     uint32_t                    bufferImageHeight;
    //     VkImageSubresourceLayers    imageSubresource;
    //     VkOffset3D                  imageOffset;
    //     VkExtent3D                  imageExtent;
    // } VkBufferImageCopy;
    // typedef struct VkImageSubresourceLayers {
    //     VkImageAspectFlags    aspectMask;
    //     uint32_t              mipLevel;
    //     uint32_t              baseArrayLayer;
    //     uint32_t              layerCount;
    // } VkImageSubresourceLayers;
    static VkBufferImageCopy bic = { 0 } ;
    bic.bufferOffset                        = 0 ;
    bic.bufferRowLength                     = 0 ;
    bic.bufferImageHeight                   = 0 ;
    bic.imageSubresource.aspectMask         = VK_IMAGE_ASPECT_COLOR_BIT ;
    bic.imageSubresource.mipLevel           = 0 ;
    bic.imageSubresource.baseArrayLayer     = 0 ;
    bic.imageSubresource.layerCount         = 1 ;
    bic.imageOffset.x                       = 0.0f ;
    bic.imageOffset.y                       = 0.0f ;
    bic.imageOffset.z                       = 0.0f ;
    bic.imageExtent.width                   = width ;
    bic.imageExtent.height                  = height ;
    bic.imageExtent.depth                   = 1 ;

    // void vkCmdCopyBufferToImage(
    //     VkCommandBuffer                             commandBuffer,
    //     VkBuffer                                    srcBuffer,
    //     VkImage                                     dstImage,
    //     VkImageLayout                               dstImageLayout,
    //     uint32_t                                    regionCount,
    //     const VkBufferImageCopy*                    pRegions);
    vkCmdCopyBufferToImage(
        command_buffer
    ,   buffer
    ,   image
    ,   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    ,   1
    ,   &bic
    ) ;

    if(check(end_single_time_commands(
                device
            ,   command_pool
            ,   graphics_queue
            ,   command_buffer
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_vertex_buffer(
    VkBuffer *          out_buffer
,   VkDeviceMemory *    out_buffer_memory
,   vulkan_context *    vc
)
{
    require(out_buffer) ;
    require(out_buffer_memory) ;
    require(vc) ;
    require(vc_->device_) ;
    begin_timed_block() ;

    VkDeviceSize const buffer_size = vertices_size ;

    VkBuffer staging_buffer = NULL ;
    VkDeviceMemory staging_buffer_memory = NULL ;

    if(check(create_buffer(
                &staging_buffer
            ,   &staging_buffer_memory
            ,   vc->device_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   buffer_size
            ,   VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            ,   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(staging_buffer) ;
    require(staging_buffer_memory) ;

    void * data = NULL ;

    // VkResult vkMapMemory(
    //     VkDevice                                    device,
    //     VkDeviceMemory                              memory,
    //     VkDeviceSize                                offset,
    //     VkDeviceSize                                size,
    //     VkMemoryMapFlags                            flags,
    //     void**                                      ppData);
    if(check_vulkan(vkMapMemory(
                vc->device_
            ,   staging_buffer_memory
            ,   0
            ,   buffer_size
            ,   0
            ,   &data
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(data) ;

    SDL_memcpy(data, vertices, buffer_size) ;

    // void vkUnmapMemory(
    //     VkDevice                                    device,
    //     VkDeviceMemory                              memory);
    vkUnmapMemory(vc->device_, staging_buffer_memory) ;

    if(check(create_buffer(
                out_buffer
            ,   out_buffer_memory
            ,   vc->device_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   buffer_size
            ,   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
            ,   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_buffer) ;
    require(*out_buffer_memory) ;

    if(check(copy_buffer(vc, staging_buffer, *out_buffer, buffer_size)))
    {
        end_timed_block() ;
        return false ;
    }

    vkDestroyBuffer(vc->device_, staging_buffer, NULL) ;
    vkFreeMemory(vc->device_, staging_buffer_memory, NULL) ;

    end_timed_block() ;
    return true ;
}


static bool
create_index_buffer(
    VkBuffer *          out_buffer
,   VkDeviceMemory *    out_buffer_memory
,   vulkan_context *    vc
)
{
    require(out_buffer) ;
    require(out_buffer_memory) ;
    require(vc) ;
    require(vc_->device_) ;
    begin_timed_block() ;

    VkDeviceSize const buffer_size = indices_size ;
    VkBuffer staging_buffer = NULL ;
    VkDeviceMemory staging_buffer_memory = NULL ;

    if(check(create_buffer(
                &staging_buffer
            ,   &staging_buffer_memory
            ,   vc->device_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   buffer_size
            ,   VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            ,   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(staging_buffer) ;
    require(staging_buffer_memory) ;

    void * data = NULL ;

    // VkResult vkMapMemory(
    //     VkDevice                                    device,
    //     VkDeviceMemory                              memory,
    //     VkDeviceSize                                offset,
    //     VkDeviceSize                                size,
    //     VkMemoryMapFlags                            flags,
    //     void**                                      ppData);
    if(check_vulkan(vkMapMemory(
                vc->device_
            ,   staging_buffer_memory
            ,   0
            ,   buffer_size
            ,   0
            ,   &data
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(data) ;

    SDL_memcpy(data, indices, buffer_size) ;

    // void vkUnmapMemory(
    //     VkDevice                                    device,
    //     VkDeviceMemory                              memory);
    vkUnmapMemory(vc->device_, staging_buffer_memory) ;

    if(check(create_buffer(
                out_buffer
            ,   out_buffer_memory
            ,   vc->device_
            ,   &vc->picked_physical_device_->memory_properties_
            ,   buffer_size
            ,   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            ,   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_buffer) ;
    require(*out_buffer_memory) ;

    if(check(copy_buffer(vc, staging_buffer, *out_buffer, buffer_size)))
    {
        end_timed_block() ;
        return false ;
    }

    vkDestroyBuffer(vc->device_, staging_buffer, NULL) ;
    vkFreeMemory(vc->device_, staging_buffer_memory, NULL) ;

    end_timed_block() ;
    return true ;
}



static bool
create_image(
    VkImage *                                   out_image
,   VkDeviceMemory *                            out_image_memory
,   VkDevice const                              device
,   VkPhysicalDeviceMemoryProperties const *    pdmp
,   uint32_t const                              width
,   uint32_t const                              height
,   uint32_t const                              mip_levels
,   VkSampleCountFlagBits const                 msaa_sample_count
,   VkFormat const                              format
,   VkImageTiling const                         tiling
,   VkImageUsageFlags const                     usage
,   VkMemoryPropertyFlags const                 mem_prop_flags
)
{
    require(out_image) ;
    require(out_image_memory) ;
    require(device) ;
    require(pdmp) ;
    begin_timed_block() ;

    // typedef struct VkImageCreateInfo {
    //     VkStructureType          sType;
    //     const void*              pNext;
    //     VkImageCreateFlags       flags;
    //     VkImageType              imageType;
    //     VkFormat                 format;
    //     VkExtent3D               extent;
    //     uint32_t                 mipLevels;
    //     uint32_t                 arrayLayers;
    //     VkSampleCountFlagBits    samples;
    //     VkImageTiling            tiling;
    //     VkImageUsageFlags        usage;
    //     VkSharingMode            sharingMode;
    //     uint32_t                 queueFamilyIndexCount;
    //     const uint32_t*          pQueueFamilyIndices;
    //     VkImageLayout            initialLayout;
    // } VkImageCreateInfo;
    static VkImageCreateInfo ici = { 0 } ;
    ici.sType                       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO ;
    ici.pNext                       = NULL ;
    ici.flags                       = 0 ;
    ici.imageType                   = VK_IMAGE_TYPE_2D ;
    ici.format                      = format ;
    ici.extent.width                = width ;
    ici.extent.height               = height ;
    ici.extent.depth                = 1 ;
    ici.mipLevels                   = mip_levels ;
    ici.arrayLayers                 = 1 ;
    ici.samples                     = msaa_sample_count ;
    ici.tiling                      = tiling ;
    ici.usage                       = usage ;
    ici.sharingMode                 = VK_SHARING_MODE_EXCLUSIVE ;
    ici.queueFamilyIndexCount       = 0 ;
    ici.pQueueFamilyIndices         = NULL ;
    ici.initialLayout               = VK_IMAGE_LAYOUT_UNDEFINED ;


    // VkResult vkCreateImage(
    //     VkDevice                                    device,
    //     const VkImageCreateInfo*                    pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkImage*                                    pImage);
    if(check_vulkan(vkCreateImage(
                device
            ,   &ici
            ,   NULL
            ,   out_image
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_image) ;

    VkMemoryRequirements mem_requirements = { 0 } ;
    vkGetImageMemoryRequirements(device, *out_image, &mem_requirements) ;

    uint32_t desired_memory_type = 0 ;
    if(check(find_memory_type(
                &desired_memory_type
            ,   pdmp
            ,   mem_requirements.memoryTypeBits
            ,   mem_prop_flags
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    static VkMemoryAllocateInfo mai = { 0 } ;
    mai.sType               = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO ;
    mai.pNext               = NULL ;
    mai.allocationSize      = mem_requirements.size ;
    mai.memoryTypeIndex     = desired_memory_type ;

    if(check_vulkan(vkAllocateMemory(
                device
            ,   &mai
            ,   NULL
            ,   out_image_memory
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_image_memory) ;

    if(check_vulkan(vkBindImageMemory(
                device
            ,   *out_image
            ,   *out_image_memory
            ,   0
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}




static bool
generate_mipmaps(
    VkDevice const      device
,   VkCommandPool const command_pool
,   VkQueue const       graphics_queue
,   VkImage const       image
,   int32_t const       width
,   int32_t const       height
,   uint32_t const      mip_levels
)
{
    require(device) ;
    require(command_pool) ;
    require(graphics_queue) ;
    require(image) ;
    require(width) ;
    require(height) ;
    require(mip_levels) ;
    begin_timed_block() ;

    // if(check(vc->picked_physical_device_->swapchain_support_details_.formats_properties_->optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    // {
    //     end_timed_block() ;
    //     return false ;
    // }

    VkCommandBuffer command_buffer = NULL ;
    if(check(begin_single_time_commands(
                &command_buffer
            ,   device
            ,   command_pool
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    static VkImageMemoryBarrier imb = { 0 } ;
    imb.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER ;
    imb.pNext                           = NULL ;
    imb.srcAccessMask                   = 0 ;
    imb.dstAccessMask                   = 0 ;
    imb.oldLayout                       = 0 ;
    imb.newLayout                       = 0 ;
    imb.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED ;
    imb.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED ;
    imb.image                           = image ;
    imb.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT ;
    imb.subresourceRange.baseMipLevel   = 0 ;
    imb.subresourceRange.levelCount     = 1 ;
    imb.subresourceRange.baseArrayLayer = 0 ;
    imb.subresourceRange.layerCount     = 1 ;

    int32_t mip_width   = width ;
    int32_t mip_height  = height ;

    for(
        uint32_t i = 1
    ;   i < mip_levels
    ;   ++i
    )
    {
        imb.subresourceRange.baseMipLevel   = i - 1;
        imb.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imb.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imb.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        imb.dstAccessMask                   = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            command_buffer
        ,   VK_PIPELINE_STAGE_TRANSFER_BIT
        ,   VK_PIPELINE_STAGE_TRANSFER_BIT
        ,   0
        ,   0
        ,   NULL
        ,   0
        ,   NULL
        ,   1
        ,   &imb
        ) ;


        // typedef struct VkImageBlit {
        //     VkImageSubresourceLayers    srcSubresource;
        //     VkOffset3D                  srcOffsets[2];
        //     VkImageSubresourceLayers    dstSubresource;
        //     VkOffset3D                  dstOffsets[2];
        // } VkImageBlit;
        // typedef struct VkImageSubresourceLayers {
        //     VkImageAspectFlags    aspectMask;
        //     uint32_t              mipLevel;
        //     uint32_t              baseArrayLayer;
        //     uint32_t              layerCount;
        // } VkImageSubresourceLayers;
        // typedef struct VkOffset3D {
        //     int32_t    x;
        //     int32_t    y;
        //     int32_t    z;
        // } VkOffset3D;

        VkImageBlit ib = { 0 } ;
        ib.srcSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT ;
        ib.srcSubresource.mipLevel          = i - 1 ;
        ib.srcSubresource.baseArrayLayer    = 0 ;
        ib.srcSubresource.layerCount        = 1 ;
        ib.srcOffsets[0].x                  = 0 ;
        ib.srcOffsets[0].y                  = 0 ;
        ib.srcOffsets[0].z                  = 0 ;
        ib.srcOffsets[1].x                  = mip_width ;
        ib.srcOffsets[1].y                  = mip_height ;
        ib.srcOffsets[1].z                  = 1 ;
        ib.dstSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT ;
        ib.dstSubresource.mipLevel          = i ;
        ib.dstSubresource.baseArrayLayer    = 0 ;
        ib.dstSubresource.layerCount        = 1 ;
        ib.dstOffsets[0].x                  = 0 ;
        ib.dstOffsets[0].y                  = 0 ;
        ib.dstOffsets[0].z                  = 0 ;
        ib.dstOffsets[1].x                  = mip_width > 1 ? mip_width / 2 : 1 ;
        ib.dstOffsets[1].y                  = mip_height > 1 ? mip_height / 2 : 1 ;
        ib.dstOffsets[1].z                  = 1 ;

        // void vkCmdBlitImage(
        //     VkCommandBuffer                             commandBuffer,
        //     VkImage                                     srcImage,
        //     VkImageLayout                               srcImageLayout,
        //     VkImage                                     dstImage,
        //     VkImageLayout                               dstImageLayout,
        //     uint32_t                                    regionCount,
        //     const VkImageBlit*                          pRegions,
        //     VkFilter                                    filter);
        vkCmdBlitImage(
            command_buffer
        ,   image
        ,   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        ,   image
        ,   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        ,   1
        ,   &ib
        ,   VK_FILTER_LINEAR
        ) ;

        imb.oldLayout       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ;
        imb.newLayout       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ;
        imb.srcAccessMask   = VK_ACCESS_TRANSFER_READ_BIT ;
        imb.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT ;

        vkCmdPipelineBarrier(
            command_buffer
        ,   VK_PIPELINE_STAGE_TRANSFER_BIT
        ,   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        ,   0
        ,   0
        ,   NULL
        ,   0
        ,   NULL
        ,   1
        ,   &imb
        ) ;


        if(mip_width > 1)
        {
            mip_width /= 2 ;
        }

        if(mip_height > 1)
        {
            mip_height /= 2 ;
        }

    }

    imb.subresourceRange.baseMipLevel   = mip_levels - 1 ;
    imb.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ;
    imb.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ;
    imb.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT ;
    imb.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT ;

    vkCmdPipelineBarrier(
        command_buffer
    ,   VK_PIPELINE_STAGE_TRANSFER_BIT
    ,   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
    ,   0
    ,   0
    ,   NULL
    ,   0
    ,   NULL
    ,   1
    ,   &imb
    ) ;

    if(check(end_single_time_commands(
                device
            ,   command_pool
            ,   graphics_queue
            ,   command_buffer
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}

//away
// static bool
// create_texture_image(
//     vulkan_context *    vc
// )
// {
//     require(vc) ;
//     require(vc->device_) ;
//     begin_timed_block() ;

//     int tx_width    = 0 ;
//     int tx_height   = 0 ;
//     int tx_channels = 0 ;

//     stbi_uc * pixels = NULL ;

//     if(check(pixels = stbi_load(
//                 "ass/textures/statue-1275469_1280.jpg"
//             ,   &tx_width
//             ,   &tx_height
//             ,   &tx_channels
//             ,   STBI_rgb_alpha
//             )
//         )
//     )
//     {
//         end_timed_block() ;
//         return false ;
//     }
//     require(pixels) ;
//     require(tx_width > 0) ;
//     require(tx_height > 0) ;
//     require(tx_channels > 0) ;

//     vc->texture_mip_levels_ = calc_mip_levels(tx_width, tx_height) ;
//     log_debug_u32(tx_width) ;
//     log_debug_u32(tx_height) ;
//     log_debug_u32(vc->texture_mip_levels_) ;



//     VkDeviceSize const image_size = tx_width * tx_height * 4 ;

//     VkBuffer staging_buffer                 = NULL ;
//     VkDeviceMemory staging_buffer_memory    = NULL ;

//     if(check(create_buffer(
//                 &staging_buffer
//             ,   &staging_buffer_memory
//             ,   vc->device_
//             ,   &vc->picked_physical_device_->memory_properties_
//             ,   image_size
//             ,   VK_BUFFER_USAGE_TRANSFER_SRC_BIT
//             ,   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
//             )
//         )
//     )
//     {
//         end_timed_block() ;
//         return false ;
//     }
//     require(staging_buffer) ;
//     require(staging_buffer_memory) ;

//     void * data = NULL ;

//     if(check_vulkan(vkMapMemory(
//                 vc->device_
//             ,   staging_buffer_memory
//             ,   0
//             ,   image_size
//             ,   0
//             ,   &data
//             )
//         )
//     )
//     {
//         end_timed_block() ;
//         return false ;
//     }
//     require(data) ;

//     SDL_memcpy(data, pixels, image_size) ;

//     vkUnmapMemory(vc->device_, staging_buffer_memory) ;

//     stbi_image_free(pixels) ;

//     if(check(create_image(
//                 &vc->texture_image_
//             ,   &vc->texture_image_memory_
//             ,   vc
//             ,   tx_width
//             ,   tx_height
//             ,   vc->texture_mip_levels_
//             ,   VK_SAMPLE_COUNT_1_BIT
//             ,   VK_FORMAT_R8G8B8A8_SRGB
//             ,   VK_IMAGE_TILING_OPTIMAL
//             ,   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
//             ,   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
//             )
//         )
//     )
//     {
//         end_timed_block() ;
//         return false ;
//     }

//     if(check(transition_image_layout(
//                 vc
//             ,   vc->texture_image_
//             ,   VK_FORMAT_R8G8B8A8_SRGB
//             ,   VK_IMAGE_LAYOUT_UNDEFINED
//             ,   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
//             ,   vc->texture_mip_levels_
//             )
//         )
//     )
//     {
//         end_timed_block() ;
//         return false ;
//     }

//     if(check(copy_buffer_to_image(
//                 vc
//             ,   staging_buffer
//             ,   vc->texture_image_
//             ,   tx_width
//             ,   tx_height
//             )
//         )
//     )
//     {
//         end_timed_block() ;
//         return false ;
//     }

//     if(check(generate_mipmaps(
//                 vc
//             ,   vc->texture_image_
//             ,   tx_width
//             ,   tx_height
//             ,   vc->texture_mip_levels_
//             )
//         )
//     )
//     {
//         end_timed_block() ;
//         return false ;
//     }


//     // if(check(transition_image_layout(
//     //             vc
//     //         ,   vc->texture_image_
//     //         ,   VK_FORMAT_R8G8B8A8_SRGB
//     //         ,   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
//     //         ,   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
//     //         ,   vc->texture_mip_levels_
//     //         )
//     //     )
//     // )
//     // {
//     //     end_timed_block() ;
//     //     return false ;
//     // }

//     vkDestroyBuffer(vc->device_, staging_buffer, NULL) ;
//     vkFreeMemory(vc->device_, staging_buffer_memory, NULL) ;

//     end_timed_block() ;
//     return true ;

// }


static bool
create_texture_image_view(
    vulkan_context *    vc
)
{
    require(vc) ;
    require(vc->device_) ;
    begin_timed_block() ;

    if(check(create_image_views(
                &vc->texture_image_view_
            ,   &vc->texture_image_
            ,   1
            ,   vc->device_
            ,   VK_FORMAT_R8G8B8A8_SRGB
            ,   VK_IMAGE_ASPECT_COLOR_BIT
            ,   vc->texture_mip_levels_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
create_texture_sampler(
    vulkan_context *    vc
)
{
    require(vc) ;
    require(vc->device_) ;
    begin_timed_block() ;

    // typedef struct VkSamplerCreateInfo {
    //     VkStructureType         sType;
    //     const void*             pNext;
    //     VkSamplerCreateFlags    flags;
    //     VkFilter                magFilter;
    //     VkFilter                minFilter;
    //     VkSamplerMipmapMode     mipmapMode;
    //     VkSamplerAddressMode    addressModeU;
    //     VkSamplerAddressMode    addressModeV;
    //     VkSamplerAddressMode    addressModeW;
    //     float                   mipLodBias;
    //     VkBool32                anisotropyEnable;
    //     float                   maxAnisotropy;
    //     VkBool32                compareEnable;
    //     VkCompareOp             compareOp;
    //     float                   minLod;
    //     float                   maxLod;
    //     VkBorderColor           borderColor;
    //     VkBool32                unnormalizedCoordinates;
    // } VkSamplerCreateInfo;
    static VkSamplerCreateInfo sci = { 0 } ;
    sci.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO ;
    sci.pNext                   = NULL ;
    sci.flags                   = 0 ;
    sci.magFilter               = VK_FILTER_LINEAR ;
    sci.minFilter               = VK_FILTER_LINEAR ;
    sci.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR ;
    sci.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT ;
    sci.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT ;
    sci.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT ;
    sci.mipLodBias              = 0.0f ;
    sci.anisotropyEnable        = VK_TRUE ;
    sci.maxAnisotropy           = vc_->desired_sampler_aniso_ ;
    sci.compareEnable           = VK_FALSE ;
    sci.compareOp               = VK_COMPARE_OP_ALWAYS ;
    sci.minLod                  = 0.0f ;
    sci.maxLod                  = (float) vc->texture_mip_levels_ ;
    sci.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK ;
    sci.unnormalizedCoordinates = VK_FALSE ;

    // VkResult vkCreateSampler(
    //     VkDevice                                    device,
    //     const VkSamplerCreateInfo*                  pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkSampler*                                  pSampler);
    if(check_vulkan(vkCreateSampler(
                vc->device_
            ,   &sci
            ,   NULL
            ,   &vc->texture_sampler_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


static bool
destroy_vulkan_instance(
    vulkan_context *    vc
)
{
    require(vc) ;
    require(vc->device_) ;
    begin_timed_block() ;


    if(vc->device_)
    {
        // VkResult vkDeviceWaitIdle(
        //     VkDevice                                    device);
        vkDeviceWaitIdle(vc->device_) ;
    }

    cleanup_swapchain(vc) ;

    if(vc->texture_sampler_)
    {
        // void vkDestroySampler(
        //     VkDevice                                    device,
        //     VkSampler                                   sampler,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroySampler(vc->device_, vc->texture_sampler_, NULL) ;
        vc->texture_sampler_ = NULL ;
    }

    if(vc->texture_image_view_)
    {
        vkDestroyImageView(vc->device_, vc->texture_image_view_, NULL) ;
        vc->texture_image_view_ = NULL ;
    }

    if(vc->texture_image_)
    {
        // void vkDestroyImage(
        // VkDevice                                    device,
        // VkImage                                     image,
        // const VkAllocationCallbacks*                pAllocator);
        vkDestroyImage(vc->device_, vc->texture_image_, NULL) ;
        vc->texture_image_ = NULL ;
    }

    if(vc->texture_image_memory_)
    {
        vkFreeMemory(vc->device_, vc->texture_image_memory_, NULL) ;
        vc->texture_image_memory_ = NULL ;
    }

    for(
        uint32_t i = 0
    ;   i < vc->frames_in_flight_count_
    ;   ++i
    )
    {
        vkDestroyBuffer(vc->device_, vc->uniform_buffers_[i], NULL) ;
        vc->uniform_buffers_[i] = NULL ;
        vkFreeMemory(vc->device_, vc->uniform_buffers_memory_[i], NULL) ;
        vc->uniform_buffers_memory_[i] = NULL ;
    }


    if(vc->descriptor_pool_)
    {
        // void vkDestroyDescriptorPool(
        //     VkDevice                                    device,
        //     VkDescriptorPool                            descriptorPool,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyDescriptorPool(vc->device_, vc->descriptor_pool_, NULL) ;
        vc->descriptor_pool_ = NULL ;
    }


    if(vc->descriptor_set_layout_)
    {
        // void vkDestroyDescriptorSetLayout(
        //     VkDevice                                    device,
        //     VkDescriptorSetLayout                       descriptorSetLayout,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyDescriptorSetLayout(vc->device_, vc->descriptor_set_layout_, NULL) ;
        vc->descriptor_set_layout_ = NULL ;
    }


    if(vc->descriptor_set_layout_)
    {
        // void vkDestroyDescriptorSetLayout(
        //     VkDevice                                    device,
        //     VkDescriptorSetLayout                       descriptorSetLayout,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyDescriptorSetLayout(vc->device_, vc->descriptor_set_layout_, NULL) ;
        vc->descriptor_set_layout_ = NULL ;
    }


    if(vc->index_buffer_)
    {
        vkDestroyBuffer(vc->device_, vc->index_buffer_, NULL) ;
        vc->index_buffer_ = NULL ;
    }

    if(vc->index_buffer_memory_)
    {
        vkFreeMemory(vc->device_, vc->index_buffer_memory_, NULL) ;
        vc->index_buffer_memory_ = NULL ;
    }


    if(vc->vertex_buffer_)
    {
        // void vkDestroyBuffer(
        //     VkDevice                                    device,
        //     VkBuffer                                    buffer,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyBuffer(vc->device_, vc->vertex_buffer_, NULL) ;
        vc->vertex_buffer_ = NULL ;
    }

    if(vc->vertex_buffer_memory_)
    {
        // void vkFreeMemory(
        //     VkDevice                                    device,
        //     VkDeviceMemory                              memory,
        //     const VkAllocationCallbacks*                pAllocator);
        //     }
        vkFreeMemory(vc->device_, vc->vertex_buffer_memory_, NULL) ;
        vc->vertex_buffer_memory_ = NULL ;
    }

    if(vc->graphics_pipeline_)
    {
        // void vkDestroyPipeline(
        //     VkDevice                                    device,
        //     VkPipeline                                  pipeline,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyPipeline(vc->device_, vc->graphics_pipeline_, NULL) ;
        vc->graphics_pipeline_ = NULL ;
    }

    if(vc->pipeline_layout_)
    {
        // void vkDestroyPipelineLayout(
        //     VkDevice                                    device,
        //     VkPipelineLayout                            pipelineLayout,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyPipelineLayout(vc->device_, vc->pipeline_layout_, NULL) ;
        vc->pipeline_layout_ = NULL ;
    }

    if(vc->render_pass_)
    {
        // void vkDestroyRenderPass(
        //     VkDevice                                    device,
        //     VkRenderPass                                renderPass,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyRenderPass(vc->device_, vc->render_pass_, NULL) ;
        vc->render_pass_ = NULL ;
    }


    for(
        uint32_t i = 0
    ;   i < vc->frames_in_flight_count_
    ;   ++i
    )
    {
        if(vc->image_available_semaphore_[i])
        {
            // void vkDestroySemaphore(
            //     VkDevice                                    device,
            //     VkSemaphore                                 semaphore,
            //     const VkAllocationCallbacks*                pAllocator);
            vkDestroySemaphore(vc->device_, vc->image_available_semaphore_[i], NULL) ;
            vc->image_available_semaphore_[i] = NULL ;
        }

        if(vc->render_finished_semaphore_[i])
        {
            vkDestroySemaphore(vc->device_, vc->render_finished_semaphore_[i], NULL) ;
            vc->render_finished_semaphore_[i] = NULL ;
        }

        if(vc->in_flight_fence_[i])
        {
            // void vkDestroyFence(
            //     VkDevice                                    device,
            //     VkFence                                     fence,
            //     const VkAllocationCallbacks*                pAllocator);
            vkDestroyFence(vc->device_, vc->in_flight_fence_[i], NULL) ;
            vc->in_flight_fence_[i] = NULL ;
        }
    }

    if(vc->command_pool_)
    {
        // void vkDestroyCommandPool(
        //     VkDevice                                    device,
        //     VkCommandPool                               commandPool,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyCommandPool(vc->device_, vc->command_pool_, NULL) ;
        vc->command_pool_ = NULL ;
    }


    if(vc_->device_)
    {
        // void vkDestroyDevice(
        //     VkDevice                                    device,
        //     const VkAllocationCallbacks*                pAllocator);
        vkDestroyDevice(vc_->device_, NULL) ;
        vc_->device_ = NULL ;
    }

    if(vc->surface_)
    {
        vkDestroySurfaceKHR(vc->instance_, vc->surface_, NULL) ;
        vc->surface_ = NULL ;
    }

    destroy_debug_messenger(
        &vc->debug_messenger_
    ,   vc->instance_
    ,   vc->destroy_debug_messenger_func_
    ) ;

    if(vc->instance_)
    {
        vkDestroyInstance(vc->instance_, NULL) ;
        vc->instance_ = NULL ;
    }

    end_timed_block() ;
    return true ;
}


int
create_vulkan()
{
    begin_timed_block() ;

    vc_->enable_validation_ = VK_TRUE ;
    vc_->enable_sampling_ = VK_FALSE ;
    vc_->enable_sample_shading_ = VK_FALSE ;

    vc_->frames_in_flight_count_            = 2 ;
    vc_->desired_swapchain_image_count_     = 2 ;
    require(vc_->frames_in_flight_count_ < max_vulkan_frames_in_flight) ;

    vc_->sample_count_ = VK_SAMPLE_COUNT_8_BIT ;
    vc_->min_sample_shading_ = 0.2f ;

    vc_->desired_sampler_aniso_ = 1.0f ;


    vc_->resizing_ = VK_FALSE ;

    vc_->platform_instance_extensions_ = SDL_Vulkan_GetInstanceExtensions(
        &vc_->platform_instance_extensions_count_
    ) ;

    dump_char_star_array(
        vc_->platform_instance_extensions_
    ,   vc_->platform_instance_extensions_count_
    ) ;

    if(check(create_instance_extensions_properties(
                &vc_->instance_extensions_properties_
            ,   &vc_->instance_extensions_properties_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    dump_extension_properties(
        vc_->instance_extensions_properties_
    ,   vc_->instance_extensions_properties_count_
    ) ;

    for(
        uint32_t i = 0
    ;   i < vc_->platform_instance_extensions_count_
    ;   ++i
    )
    {
        add_to_extensions(
            vc_->desired_extensions_
        ,   &vc_->desired_extensions_count_
        ,   vc_->platform_instance_extensions_[i]
        ) ;
    }

    if(vc_->enable_validation_)
    {
        add_to_extensions(
            vc_->desired_extensions_
        ,   &vc_->desired_extensions_count_
        ,   vk_ext_debug_utils_extension_name
        ) ;
    }

    dump_char_star_array(
        vc_->desired_extensions_
    ,   vc_->desired_extensions_count_
    ) ;

    check(has_all_extensions(
            vc_->instance_extensions_properties_
        ,   vc_->instance_extensions_properties_count_
        ,   vc_->desired_extensions_
        ,   vc_->desired_extensions_count_
        )
    ) ;

    ///// layers

    if(check(create_layer_properties(
                &vc_->layer_properties_
            ,   &vc_->layer_properties_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    dump_layer_properties(
        vc_->layer_properties_
    ,   vc_->layer_properties_count_
    ) ;

    add_to_layers(
        vc_->desired_layers_
    ,   &vc_->desired_layers_count_
    ,   vk_layer_khronos_validation_name
    ) ;

    dump_char_star_array(
        vc_->desired_layers_
    ,   vc_->desired_layers_count_
    ) ;

    check(has_all_layer_properties(
            vc_->layer_properties_
        ,   vc_->layer_properties_count_
        ,   vc_->desired_layers_
        ,   vc_->desired_layers_count_
        )
    ) ;

    if(check(create_vulkan_instance(
                &vc_->instance_
            ,   vc_->desired_extensions_
            ,   vc_->desired_extensions_count_
            ,   vc_->desired_layers_
            ,   vc_->desired_layers_count_
            ,   vc_->enable_validation_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    check(create_function_pointers(vc_->instance_, vc_)) ;

    if(check(create_debug_messenger(
                &vc_->debug_messenger_
            ,   vc_->instance_
            ,   vc_->create_debug_messenger_func_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_surface(
                &vc_->surface_
            ,   vc_->instance_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_physical_devices(
                vc_->physical_devices_
            ,   &vc_->physical_devices_count_
            ,   vc_->instance_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    log_debug_u32(vc_->physical_devices_count_) ;

    fill_physical_devices_info(
        vc_->physical_devices_info_
    ,   vc_->physical_devices_
    ,   vc_->physical_devices_count_
    ,   vc_->surface_
    ) ;

    dump_physical_device_info(
        vc_->physical_devices_info_
    ,   vc_->physical_devices_count_
    ) ;

    if(check(pick_physical_device(
                &vc_->picked_physical_device_
            ,   vc_->physical_devices_info_
            ,   vc_->physical_devices_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_logical_device(
                &vc_->device_
            ,   vc_->picked_physical_device_->device_
            ,   &vc_->picked_physical_device_->features_
            ,   vc_->picked_physical_device_->unique_queue_families_indices_
            ,   vc_->picked_physical_device_->unique_queue_families_indices_count_
            ,   vc_->picked_physical_device_->desired_device_extensions_
            ,   vc_->picked_physical_device_->desired_device_extensions_count_
            ,   vc_->desired_layers_
            ,   vc_->desired_layers_count_
            ,   vc_->enable_validation_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc_->device_) ;

    if(check(create_queues(
                &vc_->graphics_queue_
            ,   &vc_->present_queue_
            ,   vc_->device_
            ,   vc_->picked_physical_device_->queue_families_indices_.graphics_family_
            ,   vc_->picked_physical_device_->queue_families_indices_.present_family_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc_->graphics_queue_) ;
    require(vc_->present_queue_) ;

    if(check(create_command_pool(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_swapchain(
                &vc_->swapchain_
            ,   &vc_->swapchain_surface_format_
            ,   &vc_->swapchain_present_mode_
            ,   &vc_->swapchain_extent_
            ,   vc_->swapchain_images_
            ,   &vc_->swapchain_images_count_
            ,   vc_->device_
            ,   vc_->surface_
            ,   &vc_->picked_physical_device_->swapchain_support_details_
            ,   &vc_->picked_physical_device_->queue_families_indices_
            ,   vc_->desired_swapchain_image_count_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc_->swapchain_) ;

    if(check(create_image_views(
                vc_->swapchain_views_
            ,   vc_->swapchain_images_
            ,   vc_->swapchain_images_count_
            ,   vc_->device_
            ,   vc_->swapchain_surface_format_.format
            ,   VK_IMAGE_ASPECT_COLOR_BIT
            ,   1
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    log_debug("created swapchain with extent (%d,%d) and %d images, desired images=%d"
    ,   vc_->swapchain_extent_.width
    ,   vc_->swapchain_extent_.height
    ,   vc_->swapchain_images_count_
    ,   vc_->desired_swapchain_image_count_
    ) ;

    if(check(create_color_resource(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_depth_resource(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_render_pass(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_framebuffers(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_sync_objects(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_command_buffer(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    // ------------------


    if(check(create_rob(vc_)))
    {
        end_timed_block() ;
        return false ;
    }


    if(check(create_graphics_pipeline(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_vertex_buffer(
                &vc_->vertex_buffer_
            ,   &vc_->vertex_buffer_memory_
            ,   vc_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc_->vertex_buffer_) ;
    require(vc_->vertex_buffer_memory_) ;

    if(check(create_index_buffer(
                &vc_->index_buffer_
            ,   &vc_->index_buffer_memory_
            ,   vc_
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(vc_->index_buffer_) ;
    require(vc_->index_buffer_memory_) ;

    // if(check(create_uniform_buffers(vc_)))
    // {
    //     end_timed_block() ;
    //     return false ;
    // }

    // if(check(create_texture_image(vc_)))
    // {
    //     end_timed_block() ;
    //     return false ;
    // }

    if(check(create_texture_image_view(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    require(vc_->desired_sampler_aniso_ <= vc_->picked_physical_device_->properties_.limits.maxSamplerAnisotropy) ;

    if(check(create_texture_sampler(vc_)))
    {
        end_timed_block() ;
        return false ;
    }


    // if(check(create_descriptor_sets(vc_)))
    // {
    //     end_timed_block() ;
    //     return false ;
    // }

    end_timed_block() ;
    return true ;
}


void
destroy_vulkan()
{
    begin_timed_block() ;

    destroy_vulkan_instance(vc_) ;

    end_timed_block() ;
}


int
draw_vulkan()
{
    begin_timed_block() ;

    if(check(draw_frame(vc_)))
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


void
resize_vulkan()
{
    vc_->resizing_ = VK_TRUE ;
}



void
add_desriptor_set_layout_binding(
    VkDescriptorSetLayoutBinding *  bindings
,   uint32_t *                      bindings_count
,   uint32_t const                  bindings_count_max
,   uint32_t const                  binding
,   VkDescriptorType const          descriptor_type
,   VkShaderStageFlags const        stage_flags
)
{
    require(bindings) ;
    require(bindings_count) ;
    require(*bindings_count < bindings_count_max) ;

    begin_timed_block() ;

    uint32_t const idx = *bindings_count ;
    VkDescriptorSetLayoutBinding * dslb = &bindings[idx] ;

    // typedef struct VkDescriptorSetLayoutBinding {
    //     uint32_t              binding;
    //     VkDescriptorType      descriptorType;
    //     uint32_t              descriptorCount;
    //     VkShaderStageFlags    stageFlags;
    //     const VkSampler*      pImmutableSamplers;
    // } VkDescriptorSetLayoutBinding;
    dslb->binding                = binding ;
    dslb->descriptorType         = descriptor_type ;
    dslb->descriptorCount        = 1 ;
    dslb->stageFlags             = stage_flags ;
    dslb->pImmutableSamplers     = NULL ;

    ++ *bindings_count ;

    end_timed_block() ;
}


bool
create_descriptor_set_layout(
    VkDescriptorSetLayout *                 out_layout
,   VkDevice const                          device
,   VkDescriptorSetLayoutBinding const *    bindings
,   uint32_t const                          bindings_count
)
{
    require(out_layout) ;
    require(device) ;
    require(bindings) ;
    require(bindings_count) ;
    begin_timed_block() ;


    // typedef struct VkDescriptorSetLayoutCreateInfo {
    //     VkStructureType                        sType;
    //     const void*                            pNext;
    //     VkDescriptorSetLayoutCreateFlags       flags;
    //     uint32_t                               bindingCount;
    //     const VkDescriptorSetLayoutBinding*    pBindings;
    // } VkDescriptorSetLayoutCreateInfo;
    static VkDescriptorSetLayoutCreateInfo dslci = { 0 } ;
    dslci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO ;
    dslci.pNext         = NULL ;
    dslci.flags         = 0 ;
    dslci.bindingCount  = bindings_count ;
    dslci.pBindings     = bindings ;


    // VkResult vkCreateDescriptorSetLayout(
    //     VkDevice                                    device,
    //     const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkDescriptorSetLayout*                      pSetLayout);
    if(check_vulkan(vkCreateDescriptorSetLayout(
                device
            ,   &dslci
            ,   NULL
            ,   out_layout
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(*out_layout) ;

    end_timed_block() ;
    return true ;
}


void
fill_pipeline_layout_create_info(
    VkPipelineLayoutCreateInfo *    plci
,   VkDescriptorSetLayout const *   descriptor_set_layouts
,   uint32_t const                  descriptor_set_layouts_count
)
{
    require(plci) ;
    require(descriptor_set_layouts) ;
    require(descriptor_set_layouts_count) ;
    begin_timed_block() ;

    // typedef struct VkPipelineLayoutCreateInfo {
    //     VkStructureType                 sType;
    //     const void*                     pNext;
    //     VkPipelineLayoutCreateFlags     flags;
    //     uint32_t                        setLayoutCount;
    //     const VkDescriptorSetLayout*    pSetLayouts;
    //     uint32_t                        pushConstantRangeCount;
    //     const VkPushConstantRange*      pPushConstantRanges;
    // } VkPipelineLayoutCreateInfo;
    plci->sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO ;
    plci->pNext                      = NULL ;
    plci->flags                      = 0 ;
    plci->setLayoutCount             = descriptor_set_layouts_count ;
    plci->pSetLayouts                = descriptor_set_layouts ;
    plci->pushConstantRangeCount     = 0 ;
    plci->pPushConstantRanges        = NULL ;

    end_timed_block() ;
}


void
add_descriptor_pool_size(
    VkDescriptorPoolSize *  pool_sizes
,   uint32_t *              pool_sizes_count
,   uint32_t const          pool_sizes_count_max
,   VkDescriptorType const  descriptor_type
,   uint32_t const          frames_in_flight_count
)
{
    require(pool_sizes) ;
    require(pool_sizes_count) ;
    require(*pool_sizes_count < pool_sizes_count_max) ;
    begin_timed_block() ;

    uint32_t const idx = *pool_sizes_count ;

    // typedef struct VkDescriptorPoolSize {
    //     VkDescriptorType    type;
    //     uint32_t            descriptorCount;
    // } VkDescriptorPoolSize;
    VkDescriptorPoolSize * dps = &pool_sizes[idx] ;
    dps->type            = descriptor_type ;
    dps->descriptorCount = frames_in_flight_count ;

    end_timed_block() ;
}


bool
create_descriptor_pool(
    VkDescriptorPool *              out_descriptor_pool
,   VkDevice const                  device
,   VkDescriptorPoolSize const *    pool_sizes
,   uint32_t const                  pool_sizes_count
,   uint32_t const                  frames_in_flight_count
)
{
    require(out_descriptor_pool) ;
    require(device) ;
    require(pool_sizes) ;
    require(pool_sizes_count) ;
    require(frames_in_flight_count) ;
    require(frames_in_flight_count < max_vulkan_frames_in_flight) ;

    begin_timed_block() ;


    // typedef struct VkDescriptorPoolCreateInfo {
    //     VkStructureType                sType;
    //     const void*                    pNext;
    //     VkDescriptorPoolCreateFlags    flags;
    //     uint32_t                       maxSets;
    //     uint32_t                       poolSizeCount;
    //     const VkDescriptorPoolSize*    pPoolSizes;
    // } VkDescriptorPoolCreateInfo;
    static VkDescriptorPoolCreateInfo dpci = { 0 } ;
    dpci.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO ;
    dpci.pNext          = NULL ;
    dpci.flags          = 0 ;
    dpci.maxSets        = frames_in_flight_count ;
    dpci.poolSizeCount  = pool_sizes_count ;
    dpci.pPoolSizes     = pool_sizes ;


    // VkResult vkCreateDescriptorPool(
    //     VkDevice                                    device,
    //     const VkDescriptorPoolCreateInfo*           pCreateInfo,
    //     const VkAllocationCallbacks*                pAllocator,
    //     VkDescriptorPool*                           pDescriptorPool);
    if(check_vulkan(vkCreateDescriptorPool(
                device
            ,   &dpci
            ,   NULL
            ,   out_descriptor_pool
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(out_descriptor_pool) ;

    end_timed_block() ;
    return true ;

}


bool
create_descriptor_sets(
    VkDescriptorSet *           out_descriptor_sets
,   VkDevice const              device
,   VkDescriptorSetLayout const descriptor_set_layout
,   VkDescriptorPool const      descriptor_pool
,   uint32_t const              frames_in_flight_count
)
{
    require(out_descriptor_sets) ;
    require(device) ;
    require(descriptor_set_layout) ;
    require(descriptor_pool) ;
    require(frames_in_flight_count) ;
    require(frames_in_flight_count < max_vulkan_frames_in_flight) ;
    begin_timed_block() ;

    static VkDescriptorSetLayout   layouts[max_vulkan_frames_in_flight] = { 0 } ;

    for(
        uint32_t i = 0
    ;   i < frames_in_flight_count
    ;   ++i
    )
    {
        layouts[i] = descriptor_set_layout ;
    }


    // typedef struct VkDescriptorSetAllocateInfo {
    //     VkStructureType                 sType;
    //     const void*                     pNext;
    //     VkDescriptorPool                descriptorPool;
    //     uint32_t                        descriptorSetCount;
    //     const VkDescriptorSetLayout*    pSetLayouts;
    // } VkDescriptorSetAllocateInfo;
    static VkDescriptorSetAllocateInfo  dsai = { 0 } ;
    dsai.sType                  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO ;
    dsai.pNext                  = NULL ;
    dsai.descriptorPool         = descriptor_pool ;
    dsai.descriptorSetCount     = frames_in_flight_count ;
    dsai.pSetLayouts            = layouts ;


    // VkResult vkAllocateDescriptorSets(
    //     VkDevice                                    device,
    //     const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    //     VkDescriptorSet*                            pDescriptorSets);
    if(check_vulkan(vkAllocateDescriptorSets(
                device
            ,   &dsai
            ,   out_descriptor_sets
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


bool
create_uniform_buffers(
    VkBuffer *                                  out_uniform_buffers
,   VkDeviceMemory *                            out_uniform_buffers_memory
,   void **                                     out_uniform_buffers_mapped
,   VkDevice const                              device
,   uint32_t const                              uniform_buffer_size
,   VkPhysicalDeviceMemoryProperties const *    pdmp
,   uint32_t const                              frames_in_flight_count
)
{
    require(out_uniform_buffers) ;
    require(out_uniform_buffers_memory) ;
    require(out_uniform_buffers_mapped) ;
    require(device) ;
    require(uniform_buffer_size) ;
    require(pdmp) ;
    require(frames_in_flight_count) ;
    require(frames_in_flight_count < max_vulkan_frames_in_flight) ;

    begin_timed_block() ;

    VkDeviceSize const buffer_size = uniform_buffer_size ;

    for(
        uint32_t i = 0
    ;   i < frames_in_flight_count
    ;   ++i
    )
    {
        if(check(create_buffer(
                    &out_uniform_buffers[i]
                ,   &out_uniform_buffers_memory[i]
                ,   device
                ,   pdmp
                ,   buffer_size
                ,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                ,   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
        require(out_uniform_buffers[i]) ;
        require(out_uniform_buffers_memory[i]) ;

        if(check_vulkan(vkMapMemory(
                    device
                ,   out_uniform_buffers_memory[i]
                ,   0
                ,   buffer_size
                ,   0
                ,   &out_uniform_buffers_mapped[i]
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
        require(out_uniform_buffers_mapped[i]) ;
    }

    end_timed_block() ;
    return true ;
}


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
)
{
    require(out_image) ;
    require(out_image_memory) ;
    require(full_name) ;
    require(device) ;
    require(command_pool) ;
    require(graphics_queue) ;
    require(pdmp) ;
    begin_timed_block() ;

    int tx_width    = 0 ;
    int tx_height   = 0 ;
    int tx_channels = 0 ;

    stbi_uc * pixels = stbi_load(
        full_name
    ,   &tx_width
    ,   &tx_height
    ,   &tx_channels
    ,   STBI_rgb_alpha
    ) ;
    require(pixels) ;
    require(tx_width > 0) ;
    require(tx_height > 0) ;
    require(tx_channels > 0) ;

    VkDeviceSize const image_size = tx_width * tx_height * 4 ;

    log_debug_u32(tx_width) ;
    log_debug_u32(tx_height) ;
    log_debug_u32(mip_levels) ;

    if(check(pixels && image_size))
    {
        end_timed_block() ;
        return false ;
    }

    uint32_t desired_mip_levels = mip_levels ;
    if(0 == mip_levels)
    {
        desired_mip_levels = calc_mip_levels(tx_width, tx_height) ;
    }

    VkBuffer staging_buffer                 = NULL ;
    VkDeviceMemory staging_buffer_memory    = NULL ;

    if(check(create_buffer(
                &staging_buffer
            ,   &staging_buffer_memory
            ,   device
            ,   pdmp
            ,   image_size
            ,   VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            ,   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(staging_buffer) ;
    require(staging_buffer_memory) ;

    void * data = NULL ;

    if(check_vulkan(vkMapMemory(
                device
            ,   staging_buffer_memory
            ,   0
            ,   image_size
            ,   0
            ,   &data
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }
    require(data) ;

    SDL_memcpy(data, pixels, image_size) ;

    vkUnmapMemory(device, staging_buffer_memory) ;

    stbi_image_free(pixels) ;

    if(check(create_image(
                out_image
            ,   out_image_memory
            ,   device
            ,   pdmp
            ,   tx_width
            ,   tx_height
            ,   desired_mip_levels
            ,   VK_SAMPLE_COUNT_1_BIT
            ,   VK_FORMAT_R8G8B8A8_SRGB
            ,   VK_IMAGE_TILING_OPTIMAL
            ,   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
            ,   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }


    if(check(transition_image_layout(
                device
            ,   command_pool
            ,   graphics_queue
            ,   *out_image
            ,   VK_FORMAT_R8G8B8A8_SRGB
            ,   VK_IMAGE_LAYOUT_UNDEFINED
            ,   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            ,   desired_mip_levels
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(check(copy_buffer_to_image(
                device
            ,   command_pool
            ,   graphics_queue
            ,   staging_buffer
            ,   *out_image
            ,   tx_width
            ,   tx_height
            )
        )
    )
    {
        end_timed_block() ;
        return false ;
    }

    if(1 == desired_mip_levels)
    {
        if(check(transition_image_layout(
                    device
                ,   command_pool
                ,   graphics_queue
                ,   *out_image
                ,   VK_FORMAT_R8G8B8A8_SRGB
                ,   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                ,   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                ,   desired_mip_levels
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
    }
    else
    {
        if(check(generate_mipmaps(
                    device
                ,   command_pool
                ,   graphics_queue
                ,   *out_image
                ,   tx_width
                ,   tx_height
                ,   desired_mip_levels
                )
            )
        )
        {
            end_timed_block() ;
            return false ;
        }
    }

    vkDestroyBuffer(device, staging_buffer, NULL) ;
    vkFreeMemory(device, staging_buffer_memory, NULL) ;

    end_timed_block() ;
    return true ;
}
