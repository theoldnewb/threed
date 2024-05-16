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
fill_physical_device_info(
    vulkan_physical_device_info *   out_physical_device_info
,   VkPhysicalDevice                device
)
{
    require(out_physical_device_info) ;
    require(device) ;

    out_physical_device_info->device_ = device ;

    // void vkGetPhysicalDeviceProperties(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkPhysicalDeviceProperties*                 pProperties);
    vkGetPhysicalDeviceProperties(
        device
    ,   &out_physical_device_info->properties_
    ) ;


    // void vkGetPhysicalDeviceFeatures(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkPhysicalDeviceFeatures*                   pFeatures);
    vkGetPhysicalDeviceFeatures(
        device
    ,   &out_physical_device_info->features_
    ) ;

    // void vkGetPhysicalDeviceMemoryProperties(
    //     VkPhysicalDevice                            physicalDevice,
    //     VkPhysicalDeviceMemoryProperties*           pMemoryProperties);
    vkGetPhysicalDeviceMemoryProperties(
        device
    ,   &out_physical_device_info->memory_properties_
    ) ;

    // void vkGetPhysicalDeviceQueueFamilyProperties(
    //     VkPhysicalDevice                            physicalDevice,
    //     uint32_t*                                   pQueueFamilyPropertyCount,
    //     VkQueueFamilyProperties*                    pQueueFamilyProperties);
    //     vkGetPhysicalDeviceQueueFamilyProperties()
    vkGetPhysicalDeviceQueueFamilyProperties(
        device
    ,   &out_physical_device_info->queue_family_properties_count_
    ,   NULL
    ) ;

    log_debug_u32(out_physical_device_info->queue_family_properties_count_) ;
    require(out_physical_device_info->queue_family_properties_count_ < max_vulkan_queue_family_properties)
    out_physical_device_info->queue_family_properties_count_ = min_u32(out_physical_device_info->queue_family_properties_count_, max_vulkan_queue_family_properties) ;

    vkGetPhysicalDeviceQueueFamilyProperties(
        device
    ,   &out_physical_device_info->queue_family_properties_count_
    ,   out_physical_device_info->queue_family_properties_
    ) ;


    // VkResult vkEnumerateDeviceExtensionProperties(
    //     VkPhysicalDevice                            physicalDevice,
    //     const char*                                 pLayerName,
    //     uint32_t*                                   pPropertyCount,
    //     VkExtensionProperties*                      pProperties);
    if(check_vulkan(vkEnumerateDeviceExtensionProperties(
                device
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
                device
            ,   NULL
            ,   &out_physical_device_info->device_extensions_count_
            ,   out_physical_device_info->device_extensions_
            )
        )
    )
    {
        return false ;
    }


    return true ;
}


static void
fill_physical_devices_info(
    vulkan_physical_device_info *   out_physical_device_info
,   VkPhysicalDevice *              devices
,   uint32_t const                  devices_count
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
                )
            )
        )
        {
            require(0) ;
        }

        dump_physical_device_properties(&out_physical_device_info[i].properties_) ;
        dump_physical_device_features(&out_physical_device_info[i].features_) ;
        dump_physical_device_memory_properties(&out_physical_device_info[i].memory_properties_) ;
        dump_queue_family_properties(out_physical_device_info[i].queue_family_properties_, out_physical_device_info->queue_family_properties_count_) ;
        dump_extension_properties(out_physical_device_info[i].device_extensions_, out_physical_device_info->device_extensions_count_) ;
    }

}


static bool
destroy_vulkan_instance(
    vulkan_context *    vc
)
{
    require(vc) ;
    begin_timed_block() ;

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
