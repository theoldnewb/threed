#include "app.h"
#include "vulkan.h"
#include "types.h"
#include "check.h"
#include "log.h"
#include "debug.h"
#include "math.h"

#include <SDL3/SDL_vulkan.h>


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



#define max_dump_buffer 4096

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
    }
}


// static bool
// create_queue_families(
//     vulkan_physical_device_info *   physical_device_info
// ,   uint32_t const                  physical_device_info_count
// )
// {
//     for(
//         uint32_t i = 0
//     ;   i < physical_device_info_count
//     ;   ++i
//     )
//     {
//         vulkan_physical_device_info * pdi = &physical_device_info[i] ;

//     }
// }



static bool
destroy_vulkan_instance(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

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
